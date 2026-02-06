#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <asm/signal.h>
#include <asm-generic/siginfo.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/regmap.h>
#include <linux/sysfs.h>
#include <linux/spi/spi.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/time64.h>

#include "sanway_spi_adc_hrtimer.h"

// #define OK_PRINT_IRIGB_TIME	1
/* 此处定义1个可以传递给驱动模块的参数，表示是否打印IRIG-B码解码完成后的时间 */
// static int irigb_print_time = OK_PRINT_IRIGB_TIME;
// module_param(irigb_print_time, int, 0);
// MODULE_PARM_DESC(irigb_print_time, "Print IRIG-B decode time or not.");

#define DEBUG_SPI_CMD

#ifdef DEBUG_SPI_CMD
#define CPLD_PRN(function, lineno, fmt, args...) printk(fmt, function, lineno, ##args)
#define DBG_PRN(fmt,args...) CPLD_PRN((__FUNCTION__),(__LINE__),KERN_INFO __FILE__"::%s(L%.4d): "fmt,##args)
#else
#define DBG_PRN(fmt, args...) /* not debugging: nothing */
#endif

#define ESAM_DEV_MAJOR			199
#define ESAM_DEVNAME			"spiadc"
#define ESAM_CLASS_NAME			"spiadc_class"

#define SPI_CMD_MAX_HZ			50000000
#define SPI_CMD_DRV_NAME		"sanway_spiadc_dm200"

#define ADC_CHANNEL_NUM			16			//ADC通道数
#define SPI_BYTE_MAX_LEN		(ADC_CHANNEL_NUM * 2)	//SPI最大传输字节数
#define FPGA_STORE_L            	120000 	//采用kmalloc，根据计算，分配连续内存空间最大不能超过2MB
//#define RECORD_BUF_SIZE_MAX		(FPGA_STORE_L * SPI_BYTE_MAX_LEN)

#define RECORD_BUF_SIZE_MAX		2097152		//65536 * 32
#define RECORD_BUF_SIZE_USED		(RECORD_BUF_SIZE_MAX - 8192)

struct cpld_spi_cmd_priv {
	struct spi_device 			*spi;
	struct device				*dev;
	struct cdev				cdev;
	struct class				*dev_class;
	dev_t					dev_no;
	struct device           		*sysfs_dev;		//用于/sysfs的设备指针
	
	struct mutex 				m_lock; /* SPI device lock */
	struct fasync_struct 			*fasync;
	void 					*mmap_start;			//用于mmap的首地址
	struct page				*mmap_page;       	// 分配的起始页面（alloc_pages）
	unsigned int				mmap_order;       	// alloc_pages 使用的 order
	unsigned long				mmap_size;        	// 实际分配的字节数
	struct gpio_desc 			*gpiod_reset;	
	struct gpio_desc 			*gpiod_convst;
	u8 					*spi_tx_buf;		//SPI发送buffer
	u8 					*spi_rx_buf;		//SPI接收buffer
	struct fifo_mmap_mem 			appdata;		//用于异步通知时传递给用户程序的数据
	int					irq_adc;
	u32					delay_counter;
	unsigned long				interval;		//ADC采样周期，单位ns
	u8					adc_state;
	struct hrtimer				hrtimer;
	struct work_struct			spi_work;
	struct workqueue_struct 		*wq; 			// 新增，工作队列            
    	int 					target_cpu;             // 新增，指定工作队列运行在哪个CPU上
	u32					offset;	
	u32					size;		//记录缓存大小
	u32					curr_pos;
	u32					fsync_overflow;	
};

#define HRTIMER_COUNT_TIME_SLICE		100	//hrtimer采样间隔，单位us
#define HRTIMER_TIME_US				1000	//hrtimer时间片，单位us
#define HRTIMER_COUNT_TIME_FSYNC		1500	//hrtimer异步通知时间片，单位us，默认1.5毫秒发送一次
#define COUNT_TIME_SLICE(x) 			((x) * HRTIMER_TIME_US)
static unsigned long 	interval = COUNT_TIME_SLICE(HRTIMER_COUNT_TIME_SLICE);

