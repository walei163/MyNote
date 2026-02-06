# SDK3.0版本调试问题汇总

## 2024-11-08：

### 1、配置SPL，以支持从`SD卡`或者`QSPI Flash`启动：

#### 1.1. 从SD卡启动：

- 需要配置`u-boot-810/spl/`下的menuconfig：

```sh
make CROSS_COMPILE=csky-abiv2-linux- menuconfig
```

- 进入到menuconfig界面，进行如下设置：

```sh
SPL / TPL  --->
	[*] Enable SPL                                                                                                                     SPL Boot media (Load SPL from SD Card)  --->
```

```sh
CSKY architecture  --->
	[*] Boot CK860 CPU
```

设置完成后，重新编译SPL：

```sh
make CROSS_COMPILE=csky-abiv2-linux- -j8
```

这一步会在`./spl`目录下生成：`u-boot-spl.bin`文件，然后再转为FUXI格式的`u-boot-spl.brn`格式：

```sh
/media/jason/btrfs_disk_M/Develop/FUXI-H_20240713/v3.0_sdk_20240810/tools/makefw -l0 -f 0x00 -r 0xa04c0000 -o u-boot-spl.brn -p spl/u-boot-spl.bin
```

- 根据FUXI文档：[《FUXI-H2 量产烧录指南_V1.0.pdf》](/media/jason/btrfs_disk_M/Develop/FUXI-H_20240713/FUXI-H2-SDK-V3.0/2、文档/软件开发/量产烧录指南/FUXI-H2 量产烧录指南_V1.0.pdf)，2.1 方案一（SD卡升级方案）：

还需要将`u-boot-spl.brn`重命名为：`spl.brn`，并复制到SD卡的`fat32`分区下。

至此，配置完成。

#### 1.2. 从QSPI Flash启动：

- 进入到menuconfig界面，进行如下设置：

```sh
SPL / TPL  --->
	[*] Enable SPL                                                                                                                     SPL Boot media (Load SPL from SPI flash)  --->
```

```sh
CSKY architecture  --->
	[ ] Boot CK860 CPU
```

如图所示：

![image-20241109095346530](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/image-20241109095346530.png)

（选择后，宏定义为：`CSKY_SPL_SPI_BOOT`）

![image-20241109095234609](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/image-20241109095234609.png)

（选择后，宏定义为：`CONFIG_BOOT_CK860`）



> [!note]
>
> **一定要把“Boot CK860 CPU”设置选项去掉！**

设置完成后，重新编译SPL：

```sh
make CROSS_COMPILE=csky-abiv2-linux- -j8
```

这一步会在`./spl`目录下生成：`u-boot-spl.bin`文件，然后再转为FUXI格式的`u-boot-spl.brn`格式：

```sh
/media/jason/btrfs_disk_M/Develop/FUXI-H_20240713/v3.0_sdk_20240810/tools/makefw -l0 -f 0x00 -r 0xa04c0000 -o u-boot-spl.brn -p spl/u-boot-spl.bin
```

- 用SD启动卡启动系统，然后将`u-boot-spl.brn`文件烧写到系统的QSPI1的对应mtd分区中（分区名称为：`"fsbl"`）：

```sh
root@CSG-FUXI:~# cat /proc/mtd
dev:    size   erasesize  name
mtd0: 01000000 00010000 "fault recording"
mtd1: 00100000 00010000 "fsbl"
mtd2: 00040000 00010000 "nvm"
mtd3: 00020000 00010000 "baremetal803.0"
mtd4: 00020000 00010000 "baremetal803.1"
mtd5: 00080000 00010000 "bootloader env"
mtd6: 00100000 00010000 "bootloader"
mtd7: 00100000 00010000 "baremetal1"
mtd8: 00080000 00010000 "baremetal2"
mtd9: 00080000 00010000 "device tree"
mtd10: 00b00000 00010000 "kernel"
```

> [!tip]
>
> 还需要烧写两个文件：
>
> `taeho-8100.img`：需要烧写到mtd对应分区（名称：`"baremetal2"`）中
>
> `taeho-8101.img`：需要烧写到mtd对应分区（名称：`"bootloader"`）中

至此，SPL烧写完成。

### 2、修改SPL的代码，以满足DDR3的启动频率：

- 修改`u-boot-810/board/sec-chip/fx6evb/spl.c`文件：

