# Linux内核层向应用层发送驱动

**Linux支持的信号有64种，不可靠信号31种（1-31),可靠信号(34-64)**，常规的发送信号为异步通知，从内核层发送SIGIO信号，这里介绍的是发送可靠信号的方法

1. ```sh
   kill -l
   
     1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
     2) SIGABRT      7) SIGBUS       8) SIGFPE       9) SIGKILL     10) SIGUSR1
    3) SIGSEGV     12) SIGUSR2     13) SIGPIPE     14) SIGALRM     15) SIGTERM
    4) SIGSTKFLT   17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
    5) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
    6) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO       30) SIGPWR
    7) SIGSYS      34) SIGRTMIN    35) SIGRTMIN+1  36) SIGRTMIN+2  37) SIGRTMIN+3
    8) SIGRTMIN+4  39) SIGRTMIN+5  40) SIGRTMIN+6  41) SIGRTMIN+7  42) SIGRTMIN+8
    9) SIGRTMIN+9  44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12 47) SIGRTMIN+13
    10) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14 51) SIGRTMAX-13 52) SIGRTMAX-12
    11) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9  56) SIGRTMAX-8  57) SIGRTMAX-7
    12) SIGRTMAX-6  59) SIGRTMAX-5  60) SIGRTMAX-4  61) SIGRTMAX-3  62) SIGRTMAX-2
    13) SIGRTMAX-1  64) SIGRTMAX
   ```

   



## 内核驱动示例代码（发送3种不同可靠的信号，2种不可靠的SIGUSR信号）：