#if 0
static int spi_read_trans(struct spi_device *spi, int len)
{
	struct cpld_spi_cmd_priv *priv = spi_get_drvdata(spi);
	struct spi_transfer t = {
		//.tx_buf = priv->spi_tx_buf,
		.rx_buf = priv->spi_rx_buf,
		.len = len,
		.cs_change = 0,
	};
	struct spi_message m;
	int ret;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	ret = spi_sync(spi, &m);
	if (ret)
		dev_err(&spi->dev, "spi transfer failed: ret = %d\n", ret);
	return ret;
}
#endif

static inline void adc_sample_once(struct cpld_spi_cmd_priv *drvdata, int us_delay)
{
#if 1	
	gpiod_set_value(drvdata->gpiod_convst, 1);
	udelay(us_delay);
	gpiod_set_value(drvdata->gpiod_convst, 0);
	udelay(us_delay);
#endif
}

static void spi_adc_work_func(struct work_struct *work)
{
	struct cpld_spi_cmd_priv *drvdata = container_of(work, struct cpld_spi_cmd_priv, spi_work);
	struct spi_device *spi = drvdata->spi;
	struct device *dev = drvdata->dev;
	u8 *buf = &drvdata->spi_rx_buf[drvdata->offset];
	int ret = 0;
	static u32 count = 0;
	u32 fsync_overflow = 0;

	mutex_lock(&drvdata->m_lock);
	fsync_overflow = drvdata->fsync_overflow;

	//进行一次采样
	adc_sample_once(drvdata, 1);

	//读取ADC数据
	ret = spi_read(spi, buf, SPI_BYTE_MAX_LEN);
	if(ret < 0) {
		dev_err(dev, "spi_read failed: ret = %d\n", ret);
		mutex_unlock(&drvdata->m_lock);
		return;
	}

	//计算缓存区的位置和每次新的数据长度
	drvdata->offset += SPI_BYTE_MAX_LEN;
	drvdata->offset %= RECORD_BUF_SIZE_USED;

	count++;
	//1.5ms发送一次异步通知
	if((count % fsync_overflow) == 0)
	{
		// drvdata->size = ((drvdata->offset - drvdata->curr_pos) >= 0)
				// ? (drvdata->offset - drvdata->curr_pos)
				// : (RECORD_BUF_SIZE_MAX - drvdata->curr_pos + drvdata->offset);

		drvdata->size = fsync_overflow * SPI_BYTE_MAX_LEN;
		drvdata->appdata.page_sum = drvdata->size;
		drvdata->appdata.page_pos = drvdata->curr_pos;
		//mutex_unlock(&drvdata->m_lock);
		kill_fasync(&drvdata->fasync, SIGIO, POLL_IN);          //发送异步通知

		drvdata->curr_pos = drvdata->offset;
	}	

	mutex_unlock(&drvdata->m_lock);
}

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
	struct cpld_spi_cmd_priv *drvdata = container_of(timer, struct cpld_spi_cmd_priv, hrtimer);
	// struct spi_device *spi = drvdata->spi;
	// u8 *buf = drvdata->spi_rx_buf;
	// static u32 count = 0;
	// u32 fsync_overflow = HRTIMER_COUNT_TIME_FSYNC / (drvdata->interval / HRTIMER_TIME_US);
	// count++;			
	
	// schedule_work(&drvdata->spi_work);
	queue_work_on(drvdata->target_cpu, drvdata->wq, &drvdata->spi_work);