```c
//RAY.Wang: 2024-11-05，增加定义，测试并确定DDR3的运行频率。
static int ddr3_speed = DDR_SPEED;
//static int ddr3_speed_array[] = {
//	1600, 1333, 1066, 800
//};
//#define DDR_SPEED_ARRAY_NR_MAX		4

static int ddr3_speed_array[] = {
	1600, 1333, 1066, 800
};
#define DDR_SPEED_ARRAY_NR_MAX		4

...
    
void board_init_f(ulong dummy)
{
	struct pll_cfg freq_cfg;
	int i = 0;
	int j = 0;
	int err_times = 0;

	/* iomux */
	board_early_init_f();
	/* interrupt init */
	interrupt_init();
	//Enable WDT0
	*(volatile uint32_t *)(0xa0201000) |= 0x10;
	*(volatile uint32_t *)(0xa0201014) = 0xfffffff;

	*(volatile uint32_t *)(0xa0103000) &= ~(0x1);
	*(volatile uint32_t *)(0xa0103004) = 0xa << 4 | 0xa;
	*(volatile uint32_t *)(0xa0103000) = 0x4;
	//RAY.Wang: 2024-11-08，将此处打开看门狗先屏蔽掉
//	*(volatile uint32_t *)(0xa0103000) |= 0x1;

	/* system clock change */
	freq_cfg.pll_ctrl0 = PLL_CTRL0;
	freq_cfg.pll_ctrl1 = PLL_CTRL1;
	freq_cfg.pll_ctrl2 = PLL_CTRL2;
	freq_cfg.clk_ratio_ctrl = CLK_RATIO_CTRL;
	frequency_conversion(&freq_cfg);
	pre_log_puts("frequency conversion completed!\n");

	/* ddr initialization */
//	ddr_result = ddr_setup(DDR_SPEED);
//	if (!ddr_result)	/* if success */
//		pre_log_puts("DDR setup OK!\n");
//	else
//		pre_log_puts("DDR setup FAIL!\n");

//	for (i = 0; i < DDR_SPEED_ARRAY_NR_MAX; i++) {
//	for (i = 0; i < ARRAY_SIZE(ddr3_speed_array); i++) {
//		ddr3_speed = ddr3_speed_array[i];
//		ddr_result = ddr_setup(ddr3_speed);
//
//		if (!ddr_result) {
//			break;
//		}
//	}

	for (i = 0; i < ARRAY_SIZE(ddr3_speed_array); i++) {

		ddr3_speed = ddr3_speed_array[i];
		err_times = 0;

		for (j = 0; j < 1; j++) {
			ddr_result = ddr_setup(ddr3_speed);
			if (ddr_result < 0) {
				err_times++;
			}
		}

		if (err_times == 0){
			break;
		}
	}

	if (i >= DDR_SPEED_ARRAY_NR_MAX)
		pre_log_puts("CPU810 DDR setup FAIL!\n");
//	else if (!ddr_result)	/* if success */
	else if (err_times == 0)     /* if success */
		pre_log_puts("CPU810 DDR setup OK!\n");
	else
		pre_log_puts("CPU810 DDR setup FAIL!\n");


	//modify here if you want to close devices clock
	//*(volatile uint32_t *)0xa0201000 = 0x10;			/*enable clock gate control*/

	// force all clock
	// *(volatile uint32_t *)0xa0201028 = 0xFFFFFFFF;	/*FICR*/
	// *(volatile uint32_t *)0xa0201058 = 0xFFFFFFFF;	/*FICR_SUB*/
	// *(volatile uint32_t *)0xa020113C = 0xFFFFFFFF;	/*FICR1*/

	// open clock
	// *(volatile uint32_t *)0xa0201014 = 0xF4B0F;		/*CGCR*/
	// *(volatile uint32_t *)0xa0201050 = 0x782F90E0;	/*CGCR_SUB*/
	// *(volatile uint32_t *)0xa0201134 = 0xB9272618;	/*CGCR1*/

	*(volatile uint32_t *)0xa0202004 = 0xa04c0000;
	*(volatile uint32_t *)0xa0202008 = 0xffff0000;
	*(volatile uint32_t *)0xa020200c = 0xa04d0000;
	*(volatile uint32_t *)0xa0202010 = 0xffff0000;

	*(volatile uint32_t *)0xa0202014 = 0xa04f0000;
	*(volatile uint32_t *)0xa0202018 = 0xffff0000;
	*(volatile uint32_t *)0xa020201c = 0xa04e0000;
	*(volatile uint32_t *)0xa0202020 = 0xffff0000;
}
```

- 添加SPL上电指示灯代码：

```c
void spl_board_init(void)
{
	preloader_console_init();
	console_init_f();
	print_pre_log_buffer();
//	printf("CPU %dHz, AHB %dHz, APB %dHz, DDR %dMHz\n",
//	       CPU_DEFAULT_FREQ, HSP_DEFAULT_FREQ,
//	       LSP_DEFAULT_FREQ, DDR_SPEED);

	printf("CPU %dHz, AHB %dHz, APB %dHz, DDR %dMHz\n",
	       CPU_DEFAULT_FREQ, HSP_DEFAULT_FREQ,
	       LSP_DEFAULT_FREQ, ddr3_speed);

	//Reset WDT0 IP to close WDT0
	*(volatile uint32_t *)(0xa0201038) &= ~(0x1 << 6);
	for(uint32_t i = 0; i < 100; i++)
			asm("mov r0, r0");
	*(volatile uint32_t *)(0xa0201038) |= (0x1 << 6);
	if (ddr_result)
		while (1);	/* if ddr training fail */
		
#if 0
	clkgate_other_core(CK8031);
	__raw_writel(CK8031_STOPPED, CK803_0_STOPPING);
	clkgate_other_core(CK8030);
	clkgate_other_core(CK810);
	*(volatile unsigned int *)(0xa0201000) = 0x19;
#endif

//RAY.Wang: 2024-11-07
//点灯操作，如果GPIO2_5和GPIO2_6点灯操作成功，表示板卡内存初始化成功，进入加载U-boot阶段。
#if 1
#define GPIO2_SWPORTA_DR	0xa0387000	//GPIO2端口A数据寄存器
#define GPIO2_SWPORTA_DDR	0xa0387004	//GPIO2端口A数据流向寄存器

		*(volatile uint32_t *)(GPIO2_SWPORTA_DR)  |= (0x3 << 5);
		*(volatile uint32_t *)(GPIO2_SWPORTA_DDR) |= (0x3 << 5);
#endif
}
```

- 修改`u-boot-810/board/sec-chip/fx6evb/ddr_init.c`文件：

```c
int ddr_setup(int ddr_speed)
{
	int value;
	int i;
    
    ...
        
    //	if ((ddr_training() != 0) || (ddr_three_step_test() != 0))
//		return -1;	/* ddr training failure */
//	else
//		return 0;

	//RAY.Wang: 2024-11-08
	//此处需要让ddr training多进行几次，以避免吗出现偶发的ddr speed选择错误问题
	for (i = 0; i < 5; i++) {
		if ((ddr_training() != 0) || (ddr_three_step_test() != 0))
			return -1;	/* ddr training failure */
	}

	return 0;	/* ddr training success */
}
```



## 2024-10-31：

修改`motorcomm.c`，修正网络连接时的显示错误。

原来未修改的代码，在千兆以太网连接时，会出现如下显示信息：

```sh
[   14.500678] sc gmac a0580000.ethernet eth0: yt8521_read_status, phy addr: 3, link up, media: FIBER, mii reg 0x11 = 0xbc40
[   14.528835] sc gmac a0580000.ethernet eth0: yt8521_aneg_done, phy addr: 3, link_fiber: 1, link_utp: 0
```

也就是连接`RJ45`接口时，会显示光纤连接模式，需要修改代码，如下：