```c
/*************************************************************************
    > File Name: test_sig.c
    > Author: Ltp
    > Mail: ltpforever@163.com 
    > Created Time: 2024年09月14日 星期六 09时15分19秒
 ************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/timer.h>

#include <uapi/asm-generic/siginfo.h>
#include <linux/pid.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/pid_namespace.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/sched/task.h>

#define DEV_NAME "test_sig"

//自定义信号类型
typedef enum { SIG_TIMER1 = 34, SIG_TIMER2, SIG_TIMER3, SIG_TIMER4 } SIG_TYPE;

static dev_t devid;
static struct cdev sig_cdev;
static struct class *sig_cls;

static struct timer_list my_timer1;
static struct timer_list my_timer2;
static struct timer_list my_timer3;

static int delay_timer1 = 1000;
static int delay_timer2 = 500;
static int delay_timer3 = 3000;
static int start_timer_flag = 0;

// 用来保存向谁发送信号，应用程序通过 ioctl 把自己的进程 PID 设置进来。
static int g_pid = 0;

// 用来发送信号给应用程序
static void send_signal(int sig_no)
{
	
	int ret;
	struct kernel_siginfo info;
	struct task_struct *my_task = NULL;
	if (0 == g_pid) {
		// 说明应用程序没有设置自己的 PID
		printk("pid[%d] is not valid! \n", g_pid);
		return;
	}

​	printk("send signal %d to pid %d \n", sig_no, g_pid);

​	// 构造信号结构体
​	memset(&info, 0, sizeof(struct kernel_siginfo));
​	info.si_signo = sig_no;
​	info.si_code = SI_KERNEL; 

​	// 获取自己的任务信息，使用的是 RCU 锁
​    rcu_read_lock();
​    my_task = pid_task(find_vpid(g_pid), PIDTYPE_PID);
​    rcu_read_unlock();

​	if (my_task == NULL) {
​		printk("get pid_task failed! \n");
​		return;
​	}

​	// 发送信号
​	ret = send_sig_info(sig_no, &info, my_task);
​	if (ret < 0) {
​		printk("send signal failed! \n");
​	}
​	
}

// void timer_callback(unsigned long data)
void timer_callback(struct timer_list *t)
{
	if (!start_timer_flag || g_pid == 0)
		return;

​	if (t == &my_timer1) {
​		send_signal(SIG_TIMER1);
​		mod_timer(&my_timer1, jiffies + msecs_to_jiffies(delay_timer1));
​	}

​	if (t == &my_timer2) {
​		send_signal(SIG_TIMER2);
​		mod_timer(&my_timer2, jiffies + msecs_to_jiffies(delay_timer2));
​	}

​	if (t == &my_timer3) {
​		send_signal(SIG_TIMER3);
​		mod_timer(&my_timer3, jiffies + msecs_to_jiffies(delay_timer3));
​	}
}

static int my_init_timer(void)
{
	printk("%s Init Timer\n", DEV_NAME);

​	/*
​	my_timer1.function = timer_callback;
​	my_timer1.data = SIG_TIMER1; // 传递给回调函数的数据
​	my_timer1.expires = jiffies + msecs_to_jiffies(delay_timer1);
​	init_timer(&my_timer1); // 初始化定时器

​	my_timer2.function = timer_callback;
​	my_timer2.data = SIG_TIMER2; // 传递给回调函数的数据
​	my_timer2.expires = jiffies + msecs_to_jiffies(delay_timer2);
​	init_timer(&my_timer2); // 初始化定时器

​	my_timer3.function = timer_callback;
​	my_timer3.data = SIG_TIMER3; // 传递给回调函数的数据
​	my_timer3.expires = jiffies + msecs_to_jiffies(delay_timer3);
​	init_timer(&my_timer3); // 初始化定时器
​	// 添加定时器
​	add_timer(&my_timer1);
​	add_timer(&my_timer2);
​	add_timer(&my_timer3);
​    */

​	timer_setup(&my_timer1, timer_callback, 0);
​	timer_setup(&my_timer2, timer_callback, 0);
​	timer_setup(&my_timer3, timer_callback, 0);

​	return 0;
}

static int my_destory_timer(void)
{
	if (timer_pending(&my_timer1))
		del_timer(&my_timer1);

​	if (timer_pending(&my_timer2))
​		del_timer(&my_timer2);

​	if (timer_pending(&my_timer3))
​		del_timer(&my_timer3);

​	return 0;
}

static int sig_open(struct inode *inode, struct file *file)
{
	printk("%s : Start Timer!\n", DEV_NAME);
	return 0;
}

static int sig_close(struct inode *inode, struct file *file)
{
	printk("%s : Stop Timer!\n", DEV_NAME);
	start_timer_flag = 0;
	g_pid = 0;
	return 0;
}

static long sig_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *pArg;
	printk("sig_ioctl is called. cmd = %d , arg=%lu \n", cmd, arg);

​	if (100 == cmd) {
​		// 说明应用程序设置进程的 PID
​		pArg = (void *)arg;
​		if (!pArg) {
​			printk("access failed! \n");
​			return -EACCES;
​		}

​		// 把用户空间的数据复制到内核空间
​		if (copy_from_user(&g_pid, pArg, sizeof(int))) {
​			printk("copy_from_user failed! \n");
​			return -EFAULT;
​		}

​		printk("save g_pid success: %d \n", g_pid);
​		if (g_pid > 0) {
​			// 发送信号
​			send_signal(SIGUSR1);
​			send_signal(SIGUSR2);
​		}

​	} else if (101 == cmd) {
​		start_timer_flag = 1;
​		mod_timer(&my_timer1, jiffies + msecs_to_jiffies(delay_timer1));
​		mod_timer(&my_timer2, jiffies + msecs_to_jiffies(delay_timer2));
​		mod_timer(&my_timer3, jiffies + msecs_to_jiffies(delay_timer3));
​	}

​	return 0;
}

static struct file_operations sig_fops = {
	.owner = THIS_MODULE,
	.open = sig_open,
	.release = sig_close,
	.unlocked_ioctl = sig_ioctl,
};

static int __init sig_test_init(void)
{
	//动态分配字符设备: (major,0)
	if (alloc_chrdev_region(&devid, 0, 1, DEV_NAME) !=

   0. { // ls /proc/devices看到的名字
      	printk("Register %s Error!\n", DEV_NAME);
      	return -1;
      }

      cdev_init(&sig_cdev, &sig_fops);
      cdev_add(&sig_cdev, devid, 1);

      /* 创建类,它会在sys目录下创建/sys/class/button这个类  */
      sig_cls = class_create(THIS_MODULE, DEV_NAME);
      if (IS_ERR(sig_cls)) {
      	printk("Can't create %s class !\n", DEV_NAME);
      	cdev_del(&sig_cdev);
      	unregister_chrdev_region(devid, 1);
      	return -1;
      }

      /* 在/sys/class/button下创建buttons设备，然后mdev通过这个自动创建/dev/这个设备节点 */
      device_create(sig_cls, NULL, devid, NULL, DEV_NAME);

      my_init_timer();

      printk("%s init Ok \n!", DEV_NAME);
      return 0;
      }

static void __exit sig_test_exit(void)
{
	printk("%s Exit\n", DEV_NAME);

​	my_destory_timer();
​	device_destroy(sig_cls, devid);
​	class_destroy(sig_cls);
​	cdev_del(&sig_cdev);
​	unregister_chrdev_region(devid, 1);
​	return;
}

MODULE_LICENSE("GPL");
module_init(sig_test_init);
module_exit(sig_test_exit);


```