#if 0
	//3ms发送一次异步通知
	if((count % fsync_overflow) == 0) {
		//mutex_lock(&drvdata->m_lock);

		if (drvdata->offset == drvdata->curr_pos) {
		    drvdata->size = 0; // 没有新数据
		} else if (drvdata->offset > drvdata->curr_pos) {
		    drvdata->size = drvdata->offset - drvdata->curr_pos;
		} else {
		    drvdata->size = RECORD_BUF_SIZE_MAX - drvdata->curr_pos + drvdata->offset;
		}
		drvdata->curr_pos = drvdata->offset;

		drvdata->appdata.page_sum = drvdata->size;
		drvdata->appdata.page_pos = drvdata->offset;

		//mutex_unlock(&drvdata->m_lock);
		kill_fasync(&drvdata->fasync, SIGIO, POLL_IN);          //发送异步通知
	}	
#endif
	hrtimer_forward_now(timer, ns_to_ktime(drvdata->interval));

	//每次退出之前，都进行一次采样
	// adc_sample_once(drvdata, 1);
	return HRTIMER_RESTART;
}

static void adc_reset(struct cpld_spi_cmd_priv *drvdata)
{
	gpiod_set_value(drvdata->gpiod_reset, 1);
	mdelay(10);
	gpiod_set_value(drvdata->gpiod_reset, 0);
	mdelay(10);
}

#if 0
//中断回调函数
static irqreturn_t adc_irq_handler(int irqno, void *dev_id)
{
	struct cpld_spi_cmd_priv *drvdata = (struct cpld_spi_cmd_priv *)dev_id;
	// struct spi_device *spi = drvdata->spi;
	// struct device *dev = &spi->dev;
	// int i = 0;

	mutex_lock(&drvdata->m_lock);
	drvdata->delay_counter++;
	if (drvdata->delay_counter >= 1000) {
		drvdata->delay_counter = 0;
		kill_fasync(&drvdata->fasync, SIGIO, POLL_IN);
	}
	mutex_unlock(&drvdata->m_lock);

	return IRQ_HANDLED;
}
#endif

static int esam_fasync(int fd, struct file *filep, int mode) {
	struct cpld_spi_cmd_priv *drvdata = filep->private_data;

	return fasync_helper(fd, filep, mode, &drvdata->fasync);
}

static int esam_open(struct inode *inodep, struct file *filep) 
{
	struct cpld_spi_cmd_priv *drvdata;
	struct cdev *c = inodep->i_cdev;
//	unsigned int irq_flags = 0;
//	int ret = 0;
	struct device *dev;

	if (!c) {
		return -1;
	}

	drvdata = container_of(c, struct cpld_spi_cmd_priv, cdev);
	filep->private_data = drvdata;
	dev = drvdata->dev;
	
	dev_dbg(dev, "Driver Opened success.\n");

	try_module_get(THIS_MODULE);
	return 0;
}

static int esam_release(struct inode *inodep, struct file *filep) {
	struct cpld_spi_cmd_priv *drvdata = filep->private_data;
	struct device *dev = drvdata->dev;

	esam_fasync(-1, filep, 0);
//	free_irq(drvdata->irq_adc, drvdata);
	hrtimer_cancel(&drvdata->hrtimer);
	flush_work(&drvdata->spi_work);
	dev_info(dev, "Driver released success.\n");
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t esam_read(struct file *filep, char __user *buff, size_t count, loff_t *offp) {

	return 0;
}

static ssize_t esam_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp) {

	return 0;
}

