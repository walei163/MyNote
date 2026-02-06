# 内核和用户空间的消息传递

## 一、异步通知

背景： 当内核空间的驱动收发数据需要通知用户空间时，有以下几种方法：

1.异步通知机制：

**a. 在驱动层注册fasync函数**

```c
//fops
static struct file_operations global_logic_fops={
    .owner   = THIS_MODULE,
    .unlocked_ioctl	= logic_api_ioctl,
    .open    = logic_open_func,
    .release = logic_release,
    .write   = logic_write,
    .read    = logic_read,
    .fasync  = logic_fasync_func,
    
};
```

**b.驱动实现fasync函数**

```c
static int logic_fasync_func(int fd, struct file *flip, int on)
{
	int err; 
 
    /* 注意APP调用前，需要先通过ioctl接口，设置 g_logic_device_index */
    err  = fasync_helper(fd,flip,on,&pcm_fasync[g_logic_device_index]);
 
    printk(KERN_ALERT "%s :%s index [%d]!\n", __FILE__, __func__, g_logic_device_index);
	return err;
}
```

**c.在需要发送消息的地方如下调用**

```c
static  int pcm_Async(int channel)
{
    /* 通知有数据 */
    if(channel < MAX_LOGIC_DEVCIE_NUM)
    {
	    kill_fasync(&(pcm_fasync[channel]), SIGIO, POLLIN);
    }
	return 0;
}
```

**d.在用户态绑定信号服务函数**

```c
    /* 异步信号处理函数 */
    signal(SIGIO, logic_PCMHandler);
 
    /* 设置驱动绑定对应的异步通知发生器，一个设备文件可以给多个进程发送信号，需要知道目前是绑定哪个 */
    encoder_index = m31_GetEncodeIndex();
    logic_SetDeviceIndex(encoder_index);
 
    /* 将驱动发出的异步信号绑定到本进程 */
    fcntl(g_LogicDevice, F_SETOWN, getpid());
    flags = fcntl(g_LogicDevice, F_GETFL);
    flags |= FASYNC;
    fcntl(g_LogicDevice, F_SETFL, flags);
```


————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/tea1896/article/details/51553385

## 二、事件通知eventfd

事件通知机制不仅可用户用户态两个线程之间同步，还可以用于内核空间和用户空间同步。


方法如下：

### 1.在用户层

**a. 创建一个文件描述符**

```c
int event_fd = -1;
event_fd = eventfd ( 0, EFD_NONBLOCK | EFD_SEMAPHORE );
```

**b.将文件描述符通过ioctl传递给内核** 

```c
enable_message_notification_info.event_fd       = event_fd;
ioctl ( fmb_fd, FMB_API_ENABLE_MESSAGE_NOTIFICATION, &enable_message_notification_info );
```

**c.将事件通知的文件描述符，加入文件描述集，然后用select监控**

```c
fd_set read_fds;

while(1)

{

//.将这个文件描述符加入set,然后select监控队列

FD_ZERO ( &read_fds );
FD_SET ( event_fd , &read_fds );

result = select ( max_fd + 1, &read_fds, NULL, NULL, NULL );

。。。

// 如果有事件触发

if ( FD_ISSET ( message_thread_param_p->fd_receive_message_notification, &read_fds ) )

 {
  uint32_t message_id;
  uint32_t message_param[FMB_MESSAGE_PARAM_SIZE];

  //清空
  result = eventfd_read ( message_thread_param_p->fd_receive_message_notification, &eventfd_value );
  ASSERT_APP ( result == 0, "ERR: eventfd_read" );

 //事件处理     

 ...  ...  

}

}



```

# FD_ISSET

 FD_ZERO，FD_ISSET这些都是套节字结合操作宏 
 看看MSDN上的select函数, 
 这是在select  io  模型中的核心,用来管理套节字IO的,避免出现无辜锁定. 
 int  select(   int  nfds,fd_set  FAR  *readfds,   fd_set  FAR  *writefds,               
   fd_set  FAR  *exceptfds,         
   const  struct  timeval  FAR  *timeout   
 ); 
 第一个参数不管,是兼容目的,最后的是超时标准,select是阻塞操作 
 当然要设置超时事件. 
 接着的三个类型为fd_set的参数分别是用于检查套节字的可读性,可写性,和列外数据性质. 

 我举个例子 
 比如recv(),  在没有数据到来调用它的时候,你的线程将被阻塞 
 如果数据一直不来,你的线程就要阻塞很久.这样显然不好. 
 所以采用select来查看套节字是否可读(也就是是否有数据读了) 
 步骤如下 
 socket  s; 
 ..... 
 fd_set  set; 
 while(1) 
 {   
   FD_ZERO(&set);//将你的套节字集合清空 
   FD_SET(s,  &set);//加入你感兴趣的套节字到集合,这里是一个读数据的套节字s 
   select(0,&set,NULL,NULL,NULL);//检查套节字是否可读, 
                            //很多情况下就是是否有数据(注意,只是说很多情况) 
                            //这里select是否出错没有写 
   if(FD_ISSET(s,  &set)  //检查s是否在这个集合里面, 
   {                      //select将更新这个集合,把其中不可读的套节字去掉 
                         //只保留符合条件的套节字在这个集合里面 
                 
       recv(s,...); 

   } 
   //do  something  here 
 }

### 2.在内核空间：

**a. 通过ioctl得到文件描述符，转换为eventfd_ctx**

```c
struct eventfd_ctx* eventfd_ctx_p;
eventfd_ctx_p = eventfd_ctx_fdget ( event_fd );
```

**b.在需要发送消息的地方调用以下接口：**

```c
eventfd_signal ( queue_p->notification_info.eventfd_ctx_p, 1 );
```

**c.回收这个消息的资源**

```c
eventfd_ctx_put ( queue_p->notification_info.eventfd_ctx_p );
```

————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/tea1896/article/details/51553438