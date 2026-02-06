# 内核态与用户态通信之eventfd使用

**首先需要确定eventfd已经被编译进内核**，**其次还要确定所使用的[交叉编译](https://so.csdn.net/so/search?q=交叉编译&spm=1001.2101.3001.7020)器支持eventfd。**

函数原型：

```c
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);
12
```

说明：initval的范围是0~0xfffffffffffffffe；flags的值可以是如下枚举值：

```c
enum
{
    EFD_SEMAPHORE = 00000001,//since Linux 2.6.30
#define EFD_SEMAPHORE EFD_SEMAPHORE
    EFD_CLOEXEC = 02000000,//since Linux 2.6.27
#define EFD_CLOEXEC EFD_CLOEXEC
    EFD_NONBLOCK = 00004000//since Linux 2.6.27
#define EFD_NONBLOCK EFD_NONBLOCK
};
123456789
```

**如果设置了EFD_SEMAPHORE，则不会出现粘包的情况，即write多少次，就需要read多少次；**

**如果设置了0（内核2.6.27版本之前必须设置为0），则会出现粘包的可能，write多次，可能read到的是若干次的值的和；**

另外两种一般不需要，所以并未进行测试。

1. 用户态

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
   1234567891011121314151617181920212223242526272829303132333435363738394041424344
   ```

2. 内核态

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
   123456789101112131415161718192021222324252627282930313233343536373839404142
   ```

3. 如何使用以上示例

> Compile the userspace program (efd_us.out) and the kernel module (efd_lkm.ko)
>  Run the userspace program (./efd_us.out) and note the pid and efd  values that it print. (for eg. “pid=2803 efd=3”. The userspace program  will wait endlessly on select()
>  Open a new terminal window and insert the kernel module passing the pid and efd as params: sudo insmod efd_lkm.ko pid=2803 efd=3
>  Switch back to the userspace program window and you will see that the userspace program hasbroken out of select and exited.