static long esam_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) 
{
	struct cpld_spi_cmd_priv *drvdata = filep->private_data;
	struct spi_device *spi = drvdata->spi;
	struct device *dev = &spi->dev;
	int ret = 0;
	// u8 uval = 0;
	u16 wval = 0;
	void __user *uarg = (void __user *)arg;

//	dev_info(dev, "ioctl handle...\n");

	if (_IOC_TYPE(cmd) != ESAM_MAJIC) {
		dev_err(dev, "ioctl magic error: = %d\n", _IOC_TYPE(cmd));
		return -EINVAL;
	}

	switch (cmd) {
	case IOREQ_ADC_RESET:
		adc_reset(drvdata);
		DBG_PRN("adc reset.\n");
		break;

	case IOREQ_ADC_START:				
		dev_info(dev, "adc_start_sample_cycle: %ld us.\n", drvdata->interval / HRTIMER_TIME_US);
		if(drvdata->interval == 0) {
			dev_err(dev, "adc sample cycle is 0.\n");
			return -EINVAL;
		}

		if(drvdata->adc_state) {
			hrtimer_cancel(&drvdata->hrtimer);
			drvdata->adc_state = 0;
		}

		//adc_sample_once(drvdata, 1);	//第一次采样
		hrtimer_start(&drvdata->hrtimer, ns_to_ktime(drvdata->interval), HRTIMER_MODE_REL);
		drvdata->adc_state = 1;
		DBG_PRN("adc start sample.\n");
		break;

	case IOREQ_ADC_STOP:
		hrtimer_cancel(&drvdata->hrtimer);
		drvdata->adc_state = 0;
		DBG_PRN("adc stop sample.\n");
		break;

	case IOREQ_ADC_SAMPLE_CYCLE_SET:
		ret = copy_from_user(&wval, uarg, sizeof(wval));
		if (ret < 0) {
			dev_err(dev, "copy_from_user failed: ret = %d\n", ret);
			return ret;
		}

		if((wval < 78) || (wval > 10000)) {
			dev_err(dev, "adc sample cycle out of range: %d\n", wval);
			return -EINVAL;
		}
		drvdata->interval = COUNT_TIME_SLICE(wval);
		drvdata->interval = (drvdata->interval == 78000) ? 78125 : drvdata->interval;
		drvdata->fsync_overflow = HRTIMER_COUNT_TIME_FSYNC / (drvdata->interval / HRTIMER_TIME_US);
		DBG_PRN("Set adc sample cycle: %d us, %ld ns, sample_times: %d.\n", 
			wval, drvdata->interval, drvdata->fsync_overflow);
		break;
	case IOREQ_ADC_SAMPLE_CYCLE_GET:		
		wval = (drvdata->interval / HRTIMER_TIME_US);
		ret = copy_to_user(uarg, &wval, sizeof(wval));
		if (ret < 0) {
			dev_err(dev, "copy_to_user failed: ret = %d\n", ret);
			return ret;
		}
		DBG_PRN("Get ADC sample cycle: %d us\n", wval);
		break;
	case IOREQ_ADC_READ_NOTIFY:
		mutex_lock(&drvdata->m_lock);
		ret = copy_to_user(uarg, &(drvdata->appdata), sizeof(struct fifo_mmap_mem));
		mutex_unlock(&drvdata->m_lock);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static long esam_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return esam_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}

static int esam_mmap(struct file *filep, struct vm_area_struct *vma)
{
	struct cpld_spi_cmd_priv *drvdata = filep->private_data;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = (unsigned long)vma->vm_pgoff << PAGE_SHIFT;
	void *vaddr;
	phys_addr_t phys;
	unsigned long pfn;
	unsigned long i, npages;

	if (!drvdata || !drvdata->mmap_start)
		return -EINVAL;

	vaddr = drvdata->mmap_start;

	/* bounds check against actual allocated buffer size */
	if (offset + size > drvdata->mmap_size)
		return -EINVAL;
#if 0
	/* bounds check against actual buffer size (prefer dma_size when present) */
	if (drvdata->dma_buf) {
		if (offset + size > drvdata->dma_size)
			return -EINVAL;
	} else {
		if (offset + size > RECORD_BUF_SIZE_MAX)
			return -EINVAL;
	}

	/* 1) If we have a DMA coherent buffer, map using dma_handle (fast, safe) */
	if (drvdata->dma_buf && drvdata->dma_handle) {
		phys = drvdata->dma_handle + offset;
		pfn = phys >> PAGE_SHIFT;
		if (!remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
			DBG_PRN("esam_mmap: mapped dma_buf phys=%pa pfn=0x%lx size=%lu\n", &phys, pfn, size);
			return 0;
		}
		/* fall through to per-page if single remap fails */
	}
#endif
	/* 2) Since buffer is allocated via alloc_pages (physically contiguous), map using page_to_pfn */
	if (drvdata->mmap_page) {
		unsigned long start_pfn = page_to_pfn(drvdata->mmap_page) + (offset >> PAGE_SHIFT);
		if (!remap_pfn_range(vma, vma->vm_start, start_pfn, size, vma->vm_page_prot)) {
			DBG_PRN("esam_mmap: alloc_pages fast mapped virt=%p start_pfn=0x%lx size=%lu\n",
					vaddr + offset, start_pfn, size);
			return 0;
		}
		/* fall through to per-page if single remap fails */
	}

	/* 3) Slow path: per-page mapping (covers vmalloc and non-contiguous cases) */
	npages = DIV_ROUND_UP(size, PAGE_SIZE);
	for (i = 0; i < npages; i++) {
		void *page_virt = vaddr + offset + i * PAGE_SIZE;
		struct page *page = vmalloc_to_page(page_virt);
		unsigned long page_pfn;
		unsigned long map_len = min((unsigned long)PAGE_SIZE, size - i * PAGE_SIZE);

		if (page)
			page_pfn = page_to_pfn(page);
		else {
			phys = virt_to_phys(page_virt);
			page_pfn = phys >> PAGE_SHIFT;
		}

		if (remap_pfn_range(vma, vma->vm_start + i * PAGE_SIZE, page_pfn, map_len, vma->vm_page_prot)) {
			DBG_PRN("esam_mmap: per-page remap failed at page %lu\n", i);
			return -EAGAIN;
		}
	}

	DBG_PRN("esam_mmap: per-page mapped virt=%p size=%lu pages=%lu\n", vaddr + offset, size, npages);
	return 0;
}

#if 0
static int esam_mmap(struct file *filep, struct vm_area_struct *vma)                                               
{                                                                                                                  
        unsigned long page;                                                                                        
        struct cpld_spi_cmd_priv *drvdata = filep->private_data;                                                                  
                                                                                                                   
        page = virt_to_phys(drvdata->mmap_start);                                                                        
        if(remap_pfn_range(vma, vma->vm_start, page >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot))
                return -EAGAIN;                                                                                    
                                                                                                                   
        return 0;                                                                                                  
}
#endif
#if 0
static int esam_mmap(struct file *filep, struct vm_area_struct *vma)
{
	struct cpld_spi_cmd_priv *drvdata = filep->private_data;
	unsigned long size, offset, i, npages;
	void *vaddr;
	phys_addr_t phys;
	unsigned long *pfns = NULL;
	int ret = 0;

	if (!drvdata || !drvdata->mmap_start)
		return -EINVAL;

	offset = (unsigned long)(vma->vm_pgoff) << PAGE_SHIFT;
	size = vma->vm_end - vma->vm_start;

	vaddr = drvdata->mmap_start;
	/* ensure mapping range inside allocated buffer */
	if (offset + size > RECORD_BUF_SIZE_MAX)
		return -EINVAL;

	/* Fast path: try to map the whole contiguous area at once (works for kmalloc). */
	phys = virt_to_phys(vaddr + offset);
	ret = remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT, size, vma->vm_page_prot);
	if (ret == 0)
		return 0;

	/* Slow path: per-page mapping. Pre-check PFNs first to avoid partial mapping. */
	npages = DIV_ROUND_UP(size, PAGE_SIZE);
	pfns = kcalloc(npages, sizeof(unsigned long), GFP_KERNEL);
	if (!pfns)
		return -ENOMEM;

	for (i = 0; i < npages; i++) {
		void *page_addr = vaddr + offset + i * PAGE_SIZE;
		struct page *page = vmalloc_to_page(page_addr);

		if (page) {
			pfns[i] = page_to_pfn(page);
		} else {
			/* Not vmalloc page - try virt_to_phys for this kernel virtual addr */
			phys = virt_to_phys(page_addr);
			pfns[i] = phys >> PAGE_SHIFT;
		}
	}

	/* Now perform per-page remap */
	for (i = 0; i < npages; i++) {
		unsigned long map_len = min((unsigned long)PAGE_SIZE, size - i * PAGE_SIZE);
		if (remap_pfn_range(vma, vma->vm_start + i * PAGE_SIZE, pfns[i], map_len, vma->vm_page_prot)) {
			ret = -EAGAIN;
			goto out_free;
		}
	}

out_free:
	kfree(pfns);
	return ret;
}
#endif
                                                                                                               