```sh
Makefile: 

obj-$(CONFIG_TEST_SIG) += test_sig.o 

Kconfig: 

config TEST_SIG        

	tristate "Test Kernel Signal To Userspace "
```



## 应用层示例代码(不同的信号可走不同的函数处理)

```c
#include <stdio.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DEV_NAME "/dev/test_sig"

//自定义信号类型
typedef enum { SIG_TIMER1 = 34, SIG_TIMER2, SIG_TIMER3, SIG_TIMER4 } SIG_TYPE;

// 信号处理函数
static void signal_handler(int signum, siginfo_t *info, void *context)
{
        // 打印接收到的信号值
        printf("--------------------------> signal_handler: signum = %d \n", signum);
}

static void signal_handler1(int signum, siginfo_t *info, void *context)
{
        // 打印接收到的信号值
        printf("--------------------------> signal_handler1: signum = %d \n", signum);
}

static void signal_handler2(int signum, siginfo_t *info, void *context)
{
        // 打印接收到的信号值
        printf("--------------------------> signal_handler2: signum = %d \n", signum);
}

static void signal_handler3(int signum, siginfo_t *info, void *context)
{
        // 打印接收到的信号值
        printf("--------------------------> signal_handler3: signum = %d \n", signum);
}

static void sig_handle1(int sig){
        printf("%s -----> recv sig:%d\n",sig);
}

static void sig_handle2(int sig){
        printf("%s -----> recv sig:%d\n",sig);
}

static void sig_handle3(int sig){
        printf("%s -----> recv sig:%d\n",sig);
}

static void sig_handle4(int sig){
        printf("%s -----> recv sig:%d\n",sig);
}

int main(void)
{
        int count = 0;
        // 注册信号处理函数
#if 1
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = &signal_handler;
        sa.sa_flags = SA_SIGINFO;
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);


​        struct sigaction sa1;
​        sigemptyset(&sa1.sa_mask);
​        sa1.sa_sigaction = &signal_handler1;
​        sa1.sa_flags = SA_SIGINFO;
​        sigaction(SIG_TIMER1, &sa1, NULL);


​        struct sigaction sa2;
​        sigemptyset(&sa2.sa_mask);
​        sa2.sa_sigaction = &signal_handler2;
​        sa2.sa_flags = SA_SIGINFO;
​        sigaction(SIG_TIMER2, &sa2, NULL);

​        struct sigaction sa3;
​        sigemptyset(&sa3.sa_mask);
​        sa3.sa_sigaction = &signal_handler3;
​        sa3.sa_flags = SA_SIGINFO;
​        sigaction(SIG_TIMER3, &sa3, NULL);
#endif

#if 0
        signal(SIGUSR1,sig_handle1);
        signal(SIGUSR2,sig_handle1);
        signal(SIG_TIMER1,sig_handle2);
        signal(SIG_TIMER2,sig_handle3);
        signal(SIG_TIMER3,sig_handle4);
#endif
        int fd=0;
        fd=open(DEV_NAME,O_RDWR);
        if(fd<0){
                perror("Open Fail!");
                return -1;
        }

​        int pid=0;
​        int on=1;
​        pid=getpid();
​        ioctl(fd,100,&pid);
​        ioctl(fd,101,&on);


​        // 一直循环打印信息，等待接收发信号
​        while (1)
​        {
​                printf("app_handle_signal is running...count = %d \n", ++count);
​                sleep(10);
​        }

​        return 0;
}
```

