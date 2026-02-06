# kernel通过eventfd向user空间发送消息事件通知

写在前面

        接下来我们来尝试写一个测试程序，该测试程序首先在用户空间创建eventfd对象，然后再通过kernel模块程序更新eventfd的计数器以实现向用户控件程序发送事件消息通知。
    
        *本篇基于Ubunt 18.04调试和验证*
## 一、用户空间程序-efd-us

### 1.1 源码

efd-us.c     

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>     //Definition of uint64_t
#include <sys/eventfd.h>
 
int efd; //Eventfd file descriptor
uint64_t eftd_ctr;
 
int retval;     //for select()
fd_set rfds;        //for select()
 
int s;
 
int main() {
    //Create eventfd
    efd = eventfd(0,0);
    if (efd == -1){
        printf("\nUnable to create eventfd! Exiting...\n");
        exit(EXIT_FAILURE);
    }
 
    printf("\nefd=%d pid=%d",efd,getpid());
 
    //Watch efd
    FD_ZERO(&rfds);
    FD_SET(efd, &rfds);
 
    printf("\nNow waiting on select()...");
    fflush(stdout);
 
    retval = select(efd+1, &rfds, NULL, NULL, NULL);
 
    if (retval == -1){
        printf("\nselect() error. Exiting...");
        exit(EXIT_FAILURE);
    } else if (retval > 0) {
        printf("\nselect() says data is available now. Exiting...");
        printf("\nreturned from select(), now executing read()...");
        s = read(efd, &eftd_ctr, sizeof(uint64_t));
        if (s != sizeof(uint64_t)){
            printf("\neventfd read error. Exiting...");
        } else {
            printf("\nReturned from read(), value read = %lld",eftd_ctr);
        }
    } else if (retval == 0) {
        printf("\nselect() says that no data was available");
    }
 
    printf("\nClosing eventfd. Exiting...");
    close(efd);
    printf("\n");
    exit(EXIT_SUCCESS);
}
```

### 1.2 编译方法

```sh
gcc efd-us.c -o efd-us
```

### 1.3 代码解读

```sh
//创建eventfd文件描述符

efd = eventfd(0,0);

//监听efd

retval = select(efd+1, &rfds, NULL, NULL, NULL);

//读取efd计数器内容

s = read(efd, &eftd_ctr, sizeof(uint64_t));
```

## 二、内核模块代码 - efd-lkm

### 2.1 efd-lkm.c源码

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/rcupdate.h>
#include <linux/eventfd.h>
 
//Received from userspace. Process ID and eventfd's File descriptor are enough to uniquely identify an eventfd object.
int pid;
int efd;
 
//Resolved references...
struct task_struct * userspace_task = NULL; //...to userspace program's task struct
struct file * efd_file = NULL;          //...to eventfd's file struct
struct eventfd_ctx * efd_ctx = NULL;        //...and finally to eventfd context
 
//Increment Counter by 1
static uint64_t plus_one = 1;
 
int init_module(void) {
    printk(KERN_ALERT "~~~Received from userspace: pid=%d efd=%d\n",pid,efd);
 
    userspace_task = pid_task(find_vpid(pid), PIDTYPE_PID);
    printk(KERN_ALERT "~~~Resolved pointer to the userspace program's task struct: %p\n",userspace_task);
 
    printk(KERN_ALERT "~~~Resolved pointer to the userspace program's files struct: %p\n",userspace_task->files);
 
    rcu_read_lock();
    efd_file = fcheck_files(userspace_task->files, efd);
    rcu_read_unlock();
    printk(KERN_ALERT "~~~Resolved pointer to the userspace program's eventfd's file struct: %p\n",efd_file);
    efd_ctx = eventfd_ctx_fileget(efd_file);
    if (!efd_ctx) {
        printk(KERN_ALERT "~~~eventfd_ctx_fileget() Jhol, Bye.\n");
        return -1;
    }
    printk(KERN_ALERT "~~~Resolved pointer to the userspace program's eventfd's context: %p\n",efd_ctx);
 
    eventfd_signal(efd_ctx, plus_one);
 
    printk(KERN_ALERT "~~~Incremented userspace program's eventfd's counter by 1\n");
 
    eventfd_ctx_put(efd_ctx);
 
    return 0;
}

void cleanup_module(void) {
    printk(KERN_ALERT "~~~Module Exiting...\n");
}  
 
MODULE_LICENSE("GPL");
module_param(pid, int, 0);
module_param(efd, int, 0);
```

###  2.2 代码解读

```sh
//init_module是默认的模块的入口，如果你想指定其他的函数作为模块的入口就需要module_init函数来指定。init_module()是真正的入口，module_init是宏，如果在模块中使用，最终还是要转换到init_module()上。如果不是在模块中使用，module_init可以说没有什么作用。

init_module(void)

//pid_task( ) 函数获取任务的任务描述符信息

userspace_task = pid_task(find_vpid(pid), PIDTYPE_PID);

//根据files_struct结构中的filelist和给定的fd获取file结构体

efd_file = fcheck_files(userspace_task->files, efd);

//（Acquires a reference to the internal eventfd context.）获取对内部eventfd上下文的引用，并返回该引用的指针。

efd_ctx = eventfd_ctx_fileget(efd_file);

//eventfd计数器增1（不一定增1，这个plus_one可以为任何数字）

eventfd_signal(efd_ctx, plus_one);

//（Releases a reference to the internal eventfd context）释放对内部eventfd上下文的引用

eventfd_ctx_put(efd_ctx);
```
————————————————

##   四、编译内核模块代码-**efd-lkm**

### 4.1 编写Makefile

Makefile

```sh
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
 
# efd-lkm.o即我们定义的需要编译“内核模块”， obj-y 编译进内核，obj-n 不编译。
obj-m := efd-lkm.o
 
CROSS_COMPILE =
 
CC            = gcc
 # Assume the source tree is where the running kernel was built
    # You should set KERNELDIR in the environment if it's elsewhere
    # 声明kernel源码路径
    KERNELDIR ?= /usr/src/linux-headers-$(shell uname -r)
    # The current directory is passed to sub-makes as argument
    # 生命当前Makefiles所在路径
    PWD := $(shell pwd)
 
all:     modules
modules:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
        rm -rf *.o *~ core .depend *.symvers .*.cmd *.ko *.mod.c .tmp_versions $(TARGET) 
```

    注意！Makefile中的“obj-m := efd-lkm.o"中的efd-lmk.o需要与efd-lmk.c一致（后缀面不同）。
    ————————————————

## 五、efd-us 与efd-lkm功能联调

### 5.1 启动执行efd-us

新建一个终端窗口执行用户空间应用程序efd-us，如下：

```sh
./efd-us
```

![d5fbc17dab64a446c44a2dee8025fa61](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/d5fbc17dab64a446c44a2dee8025fa61.png)

### 5.2 安装efd-lkm.ko

```sh
sudo insmod efd-lkm.ko pid-=2467 efd=3

命令解释：

语法：insmod [-fkmpsvxX][-o <模块名称>][模块文件][符号名称 = 符号值]

efd-lkm.ko//模块文件

pid=12467//pid为符号名称，12467位符号值

efd=3//同上
```

再新建一个终端窗口安装前面完成编译的内核模块efd-lkm.ko,如下：

![a96b477dc47ddbdebda517a19cf7124e](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/a96b477dc47ddbdebda517a19cf7124e.png)

### 5.3 测试结果

![a60157baa13fd0d50d1e2e478487a02f](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/a60157baa13fd0d50d1e2e478487a02f.png)

————————————————

原文链接：https://blog.csdn.net/huangyabin001/article/details/123868760