static struct file_operations esam_driver_ops = {
	.owner = THIS_MODULE,
	.open = esam_open,
	.release = esam_release,
	.write = esam_write,
	.read = esam_read,
#ifdef CONFIG_COMPAT
	.compat_ioctl = esam_compat_ioctl,		//32位用户程序调用64位内核需要实现compat_ioctl
#endif
	.unlocked_ioctl = esam_ioctl,		//32位用户空间调用32位内核或者64位用户空间调用64位内需要实现unlocked_ioctl
	.fasync = esam_fasync,
	.mmap = esam_mmap,
};                                                                                                               

static int spiadc_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct cpld_spi_cmd_priv *priv;
	int ret = 0;
	// unsigned int irq_flags = 0;
	int i = 0;

	if (spi->bits_per_word && (spi->bits_per_word != 8)) {
		dev_err(dev, "bad word length\n");
		return -EINVAL;
	} else {
		dev_info(dev, "SPI word length: %d\n", spi->bits_per_word);
	}

	if (spi->max_speed_hz > SPI_CMD_MAX_HZ) {
		dev_err(dev, "speed is too high\n");
		return -EINVAL;
	} else {
		dev_info(dev, "SPI max speed: %d(Hz)\n", spi->max_speed_hz);
	}
	 
	if (spi->mode != SPI_MODE_0) {
		dev_err(dev, "bad mode\n");
		return -EINVAL;
	} else {
		dev_info(dev, "SPI mode: %d\n", spi->mode);
	}

	priv = devm_kzalloc(dev, sizeof(struct cpld_spi_cmd_priv), GFP_KERNEL);
	if (NULL == priv) {
		dev_err(dev, "%d: kzalloc failed\n", __LINE__);
		return -ENOMEM;
	}
		
	priv->spi      = spi;	
	priv->dev_no   = MKDEV(ESAM_DEV_MAJOR, 0);;
	priv->dev      = dev;
	priv->irq_adc  = spi->irq;
	dev_info(dev, "IRQ get from SPI: %d\n", priv->irq_adc);

	spi_set_drvdata(spi, priv);
	mutex_init(&priv->m_lock);

