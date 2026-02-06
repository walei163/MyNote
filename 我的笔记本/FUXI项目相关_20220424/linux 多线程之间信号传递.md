# [linux 多线程之间信号传递](https://www.cnblogs.com/chay/p/10587511.html)

## 1、函数

- ## sigwait

sigwait的含义就如同它的字面意思:等待某个信号的到来。如果调用该函数的线程没有等到它想等待的信号那么该线程就休眠。要达到等到一个信号，我们得做下面的事：
首先，定义一个信号集：

```c
#include <signal.h>
sigset_t set;
```

其次，向信号集中加入我们想等待的信号：

```c
#include <signal.h>
int sigemptyset(sigset_t *set);//清空信号集
int sigaddset（sigset_t *set,int signo）;//将某个信号加入到信号集中
int sigdelset（sigset_t *set,int signo）;//删除信号集中的某个信号
int sigfillset（sigset_t *set）;//包含所有已定义的信号
```

最后，将该信号集中的信号加入到线程信号屏蔽字(线程信号等待队列)中：

```c
#include <signal.h>
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
```

**how:**

```sh
SIG_BLOCK:把参数set中的信号添加到线程的信号屏蔽字中
SIG_SETMASK:把线程的信号屏蔽字设置为参数set中的信号
SIG_UNBLOCK:从线程信号屏蔽字中删除参数set中的信号
set:用户设置的信号屏蔽字
oldset:返回原先的信号屏蔽字
```

经过这样的设置我们就可以在线程该等待的地方调用sigwait休眠该线程了。

- ## pthread_kill

```c
#include <signal.h>
int pthread_kill(pthread_t thread, int sig);
```

thread:给哪个线程发送信号 

sig:发送的信号值

- ## sigaction

sigaction函数可以读取和修改与指定信号相关联的处理动作，它的声明如下：

```c
#include <signal.h>
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

**参数：**
 --第一个参数是信号的值，可以为除了SIGKILL及SIGSTOP外的任何一个特定有效的信号（因为这两个信号定义了自己的处理函数，将导致信号安装错误）。

　　SIG_DFL,SIG_IGN 分别表示无返回值的函数指针，指针值分别是0和1，这两个指针值逻辑上讲是实际程序中不可能出现的函数地址值。
 　SIG_DFL：默认信号处理程序。
 　SIG_IGN：忽略信号的处理程序。
 --第二个参数是指向sigaction的一个实例的指针，在sigaction的实例中，指定了对特定信号的处理，可以为NULL，进程会以缺省方式对信号处理。
 --第三个参数oldact指向的对象用来保存原来对相应信号的处理，可以为NULL。
 **返回值：**
 函数成功返回0，失败返回-1。
 sigaction函数检查或修改与指定信号相关联的处理动作，该函数取代了signal函数。因为signal函数在信号未决时接收信号可能出现问题，所以使用sigaction更安全。
 **sigaction结构体：**

```c
struct sigaction
{
    void (*sa_handler)(int);//信号处理程序 不接受额外数据
    void (*sa_sigaction)(int, siginfo_t *, void *);//信号处理程序，能接受额外数据，可以和sigqueue配合使用
    sigset_t sa_mask;
    int sa_flags;//影响信号的行为SA_SIGINFO表示能接受数据
    void (*sa_restorer)(void);//废弃
};
```

### sa_handler & sa_sigaction

信号处理函数的指针，二者用其一：如果sa_flags中存在SA_SIGINFO标志，那么sa_sigaction将作为signum信号的处理函数，否则用sa_handler。

### sa_mask

指定一个系统在处理该信号时要屏蔽（阻塞）的信号集合，即在调用该信号捕捉函数之前，这一信号集要加进进程的信号屏蔽字中。
 仅当从信号捕捉函数返回时再将进程的信号屏蔽字复位为原先值。
 另外，除了SA_NODEFER标志被指定外，触发信号处理函数执行的那个信号也会被阻塞。

### sa_flag

指定一系列用于修改信号处理过程行为的标志，由下面的0个或多个标志通过or运算组合而成：

```sh
SA_SIGINFO 指定信号处理函数需要三个参数，所以应使用sa_sigaction替代sa_handler。
SA_NODEFER 在信号处理函数处置信号的时段中，核心程序不会把这个间隙中产生的信号阻塞。
SA_INTERRUPT 由此信号中断的系统调用不会自动重启
SA_RESTART 核心会自动重启信号中断的系统调用，否则返回EINTR错误值。（重启被中断的系统调用）
SA_RESETHAND 信号处理函数接收到信号后，会先将对信号处理的方式设为预设方式，而且当函数处理该信号时，后来发生的信号将不会被阻塞。
SA_ONSTACK 如果利用sigaltstack()建立信号专用堆栈，则此标志会把所有信号送往该堆栈。
SA_NOCLDSTOP 假如signum的值是SIGCHLD，则在子进程停止或恢复执行时不会传信号给调用本系统调用的进程。
SA_NOCLDWAIT 当调用此系统调用的进程之子进程终止时，系统不会建立zombie进程。 
```

## 2、程序示例

```c
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

void *threadfunc1(void *pvoid)
{ 
    int signum;
    sigset_t sig;

    sigemptyset(&sig);
    sigaddset(&sig,SIGUSR1);
    pthread_sigmask(SIG_BLOCK,&sig,NULL);//设置该线程的信号屏蔽字为SIGUSR1
    while(1)
    {
        sigwait(&sig,&signum);//睡眠等待SIGUSR1信号的到来 
        printf("threadfunc1 waiting is over!\n");
    }
}
void *threadfunc2(void *pvoid)
{ 
    int signum;
    sigset_t sig;

    sigemptyset(&sig);
    sigaddset(&sig,SIGUSR1);
    pthread_sigmask(SIG_BLOCK,&sig,NULL);//设置该线程的信号屏蔽字为SIGUSR1
    while(1)
    {
        sigwait(&sig,&signum);//睡眠等待SIGUSR1信号的到来
        printf("threadfunc2 waiting is over!\n");
    }
}
void main()
{
    pthread_t thread1,thread2;
    pthread_create(&thread1,NULL,threadfunc1,(void *)NULL);
    pthread_create(&thread2,NULL,threadfunc2,(void *)NULL);

    pthread_detach(thread1);
    pthread_detach(thread2);

    struct sigaction act;
    act.sa_handler=SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGUSR1,&act,0);//设置信号SIGUSR1的处理方式忽略 
    while(1)
    {
        int i;
        printf("please input select a number in (1,2)!\n");
        scanf("%d",&i);
        if(i==1)
        pthread_kill(thread1,SIGUSR1); 
        if(i==2)
        pthread_kill(thread2,SIGUSR1);
    }
}
```

**编译：**

```sh
gcc -o pthread pthread.c -lpthread
```

## 3、自定义信号

用户自定义的信号不是只有SIGUSR1,SIGUSR2，linux下预留了**SIGRTMIN（32）到SIGRTMAX（64）**可供用户自定义，当用户需要自定义信号时可以采用下面的方法：

```c
#define SIG_RECVDATA    __SIGRTMIN+offsetnum
```

offset是一个偏移量，实时信号前三个最好不要用，linux thread在使用它们。
 例如使用：

```c
#define SIG_RECVDATA    __SIGRTMIN+10
```