```c
//RAY.Wang: 2024-10-31，重新改写yt8521_aneg_done函数
static int yt8521_aneg_done(struct phy_device *phydev)
{
	struct yt8xxx_priv *priv = phydev->priv;
	int link_fiber = 0, link_utp = 0;

	if (priv->polling_mode == YT_PHY_MODE_FIBER) {
		/* reading Fiber */
		ytphy_write_ext(phydev, 0xa000, 2);
		link_fiber = !!(phy_read(phydev, REG_PHY_SPEC_STATUS) & (BIT(YTXXXX_LINK_STATUS_BIT)));
	} else if (priv->polling_mode == YT_PHY_MODE_UTP) {

		/* reading UTP */
		ytphy_write_ext(phydev, 0xa000, 0);
		link_utp = !!(phy_read(phydev, REG_PHY_SPEC_STATUS) & (BIT(YTXXXX_LINK_STATUS_BIT)));
	} else {
		netdev_err(phydev->attached_dev, "%s: Unknown Phy polling mode.\n", __func__);
		return !!(link_fiber | link_utp);
	}

#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	netdev_info(phydev->attached_dev, "%s, phy addr: %d, link_fiber: %d, link_utp: %d\n",
		__func__, phydev->addr, link_fiber, link_utp);
#else
	netdev_info(phydev->attached_dev, "%s, phy addr: %d, link_fiber: %d, link_utp: %d\n",
		__func__, phydev->mdio.addr, link_fiber, link_utp);
#endif
	return !!(link_fiber | link_utp);
}
```

其中，`priv->polling_mode`就表示设定当前的网络接口模式，如果硬件设计是固定的，比如`UTP`模式，则可与直接固定该模式：

```c
static int yt8521_probe(struct phy_device *phydev)
{
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	struct device *dev = &phydev->dev;
#else
	struct device *dev = &phydev->mdio.dev;
#endif
	struct yt8xxx_priv *priv;
	int chip_config;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	chip_config = ytphy_read_ext(phydev, 0xa001);

	priv->chip_mode = chip_config & 0x7;
	switch (priv->chip_mode) {
	case 1:	//fiber<>rgmii
	case 4:
	case 5:
		priv->polling_mode = YT_PHY_MODE_FIBER;
		break;
	case 2:	//utp/fiber<>rgmii
	case 6:
	case 7:
		priv->polling_mode = YT_PHY_MODE_POLL;
		break;
	case 3:	//utp<>sgmii
	case 0:	//utp<>rgmii
	default:
		priv->polling_mode = YT_PHY_MODE_UTP;
		break;
	}

	//将PHY模式直接设置为UTP模式
	priv->polling_mode = YT_PHY_MODE_UTP;
	
	return 0;
}
```

按照以上修改完代码后，网络显示信息如下：

```sh
[  418.404669] sc gmac a0580000.ethernet eth0: yt8521_read_status, phy addr: 3, link up, media: UTP, mii reg 0x11 = 0xbc40
[  418.416104] sc gmac a0580000.ethernet eth0: yt8521_aneg_done, phy addr: 3, link_fiber: 0, link_utp: 1
[  418.425397] sc gmac a0580000.ethernet eth0: Link is Up - 1Gbps/Full - flow control off
[  418.433459] IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
```



## 2024-10-29:

### 1、文件系统支持`syslinux`模式加载：

刘天鹏这边修改`u-boot`代码，使其支持系统的`syslinux`加载模式。

在/boot目录下建立`extlinux`目录，并在其下再建立一个叫：`extlinux.conf`的文件。内容如下：

```sh
# Generic Distro Configuration file generated by OpenEmbedded
menu title Select the boot mode
MENU BACKGROUND /splash.bmp
TIMEOUT 20
DEFAULT fuxi_emmc

LABEL fuxi_emmc
	KERNEL /boot/uImage
	FDT /boot/sanway_fx6evb_860mp_PDM_sichuang_emmc.dtb
	APPEND root=/dev/mmcblk1p1 rw rootwait rootfstype=ext4 console=ttyS0,115200

LABEL fuxi_sdcard
	KERNEL /boot/uImage
	FDT /boot/sanway_fx6evb_860mp_PDM_sichuang_sdcard.dtb
	APPEND root=/dev/mmcblk0p2 rw rootwait rootfstype=ext4 console=ttyS0,115200
	
LABEL fuxi_nfs
	KERNEL /boot/uImage
	FDT /boot/sanway_fx6evb_860mp_PDM_sichuang_nfs.dtb
	APPEND console=ttyS0,115200n8 root=/dev/nfs nfsroot=192.168.1.177:/media/ubuntu16.04/mnt/fuxi_rootfs_sanway_20241022,nolock,nfsvers=3 rw ip=192.168.1.223:192.168.1.177:192.168.1.1:255.255.255.0::eth0:off
```



### 2、今天又发现千兆以太网的一处bug：

采用FTP文件传输时，从电脑上传到板子上的速率没有问题，但是从板子向电脑下载一个较大的文件时，传输非常慢。

电脑到板卡传输是走的千兆以太网的RX通道，而板卡到电脑传输是走的千兆以太网的TX通道。

怀疑需要在千兆以太网的TX通道上添加tx_delay延迟才行。

查看YT8531S的数据手册，有这么一段内容：

![image-20241029145231276](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/image-20241029145231276.png)

也就是说，需要修改`Tx_delay_sel`来满足延迟条件。

修改内核驱动：`motorcomm.c`：

```c
//LED配置寄存器，在扩展页上
#define LED0_CFG_ADDRESS	0xa00c
#define LED1_CFG_ADDRESS	0xa00d
#define LED2_CFG_ADDRESS	0xa00e
#define SYNCE_CFG		0xa012
#define CHIP_CONFIG_REG		0xa001
#define RGMII_CONFIG1_REG	0xa003

static int yt8521_config_init(struct phy_device *phydev)
{
    ...
    //RAY.Wang: 设置CHIP_CONFIG寄存器（0xa001）：去掉rx_delay 2ns延时，
	//根据FUXI数据手册中描述：RX链路上已经在MAC端做Delay，PHY端不建议做Delay。
	val = ytphy_read_ext(phydev, CHIP_CONFIG_REG);
	netdev_info(phydev->attached_dev, "CHIP_CONFIG_REG before read reg: 0x%x\n", val);

	val &= ~(1 << 8);
	ret = ytphy_write_ext(phydev, CHIP_CONFIG_REG, val);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, CHIP_CONFIG_REG);
	netdev_info(phydev->attached_dev, "CHIP_CONFIG_REG after read reg: 0x%x\n", val);

	//RGMII tx_delay设置：添加0xe（14）= 150ps x 14 = 2.1ns发送延迟
	val = ytphy_read_ext(phydev, RGMII_CONFIG1_REG);
	netdev_info(phydev->attached_dev, "RGMII_CONFIG1_REG before read reg: 0x%x\n", val);

	val &= 0xfff0;
	val |= 0xe;
	ret = ytphy_write_ext(phydev, RGMII_CONFIG1_REG, val);
	if (ret < 0)
		return ret;
	netdev_info(phydev->attached_dev, "RGMII_CONFIG1_REG after read reg: 0x%x\n", val);
}
```