#if 0
	//申请SPI发送和接收BUFFER
	priv->spi_tx_buf = devm_kzalloc(dev, SPI_BYTE_MAX_LEN, GFP_KERNEL);
	if (!priv->spi_tx_buf) {
		dev_err(dev, "SPI TX buffer malloc failed.\n");
		return -ENOMEM;
	}

	priv->spi_rx_buf = devm_kzalloc(dev, SPI_BYTE_MAX_LEN, GFP_KERNEL);
	if (!priv->spi_rx_buf) {
		dev_err(dev, "SPI RX buffer malloc failed.\n");
		return -ENOMEM;
	}
#endif

	/* Allocate physically contiguous pages using alloc_pages so we can do a single remap */
	priv->mmap_order = get_order(RECORD_BUF_SIZE_MAX);
	priv->mmap_page = alloc_pages(GFP_KERNEL, priv->mmap_order);
	if (!priv->mmap_page) {
		dev_err(dev, "alloc_pages failed order=%u size=%u\n", priv->mmap_order, RECORD_BUF_SIZE_MAX);
		return -ENOMEM;
	}
	priv->mmap_start = page_address(priv->mmap_page);
	priv->mmap_size = (1UL << (PAGE_SHIFT + priv->mmap_order));
	if (priv->mmap_size < RECORD_BUF_SIZE_MAX)
		priv->mmap_size = RECORD_BUF_SIZE_MAX; /* ensure size is at least requested */

	dev_info(dev, "alloc_pages address start: 0x%p, alloc_size: %lu(KB)\n", priv->mmap_start, (priv->mmap_size/1000));

	priv->spi_rx_buf = (u8 *)priv->mmap_start;

	//初始化字符型设备
	cdev_init(&priv->cdev, &esam_driver_ops);
	priv->cdev.owner = THIS_MODULE;
	priv->cdev.ops   = &esam_driver_ops;
	ret = cdev_add(&priv->cdev, priv->dev_no, 1);
	if (ret) {
		dev_err(dev, "cdev add failed: %d\n", ret);
		return ret;
	}

	//此处注册到/sys/class
	priv->dev_class = class_create(THIS_MODULE, ESAM_CLASS_NAME);
	if (IS_ERR(priv->dev_class)) {
		ret = PTR_ERR(priv->dev_class);
		dev_err(dev, "failed to allocate class: %d\n", ret);
		goto cdev_err1;
	}

	//创建一个sysfs dev
	priv->sysfs_dev = device_create(priv->dev_class, NULL, priv->dev_no, "spiadc", ESAM_DEVNAME);
	if (IS_ERR(priv->sysfs_dev)) {
		ret = PTR_ERR(priv->sysfs_dev);
		dev_err(dev, "Failed to create sysfs_dev: %d\n", ret);
		goto cdev_err2;
	}

	//此处是为了将drvdata的指针放入到sysfs_dev中，为了将来可以用dev_get_drvdata来取出drvdata结构数据
	dev_set_drvdata(priv->sysfs_dev, (void *)priv);

	//接下来申请中断，B码中断需要放到probe函数中，这样系统一上电就可以进行对时，而不需要等到打开设备。
	// irq_flags = irq_get_trigger_type(priv->irq_adc);
	// if (irq_flags == IRQF_TRIGGER_NONE) {                     
		// dev_warn(dev, "WARNING: no IRQ resource flags set.\n");                                
		// irq_flags = IRQF_TRIGGER_FALLING;	//默认下降沿触发
	// } 

	// irq_flags |= IRQF_ONESHOT;

	// irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
	// 
	// ret = devm_request_threaded_irq(dev, priv->irq_adc, NULL, adc_irq_handler, irq_flags, "IRQ_SPIADC", (void *)priv);
	// if (ret < 0) {
		// dev_err(dev, "Request IRQ_SPIADC interrupt failed: %d\n", ret);
		// goto cdev_err3;
	// }
	// dev_info(dev, "Request IRQ_SPIADC IRQ: %d success, IRQ flags: 0x%x\n", priv->irq_adc, irq_flags);

	//初始化GPIO
	priv->gpiod_reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);	
	if (IS_ERR(priv->gpiod_reset)) {
		dev_err(dev, "failed to request ADC reset GPIO\n");
		goto cdev_err3;
	}
	dev_info(dev, "GPIO: ADC reset request success!\n");
	adc_reset(priv);

#if 1
	priv->gpiod_convst = devm_gpiod_get(dev, "convst", GPIOD_OUT_HIGH);	
	if (IS_ERR(priv->gpiod_convst)) {
		dev_err(dev, "failed to request ADC convst GPIO\n");
		goto cdev_err3;
	}
	dev_info(dev, "GPIO: ADC convst request success!\n");

	adc_sample_once(priv, 1);	//第一次采样
	mdelay(10);
	ret = spi_read(priv->spi, priv->spi_rx_buf, SPI_BYTE_MAX_LEN);
	if (ret < 0) {
		dev_err(dev, "SPI read failed: %d\n", ret);
		goto cdev_err3;
	}

	for(i = 0; i < ADC_CHANNEL_NUM; i++) {
		u16 *pdata = (u16 *)priv->spi_rx_buf;
		//DBG_PRN("ADC CH%02d: 0x%04x\n", i, (priv->spi_rx_buf[i*2] << 8) | priv->spi_rx_buf[i*2 + 1]);
		//DBG_PRN("ADC CH%02d: 0x%04x\n", i, be16_to_cpu(pdata[i]));

		//采用be16_to_cpu宏是为了兼容大小端模式，因为SPI传输过来的数据是大端模式
		dev_info(dev, "ADC CH%02d: 0x%04x\n", i, be16_to_cpu(pdata[i]));
	}