> [!tip]
>
> 更新内核后重新上电，问题解决。

## 2024-09-03:

今天在上电启动时，又发现了新的问题，如下所示：

```sh
[    5.844751] sysfs: cannot create duplicate filename '/bus/mdio_bus/drivers/YT8510 100!10Mb Ethernet'
[    5.853970] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 4.19.15 #51
[    5.860072] Call Trace:
[    5.862543] [<80039f30>] walk_stackframe+0x0/0xc0
[    5.867259] [<8003a06e>] show_stack+0x22/0x38
[    5.871630] [<80699cf8>] dump_stack+0x68/0x98
[    5.875998] [<801b00d0>] sysfs_warn_dup+0x40/0x58
[    5.880711] [<801b0212>] sysfs_create_dir_ns+0xde/0xec
[    5.885860] [<8069f294>] kobject_add_internal+0x84/0x200
[    5.891180] [<8069f670>] kobject_init_and_add+0x48/0x70
[    5.896424] [<803b4776>] bus_add_driver+0xce/0x1b8
[    5.901229] [<803b5e7e>] driver_register+0x4e/0xc4
[    5.906031] [<8040070c>] phy_driver_register+0x34/0x5c
[    5.911179] [<804007f4>] phy_drivers_register+0x3c/0x90
[    5.916417] [<800192d2>] phy_module_init+0x16/0x30
[    5.921222] [<8003772a>] do_one_initcall+0x4e/0x198
[    5.926110] [<80001c2c>] kernel_init_freeable+0x134/0x260
[    5.931526] [<806af010>] kernel_init+0x14/0xe8
[    5.935980] [<800380f8>] ret_from_kernel_thread+0x8/0x10
[    5.941360] kobject_add_internal failed for YT8510 100!10Mb Ethernet with -EEXIST, don't try to register things with the same name in the same directory.
[    5.955130] libphy: YT8510 100/10Mb Ethernet: Error -17 in registering driver
```

显示YT8510的注册并且崩溃信息。经过研究，发现motocomm的PHY驱动，默认是将千兆和百兆的驱动全部编译进内核的：

```sh
obj-$(CONFIG_MOTORCOMM_PHY) 	+= motorcomm.o motorcomm_100m.o
```

而在motorcomm_100m.c代码中，默认会加载YT8510的PHY驱动。我们的硬件设计上只用到了YT8531S，没有YT8510百兆以太网PHY。

**因此，可以将这两个PHY驱动分开配置和编译，用到哪个就将哪个编译进内核。**

修改方法：

1、修改`/drivers/net/phy/Kconfig`文件：

```sh
config MOTORCOMM_GIGABIT_PHY
	tristate "Motorcomm Gigabit PHYs"
	default y
	---help---
	  SUpports the YT8521,YT8531,YT8531S PHYs.

config MOTORCOMM_PHY
	tristate "Motorcomm PHYs"
	default n
	---help---
	  SUpports the YT8010,YT8510,YT8511,YT8512 PHYs.
```

增加`config MOTORCOMM_GIGABIT_PHY`选项。表示为千兆PHY配置。

2、修改`drivers/net/phy/Makefile`文件：

```sh
obj-$(CONFIG_MOTORCOMM_GIGABIT_PHY) 	+= motorcomm.o
obj-$(CONFIG_MOTORCOMM_PHY) 	+= motorcomm_100m.o
```

然后重新配置内核，去掉`config MOTORCOMM_PHY`选项，只选择`config MOTORCOMM_GIGABIT_PHY`，然后重新编译内核，烧写到板子上。

错误提示信息就没有了。问题解决。

## 2024-08-30:

千兆以太网设计上还是有问题，采用YT8521s PHY芯片时，万力达秋磊发现：

```sh
ifconfig eth0 down
ifconfig eth0 up
```

即将eth0这样操作之后，出现以下问题：

```sh
Failed to reset dma
stmmac_hw_setup: DMA engine initialization failed.
```

经过仔细研究，怀疑是YT8521 PHY芯片的clkout输出时钟不对，按照FUXI芯片数据手册的要求，`GE0_REF_CLK`必须要输入一个125MHz的时钟。默认上电时，YT8521的clkout脚会输出125MHz，但是ifconfig down再ifconfig up后，该脚没有时钟输出。怀疑问题出在这里。

更换YT8531S芯片，并联系了裕泰微电子人员，拿到最新的PHY驱动。经过一番调试，终于可以使用了。

需要修噶以下几个方面：

**1、硬件上，需要将YT8521更换为YT8531S，其它可以不用修改**。

**2、修改motocomm.c PHY驱动：**

```c
//LED配置寄存器，在扩展页上
#define LED0_CFG_ADDRESS	0xa00c
#define LED1_CFG_ADDRESS	0xa00d
#define LED2_CFG_ADDRESS	0xa00e
#define SYNCE_CFG		0xa012
#define CHIP_CONFIG_REG		0xa001

static int yt8521_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	struct yt8xxx_priv *priv = phydev->priv;

#if (YTPHY_WOL_FEATURE_ENABLE)
	struct ethtool_wolinfo wol;

	/* set phy wol enable */
	memset(&wol, 0x0, sizeof(struct ethtool_wolinfo));
	wol.wolopts |= WAKE_MAGIC;
	ytphy_wol_feature_set(phydev, &wol);
#endif

	phydev->irq = PHY_POLL;

	ytphy_write_ext(phydev, 0xa000, 0);
#if (KERNEL_VERSION(4, 0, 0) > LINUX_VERSION_CODE) || (KERNEL_VERSION(5, 3, 0) < LINUX_VERSION_CODE)
	ret = ytphy_config_init(phydev);
#else
	ret = genphy_config_init(phydev);
#endif
	if (ret < 0)
		return ret;

	/* disable auto sleep */
	val = ytphy_read_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8521_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	val = ytphy_read_ext(phydev, 0xc);
	if (val < 0)
		return val;
	val &= ~(1 << 12);
	ret = ytphy_write_ext(phydev, 0xc, val);
	if (ret < 0)
		return ret;

	//2024-08-30
	//RAY.Wang: 设置SYNCE_CFG寄存器（0xa012）：设置clkout输出125M
	//第6位、第5位和第4位均为1，第[3:1]位设置为：000，第0位为0.具体参见YT8531S的数据手册中描述。
	val = ytphy_read_ext(phydev, SYNCE_CFG);
	netdev_info(phydev->attached_dev, "SYNCE_CFG  before read reg: 0x%x\n", val);

	val = 0x70;
	ret = ytphy_write_ext(phydev, SYNCE_CFG, val);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, SYNCE_CFG);
	netdev_info(phydev->attached_dev, "SYNCE_CFG after read reg: 0x%x\n", val);

	//RAY.Wang: 设置CHIP_CONFIG寄存器（0xa001）：去掉rx_delay 2ns延时，
	//根据FUXI数据手册中描述：RX链路上已经在MAC端做Delay，PHY端不建议做Delay。
	val = ytphy_read_ext(phydev, CHIP_CONFIG_REG);
	netdev_info(phydev->attached_dev, "CHIP_CONFIG_REG before read reg: 0x%x\n", val);

	val &= ~(1 << 8);
	ret = ytphy_write_ext(phydev, CHIP_CONFIG_REG, val);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, CHIP_CONFIG_REG);
	netdev_info(phydev->attached_dev, "CHIP_CONFIG_REG after read reg: 0x%x\n", val);


#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	netdev_info(phydev->attached_dev, "%s done, phy addr: %d, chip mode = %d, polling mode = %d\n",
		__func__, phydev->addr, priv->chip_mode, priv->polling_mode);
#else
	netdev_info(phydev->attached_dev, "%s done, phy addr: %d, chip mode = %d, polling mode = %d\n",
		__func__, phydev->mdio.addr, priv->chip_mode, priv->polling_mode);
#endif
	return ret;
}
```