#endif

	//初始化工作队列
	INIT_WORK(&priv->spi_work, spi_adc_work_func);
	priv->target_cpu = 1; // 例如绑定到CPU1，可根据实际情况设置
    	priv->wq = alloc_workqueue("spi_adc_wq", WQ_UNBOUND | WQ_HIGHPRI, 1);
    	if (!priv->wq) {
    		dev_err(dev, "Failed to alloc workqueue\n");
    	    	goto cdev_err3;
    	}
	mdelay(10);

	//初始化全局变量
	priv->adc_state = 0;
	priv->offset = 0;
	priv->size = 0;
	priv->curr_pos = 0;

	hrtimer_init(&priv->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->hrtimer.function = my_hrtimer_callback;
	priv->interval = interval;
	priv->fsync_overflow = HRTIMER_COUNT_TIME_FSYNC / (interval / HRTIMER_TIME_US);
	dev_info(dev, "ADC sample interval: %ld us, sample_times: %d.\n", 
		priv->interval / HRTIMER_TIME_US, priv->fsync_overflow);
	hrtimer_start(&priv->hrtimer, ns_to_ktime(priv->interval), HRTIMER_MODE_REL);
	priv->adc_state = 1;
	dev_info(dev, "ADC sample start OK.\n");
	dev_info(dev, "Initialized success.\n");
	return 0;

cdev_err3:	
	device_del(priv->sysfs_dev);
	device_destroy(priv->dev_class, priv->dev_no);
	/* free allocated pages if any */
	if (priv->mmap_page) {
		__free_pages(priv->mmap_page, priv->mmap_order);
		priv->mmap_page = NULL;
	}

cdev_err2:
	class_destroy(priv->dev_class);

cdev_err1:
	cdev_del(&priv->cdev);
	return -1;
}

static int spiadc_remove(struct spi_device *spi)
{
	int ret = -1;
	struct cpld_spi_cmd_priv *drvdata = spi_get_drvdata(spi);
	struct device *dev = &spi->dev;

	ret = hrtimer_cancel( &drvdata->hrtimer );
	if (ret)
		dev_info(dev, "The timer was still in use...\n");

	if (drvdata->wq)
		destroy_workqueue(drvdata->wq);

	if (drvdata->sysfs_dev)  
		device_del(drvdata->sysfs_dev);

	if (drvdata->dev_class) {
		device_destroy(drvdata->dev_class, drvdata->dev_no);
		class_destroy(drvdata->dev_class);
	}

	if (drvdata->cdev.ops) {
		cdev_del(&drvdata->cdev);
	}

	/* free allocated pages */
	if (drvdata->mmap_page) {
		__free_pages(drvdata->mmap_page, drvdata->mmap_order);
		drvdata->mmap_page = NULL;
	}

	spi_set_drvdata(spi, NULL);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id spiadc_dt_ids[] = {
	{ .compatible = "sanway,spiadc", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, spiadc_dt_ids);
#endif

/* 补充 SPI 设备 ID 表，提供 spi: 前缀 modalias 匹配，解决 "spi:pm00mina" 无法自动加载的问题 */
static const struct spi_device_id spiadc_ids[] = {
	{ "spiadc", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, spiadc_ids);

static struct spi_driver sanway_spiadc_driver = {
	.driver.name	= SPI_CMD_DRV_NAME,
	.driver.of_match_table = of_match_ptr(spiadc_dt_ids),
	.id_table	= spiadc_ids,
	.probe		= spiadc_probe,
	.remove		= spiadc_remove,
};

module_spi_driver(sanway_spiadc_driver);

MODULE_DESCRIPTION("SANWAY SPI ADC driver for Wanlida DM-200");
MODULE_AUTHOR("RAY.Wang");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("spi:spiadc");