在上面的代码中，需要设置SYNCE_CFG（0xa012）寄存器，写入值为0x70，让YT8531S始终输出125MHz时钟。

需要设置CHIP_CONFIG（0xa001）寄存器，先读出该寄存器的值，然后将第8位清零，也就是去掉2ns RX_DELAY延时。

因为根据FUXI芯片数据手册的描述：

![image-20240830200120355](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/image-20240830200120355.png)

也就是说，在FUXI芯片的MAC端已经做了RX Delay，因此不需要在PHY端再做Delay。

> [!important]
>
> <font color=yellow> 在没有做修改CHIP_CONFIG寄存器之前，无论怎么设置dtb，eth0始终ping不通。后来偶然发现，百兆模式下，是可以ping通的，也就说明，应该是千兆模式的时钟延时问题。然后仔细查询手册，才发现FUXI的数据手册中有关于这一块的描述。</font>

**3、修改dtb设备文件树：`fx6_860mp.dtsi`：**

```c
gmac0: ethernet@a0580000 {			
	compatible = "fuxi,gmac";
	reg = <0xa0580000 0x2000>;
	interrupt-parent = <&intc>;
	interrupts = <62>;
	interrupt-names = "macirq";
	pinctrl-names = "default";
	pinctrl-0 = <&gmac0_rgmii_pins>, <&gmac0_mdio_pins>;
	clocks = <&csrclk>;
	clock-names = "stmmaceth";
	status = "disabled";
	max-speed = <1000>;
	phy-mode = "rgmii-txid";
	//snps,phy-addr = <0x00>;
	//phy-mode = "rgmii";
	//phy-mode = "rgmii-rxid";
	//snps,phy-addr = <0x2>;
	phy-handle = <&phy0>;
	snps,reset-gpio = <&port6 9 GPIO_ACTIVE_LOW>;
	snps,reset-active-low;
	snps,reset-delays-us = <0 20000 10000>;
	syscon = <&sysctrl 0x574>;			
	//tx_delay_mask = <0x0000000f>;
	// rx_delay_mask = <0x000000f0>;
	mac_if_select_mask = <0x0000000f>;
	//tx_delay = <0x00000000 0x00000000 0x00000000>;
	// rx_delay = <0x00000000 0x00000000 0x00000020>;
	mdio0 {
		#address-cells = <1>;
		#size-cells = <0>;
		compatible = "snps,dwmac-mdio";
		phy0: ethernet-phy@0 {
			//compatible = "ethernet-phy-id0000.011a";
			compatible = "ethernet-phy-id4f51.e91a";
			reg = <2>;
			// reg = <0>;
		};
	};
};
```

> [!note]
>
> <font color=yellow>注意：phy-mode = "rgmii-txid"; 表示不在MAC端做TX Delay。并且将所有与TX或者RX延时相关的内容全部注释掉。</font>

经过这样的修改后，eth0千兆以太网可以真正使用了。

## 2024-08-22：

1、目前发现，百兆以太网驱动`smac.ko`，SDK3.0版本自带的插入到内核中后，硬件上不能ping通。确定硬件是好的。

但是如果采用之前SDK2.4版本的`smac.ko`，则3路百兆以太网可以正常工作。**尚不清楚原因。**

2、`GMAC0`这一路也不能正常工作，需要修改dtb，并且同时需要修改内核驱动代码：`/drivers/net/phy/phy_device.c`。

但是采用之前SDK2.4版本的内核，则可以正常工作。**也不清楚原因**。   **（问题已找到，[见2024-08-23](#2024-08-23:)。）**

- dtb设备文件树修改：

```c
gmac0: ethernet@a0580000 {
	//compatible = "sec-chip,gmac";
	compatible = "fuxi,gmac";
	reg = <0xa0580000 0x2000>;
	interrupt-parent = <&intc>;
	interrupts = <62>;
	interrupt-names = "macirq";
	pinctrl-names = "default";
	pinctrl-0 = <&gmac0_rgmii_pins>, <&gmac0_mdio_pins>;
	// pinctrl-0 = <&gmac0_rgmii_pins>;
	clocks = <&csrclk>;
	clock-names = "stmmaceth";
	status = "disabled";
	max-speed = <1000>;
	//phy-mode = "rgmii-txid";
	//snps,phy-addr = <0x00>;
	phy-mode = "rgmii";
	//snps,phy-addr = <0x2>;
	phy-handle = <&phy0>;
	snps,reset-gpio = <&port6 9 GPIO_ACTIVE_LOW>;
	snps,reset-active-low;
	snps,reset-delays-us = <0 20000 10000>;
	syscon = <&sysctrl 0x574>;			
	tx_delay_mask = <0x0000000f>;
	rx_delay_mask = <0x000000f0>;
	mac_if_select_mask = <0x0000000f>;
	tx_delay = <0x00000000 0x00000000 0x00000000>;
	rx_delay = <0x00000000 0x00000000 0x00000020>;
	mdio0 {
		#address-cells = <1>;
		#size-cells = <0>;
		compatible = "snps,dwmac-mdio";
		phy0: ethernet-phy@0 {
			reg = <2>;
			// reg = <0>;
		};
	};
};
```

- 内核驱动代码`/drivers/net/phy/phy_device.c`修改：

```c
#define YT8521SH_PHY_ID		0x0000011a
#define YT8531S_PHY_ID		0x4f51e91a
#define YT8531_PHY_ID		0x4f51e91b
#define GMAC0_PHY_ADDR		2

struct phy_device *get_phy_device(struct mii_bus *bus, int addr, bool is_c45)
{
	struct phy_c45_device_ids c45_ids = {0};
	u32 phy_id = 0;
	int r;

	//pr_info("addr: %d, is_c45: %d\n", addr, is_c45);

	r = get_phy_id(bus, addr, &phy_id, is_c45, &c45_ids);
	if (r) {
		//pr_err("Get phy_id failed: 0x%x.\n", phy_id);
		return ERR_PTR(r);
	}

	//硬性写入YT8521的phy_id，硬件上设计YT8521的addr为2
//	if (((phy_id & 0x1fffffff) == 0x1fffffff) && (addr == GMAC0_PHY_ADDR)) {
//		phy_id = YT8521SH_PHY_ID;
//	}

	//pr_info("phy_id get value: 0x%x.\n", phy_id);
	/* If the phy_id is mostly Fs, there is no device there */
	if ((phy_id & 0x1fffffff) == 0x1fffffff) {

		if (addr == GMAC0_PHY_ADDR) {
			phy_id = YT8521SH_PHY_ID;
		} else {
			return ERR_PTR(-ENODEV);
		}
	}

	return phy_device_create(bus, addr, phy_id, is_c45, &c45_ids);
}
EXPORT_SYMBOL(get_phy_device);
```

> [!note]
>
> **经过这样改动后，4路网口可以正常工作。**

## 2024-08-23:

1、**千兆以太网的问题已经找到**，见代码：`drivers/of/of_mdio.c`中的函数：`of_get_phy_id`：

```c
static int of_get_phy_id(struct device_node *device, u32 *phy_id)
{
	struct property *prop;
	const char *cp;
	unsigned int upper, lower;

	of_property_for_each_string(device, "compatible", prop, cp) {
		if (sscanf(cp, "ethernet-phy-id%4x.%4x", &upper, &lower) == 2) {
			*phy_id = ((upper & 0xFFFF) << 16) | (lower & 0xFFFF);
			return 0;
		}
	}
	return -EINVAL;
}
```

<font face="微软雅黑" color=yellow>出现问题的时候，是上面这个函数没有读到YT8521 PHY的PHY_ID，根据上面的代码，其实可以在dtb中传递你所用的以太网PHY芯片的ID。</font>

格式如下：

```sh
ethernet-phy-idAAAA.BBBB
```

**其中，AAAA为PHY_ID的高8位，BBBB为PHY_ID的低8位。均为16进制格式。**

因此，我们可以修改dtb设备文件树中关于`gmac0`的配置如下：

```c
gmac0: ethernet@a0580000 {
	//compatible = "sec-chip,gmac";
	compatible = "fuxi,gmac";
	reg = <0xa0580000 0x2000>;
	interrupt-parent = <&intc>;
	interrupts = <62>;
	interrupt-names = "macirq";
	pinctrl-names = "default";
	pinctrl-0 = <&gmac0_rgmii_pins>, <&gmac0_mdio_pins>;
	// pinctrl-0 = <&gmac0_rgmii_pins>;
	clocks = <&csrclk>;
	clock-names = "stmmaceth";
	status = "disabled";
	max-speed = <1000>;
	//phy-mode = "rgmii-txid";
	//snps,phy-addr = <0x00>;
	phy-mode = "rgmii";
	//snps,phy-addr = <0x2>;
	phy-handle = <&phy0>;
	snps,reset-gpio = <&port6 9 GPIO_ACTIVE_LOW>;
	snps,reset-active-low;
	snps,reset-delays-us = <0 20000 10000>;
	syscon = <&sysctrl 0x574>;			
	tx_delay_mask = <0x0000000f>;
	rx_delay_mask = <0x000000f0>;
	mac_if_select_mask = <0x0000000f>;
	tx_delay = <0x00000000 0x00000000 0x00000000>;
	rx_delay = <0x00000000 0x00000000 0x00000020>;
	mdio0 {
		#address-cells = <1>;
		#size-cells = <0>;
		compatible = "snps,dwmac-mdio";
		phy0: ethernet-phy@0 {
			compatible = "ethernet-phy-id0000.011a";
			reg = <2>;
			// reg = <0>;
		};
	};
};
```

> [!important]
>
> 其中尤其要注意关于phy0的描述中有这么一句：
>
> ```c
> compatible = "ethernet-phy-id0000.011a";
> ```
>
> **内核中的代码函数：`of_get_phy_id`正是根据这一行的描述来生成PHY_ID。**
>
> **可以查阅对应以太网PHY芯片的datasheet，来确定PHY_ID，然后将其写入到dtb中。**
>
> 至此，问题解决。

2、<font face="微软雅黑" color=yellow>可以编译使用u-boot代码目录中`tools/env`下的工具：`fw_printenv`，来实现在Linux文件系统下对u-boot环境变量的读写与设置。</font>

2.1. u-boot下编译：

```sh
make CROSS_COMPILE=csky-abiv2-linux- envtools
```

编译成功后，会在`tools/env`下生成可执行文件：`fw_printenv`，这个是用来查看u-boot环境变量的。

对这个命令创建链接：

```sh
ln -s fw_printenv fw_setenv
```

生成的命令：`fw_setenv`是用来设置u-boot环境变量中的变量的。

2.2. 修改配置文件内容：`fw_env.config`如下：

```sh
# Configuration file for fw_(printenv/setenv) utility.
# Up to two entries are valid, in this case the redundant
# environment sector is assumed present.
# Notice, that the "Number of sectors" is not required on NOR and SPI-dataflash.
# Futhermore, if the Flash sector size is omitted, this value is assumed to
# be the same as the Environment size, which is valid for NOR and SPI-dataflash
# Device offset must be prefixed with 0x to be parsed as a hexadecimal value.

# NOR example
# MTD device name       Device offset   Env. size       Flash sector size       Number of sectors
/dev/mtd4               0x0000          0x80000         0x10000
#/dev/mtd2              0x0000          0x4000          0x4000

# MTD SPI-dataflash example
# MTD device name       Device offset   Env. size       Flash sector size       Number of sectors
#/dev/mtd5              0x4200          0x4200
#/dev/mtd6              0x4200          0x4200

# NAND example
#/dev/mtd0              0x4000          0x4000          0x20000                 2

# On a block device a negative offset is treated as a backwards offset from the
# end of the device/partition, rather than a forwards offset from the start.

# Block device example
#/dev/mmcblk0           0xc0000         0x20000
#/dev/mmcblk0           -0x20000        0x20000

# VFAT example
#/boot/uboot.env        0x0000          0x4000

# UBI volume
#/dev/ubi0_0            0x0             0x1f000         0x1f000
#/dev/ubi0_1            0x0             0x1f000         0x1f000

# UBI volume by name
#/dev/ubi0:env          0x0             0x1f000         0x1f000
#/dev/ubi0:env-redund   0x0             0x1f000         0x1f000
```

> [!tip]
>
> <font face="微软雅黑" color=cyan>修改完成后，将命令：fw_printenv，fw_setenv上传到FUXI文件系统的/usr/bin目录下，将配置文件：fw_env.config上传到FUXI文件系统的/etc/目录下，即可正常工作。</font>

## 2024-08-24：

### 1、串口调试：

根据`FUXI-H`核心板上的定义，一共引出了11路串口，分别为：

```sh
UART0, UART1, UART2, UART3, UART4, UART5, UART6, UART8, UART9, UART11, UART12
```

#### 1.1. dtb设备文件树的配置：

这11路串口在dtb中的配置见文件：`fx6_860mp.dtsi`（其中有些是我补充上的，原来的文件中没有这几路的定义）：

```c
uart0: serial@a0113000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0113000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <44>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart0_pins>;
	status = "disabled";
};
uart1: serial@a0114000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0114000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <45>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart1_pins>;
	status = "disabled";
};
uart2: serial@a0115000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0115000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <46>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart2_pins>;
	status = "disabled";
};
uart3: serial@a0116000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0116000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <47>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart3_pins>;
	status = "disabled";
};
uart4: serial@a0117000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0117000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <48>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart4_pins>;
	status = "disabled";
};
//RAY.Wang: 原来没有，我自己加上的。2022-04-23
//具体见文档：《FUXI-H2_User_Guide_Chinese_V1.2.pdf》
uart5: serial@a0383000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0383000 0x1000>;
	interrupt-parent = <&intc>;
	//interrupts = <47>;
	interrupts = <79>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart5_pins>;
	status = "disabled";
};
uart6: serial@a0384000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0384000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <80>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart6_pins>;
	status = "disabled";
};
uart8: serial@a0386000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0386000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <82>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart8_pins>;
	status = "disabled";
};
uart9: serial@a0914000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0914000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <112>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart9_pins>;
	status = "disabled";
};
uart12: serial@a0917000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0917000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <106>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart12_pins>;
	status = "disabled";
};
uart13: serial@a0918000 {
	compatible = "fuxi,apb-uart";
	reg = <0xa0918000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <107>;
	clocks = <&dummy_apb>;
	clock-names = "baudclk";
	reg-shift = <2>;
	reg-io-width = <4>;
	pinctrl-names = "default";
	pinctrl-0 = <&uart13_pins>;
	status = "disabled";
```

如果Linux下需要将串口编号与硬件一一对应固定编号，还需要在dtb中设置`alias`属性：

```c
aliases {
	ethernet0 = &gmac0;
	ethernet1 = &gmac1;
	ethernet5 = &smac0;
	//ethernet6 = &smac4;
	ethernet6 = &smac3;
	ethernet7 = &smac5;
	ethernet8 = &smac6;
	serial0 = &uart0;
	gpio0 = &gpio0;
	gpio1 = &gpio1;
	gpio3 = &gpio3;
	gpio4 = &gpio4;
	gpio5 = &gpio5;
	gpio6 = &gpio6;
	gpio7 = &gpio7;
	// mdio-gpio0 = &mdio0;
	serial1 = &uart1;
	serial2 = &uart2;
	serial3 = &uart3;
	serial4 = &uart4;
	serial5 = &uart5;
	serial6 = &uart6;
	serial8 = &uart8;
	serial9 = &uart9;
	serial12 = &uart12;
	serial13 = &uart13;
};
```

#### 1.2.配置内核： 

内核中兼容`8250`的串口默认配置为4个，因此需要修改最大配置为16个。否则内核启动时最多只能加载4路串口。

其宏定义的调用见代码：`8250_core.c`：

```c
#define UART_NR	CONFIG_SERIAL_8250_NR_UARTS
```

以及运行时允许注册的最大串口数（该数值只能小于等于宏定义：CONFIG_SERIAL_8250_NR_UARTS）：

```c
static unsigned int nr_uarts = CONFIG_SERIAL_8250_RUNTIME_UARTS;
```

具体配置界面如下：

```sh
Device Drivers  ---> 
	Character devices  ---> 
		Serial drivers  --->
```

![image-20240824131759851](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/image-20240824131759851.png)

#### 1.3. 系统启动时的打印信息：

如果一切设置无误，则内核启动时会打印11路串口的信息：

```sh
[    4.096889] Serial: 8250/16550 driver, 16 ports, IRQ sharing disabled
[    4.100896] console [ttyS0] disabled
[    4.100964] a0113000.serial: ttyS0 at MMIO 0xa0113000 (irq = 44, base_baud = 5859375) is a 16550A
[    4.569508] console [ttyS0] enabled
[    4.574172] a0114000.serial: ttyS1 at MMIO 0xa0114000 (irq = 45, base_baud = 5859375) is a 16550A
[    4.584159] a0115000.serial: ttyS2 at MMIO 0xa0115000 (irq = 46, base_baud = 5859375) is a 16550A
[    4.594124] a0116000.serial: ttyS3 at MMIO 0xa0116000 (irq = 47, base_baud = 5859375) is a 16550A
[    4.604055] a0117000.serial: ttyS4 at MMIO 0xa0117000 (irq = 48, base_baud = 5859375) is a 16550A
[    4.613989] a0383000.serial: ttyS5 at MMIO 0xa0383000 (irq = 17, base_baud = 5859375) is a 16550A
[    4.623937] a0384000.serial: ttyS6 at MMIO 0xa0384000 (irq = 18, base_baud = 5859375) is a 16550A
[    4.633850] a0386000.serial: ttyS8 at MMIO 0xa0386000 (irq = 19, base_baud = 5859375) is a 16550A
[    4.643790] a0914000.serial: ttyS9 at MMIO 0xa0914000 (irq = 49, base_baud = 5859375) is a 16550A
[    4.653721] a0917000.serial: ttyS12 at MMIO 0xa0917000 (irq = 42, base_baud = 5859375) is a 16550A
[    4.663735] a0918000.serial: ttyS13 at MMIO 0xa0918000 (irq = 43, base_baud = 5859375) is a 16550A
```

输出以上信息表明串口加载成功。

### 2、LED指示灯Linux内核心跳灯调试：

引出Linux内核心跳指示灯，是为了能够监测内核的运行是否正常，有没有死机等。

#### 2.1. 内核配置：

```sh
Device Drivers  --->
	[*] LED Support  --->
		<*>   LED Class Support
		<*>   LED Support for GPIO connected LEDs
		*** LED Triggers ***                                                                                                        [*]   LED Trigger support  --->
			<*>   LED Heartbeat Trigger
			<*>   LED activity Trigger 
			<*>   LED GPIO Trigger
			<*>   LED Default ON Trigger
```

#### 2.2. dtb配置：

```c
led {
	compatible = "gpio-leds";
	led-trigger0 {
		label = "heartbeat";
		gpios = <&port4 29 GPIO_ACTIVE_HIGH>;		
		linux,default-trigger = "heartbeat";
		default-state = "on";
	};
	led-trigger1 {
		label = "heartbeat";
		gpios = <&port4 30 GPIO_ACTIVE_LOW>;
		linux,default-trigger = "heartbeat";
		default-state = "on";
	};
};
```

> [!note]
>
> <font face="微软雅黑" color=yellow>需要根据具体的硬件设计来修改作为触发器trigger的GPIO定义。</font>

#### 2.3. Linux下使用：

如果设置成功，Linux系统启动后，可以查看到其信息：

```sh
root@CSG-FUXI:~# cat /sys/class/leds/heartbeat/trigger 
none kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock mmc0 mmc1 timer oneshot mtd nand-disk [heartbeat] backlight gpio cpu cpu0 cpu1 activity default-on transient flash torch panic netdev 
```

其中被中括号标识为：[heartbeat]的，表示这个指示灯当前触发的是`heartbeat`模式，也就是Linux内核心跳指示灯。

> [!tip]
>
> 可以手动修改触发模式来设置指示灯的具体触发模式：
>
> ```sh
> root@CSG-FUXI:~# echo activity > /sys/class/leds/heartbeat/trigger 
> root@CSG-FUXI:~# cat /sys/class/leds/heartbeat/trigger 
> none kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock mmc0 mmc1 timer oneshot mtd nand-disk heartbeat backlight gpio cpu cpu0 cpu1 [activity] default-on transient flash torch panic netdev 
> ```
>
> 可以看到，此时的中括号已经变为：[activity]。

### 3、设置QSPI0 Flash：

#### 3.1. Linux内核修改：

测试底板（万力达公司的测试底板）上焊接了一片qspi接口Flash，型号：`XT25F128B`（容量16MB），经查是深圳市芯天下的Flash，但是内核中不能正确识别，需要修改内核代码：`/drivers/mtd/spi-nor/spi-nor.c`，在定义的数组：

```c
static const struct flash_info spi_nor_ids[]
```

中添加以下识别ID头：

```c
{ "w25m512jv", INFO(0xef7119, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_QUAD_READ | SPI_NOR_DUAL_READ) },
	/*国产芯天下的qspi flash芯片*/
	{ "xt25f128b", INFO(0x0b4018, 0, 64 * 1024, 256, SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },

	/* Catalyst / On Semiconductor -- non-JEDEC */
	{ "cat25c11", CAT25_INFO(  16, 8, 16, 1, SPI_NOR_NO_ERASE | SPI_NOR_NO_FR) },
```

这样驱动就可以正确识别该Flash芯片了。

#### 3.2. dtb设备文件树的配置：

```c
&qspi0 {
	status = "okay";
	flash@0 {
		reg = <0>;
		spi-bus-width = <4>;
		spi-max-frequency = <50000000>;
		partitions {
			compatible = "fixed-partitions";
			#address-cells = <1>;
			#size-cells = <1>;

			rootfs@0 {
				label = "fault recording";
				//reg = <0x0 0xa00000>;	//10MB
				reg = <0x0 0x1000000>;	//16MB
			};
		};
	};
};
```

> [!note]
>
> <font face="微软雅黑" color=yellow>其中的label字符串设置为：“fault recording”，表示该分区16MB容量是用来存储故障录波原始数据的。</font>

#### 3.3. Linux文件系统下使用：

1）启动时如果正确加载了QSPI0这一路Flash，则内核打印信息如下：

```sh
[    4.727854] sc-qspi a0992000.qspi: dma1chan0 for rx
[    4.732833] sc-qspi a0992000.qspi: dma1chan1 for tx
[    4.738076] sc-qspi a0992000.qspi: xt25f128b (16384 Kbytes)
[    4.743721] 1 fixed-partitions partitions found on MTD device a0992000.qspi
[    4.750713] Creating 1 MTD partitions on "a0992000.qspi":
[    4.756141] 0x000000000000-0x000001000000 : "fault recording"
[    4.763197] sc-qspi a0992000.qspi: mtd registerd, cs: 0, bus width: 4
```

2）在linux文件系统下，查看：

```sh
root@CSG-FUXI:~# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 01000000 00010000 "fault recording"
mtd1: 00100000 00010000 "fsbl"
mtd2: 00040000 00010000 "nvm"
mtd3: 00020000 00010000 "baremetal803.0"
mtd4: 00020000 00010000 "baremetal803.1"
mtd5: 00080000 00010000 "bootloader env"
mtd6: 00100000 00010000 "bootloader"
mtd7: 00100000 00010000 "baremetal1"
mtd8: 00080000 00010000 "baremetal2"
mtd9: 00080000 00010000 "device tree"
mtd10: 00b00000 00010000 "kernel"
```

其中的`mtd0`即为QSPI0这一路的Flash分区。**注意QSPI0 Flash只有1个分区，即：`mtd0`。**

3）格式化分区并mount：

```sh
root@CSG-FUXI:~# mkfs.ext4 /dev/mtdblock0
root@CSG-FUXI:~# mount /dev/mtdblock0 /tmp
```

