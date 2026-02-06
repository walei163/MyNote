# Linux和TSN网络

![img](https://img-home.csdnimg.cn/images/20240711042549.png) 本文详细描述了如何在标准Linux系统中配置时间敏感网络（TSN)，包括使用LinuxPTP进行时间同步、VLAN设置、Qdiscs和数据包调度算法如MQPRIO、ETF和ETS。作者还提供了实际的配置步骤和性能测试方法。

时间敏感网络是传统以太网的扩展，实现实时能力和确定性通信。终端通常是基于Linux OS 的，它的实时能力是通过PREEMPT-RT 补丁实现的。而设备的实时性网络是通过时间敏感性网络技术实现。到目前为止，大多数Linux OS 的TSN 都是通过特殊的解决方案实现的，那么，是否能够在主流的Linux 中实现TSN 网络呢？时间同步和流量调度都已经能够实现了。但是并没有实现完整的TSN 标准。

      本文介绍如何在标准的Linux 系统中配置TSN ，主要是linuxptp，tc 等工具。
## TSN 解决什么问题

TSN 主要解决两个问题：

- 全网络设备实现时钟同步
- 实时数据和非实时数据在一个网络中同传，并且保证实时数据具有优先级。

## TSN 解决方案

###  TSN 硬件支持

TSN 的核心思想是在原有以太网的基础上，增加了硬件消息队列，每个队列安装优先级排队，由硬件消息调度器在全局时钟的控制下控制队列消息的发送开关（Gate）。这样就能够保证实时数据能够以最快的时间发送出去，并且消息的发送是有序的。不会造成冲突。其结构示意图如下：

![img](https://i-blog.csdnimg.cn/blog_migrate/84865a1970a7ba58606dbec5a6eff6b8.png)

 其中每个时间片都有一个用户指定的持续时间。在每个时间片期间，用户通过指定与每个队列对应的哪些门（可能单个或多个门）“打开”或“关闭”来配置可以传输的流量优先级/队列

### tsn 消息的软件调度算法

​    软件调度的方法是：

  将网络发送分成若干的时间片（time slice）

![img](https://i-blog.csdnimg.cn/blog_migrate/6053294afd6d067caf124f056a8d51d2.png)

 应用程序发送的所有消息包都带有一个优先级（PRI），根据优先级映射到一组时间片（time slice）。例如：

| PRI  |  0   |  1   |  2   | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   | 12   | 13   | 14   | 15   |
| ---- | :--: | :--: | :--: | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| TC   |  0   |  0   |  1   | 2    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    |

## VLAN 设置

   消息包的优先等级是如何设定的呢，它是通过VLAN 的PCP 转化而来的。

           TSN基于IEEE 802.1Q[7]的虚拟局域网（virtual local area network，VLAN）和优先级标准构建的。

   TSN 将VLAN 的优先等级信息PCP自动转化为数据包的优先等级（PRI）。

VLAN 帧结构

![img](https://i-blog.csdnimg.cn/blog_migrate/137d28422c35e4ced372e62875c7537c.png)

![img](https://i-blog.csdnimg.cn/blog_migrate/f982745ade3a54c1371cd092d34360bf.png)

因此，TSN 是构建在VLAN 之上的。

#### **时间同步协议PTP**

显然，要实现数据的周期性时隙传输，必备的条件是网络中的所有设备和交换机需要使用全网统一时钟，这样才能够避免数据帧的发送冲突。实现时分复用（TDM time division multplexer）。在TSN中，使用IEEE1588/PTP 实现全网精确时钟同步。

Linux 内核具有支持PTP 硬件时钟（PHC）的子系统·，可以使用POSIX 时钟显示该时钟。通过·POSIX 接口读取，设置和控制。PTP 协议不是Linux 内核的一部分，在用户空间，使用linuxptp 实现IEEE 1588 和IEEE 802.1AS-1 的PTP 协议。在TSN 中通过802.1AS 完成同步，它是IEEE1588 的一部分，并且做了某些扩展，例如在Wifi 中的时间同步。TNS 目前计划实现IEEE802.1AS-2020.它在主流Linux 上还没有实现。

值得一提的是，并非所有的以太网硬件接口都支持IEEE1588。有的设备上linux ptp 是软件实现的。精度大打折扣。

![img](https://i-blog.csdnimg.cn/blog_migrate/34307470804e993dd6440a96343b12f9.png)

#### PTP4L 的使用

在linux 中 ，使用Linux PTP 控制IEEE5188/PTP 的时间同步功能。

安装（已下载，在目录:[/media/jason/btrfs_disk_M/temp/linuxptp/](/media/jason/btrfs_disk_M/temp/linuxptp/)下）

```sh
git clone git://git.code.sf.net/p/linuxptp/code linuxptp
cd linuxptp
sudo make 
sudo make install
```

inuxptp主要包含2个程序：ptp4l和phc2sys

1.ptp4l 进行时钟同步，实时网卡时钟与远端的时钟同步(比如[TSN](https://so.csdn.net/so/search?q=TSN&spm=1001.2101.3001.7020)交换机），支持1588 和 802.1AS 两种协议

2.phc2sys 将网卡上的时钟同步到操作系统，或者反之.

```sh
ptp4l -i eth2 -2 –m -s -f gPTP.cfg 
phc2sys -a -r --transportSpecific=1 -m --step_threshold=1000
 
 
# phc2sys还可以写成这样，功能是一样的
phc2sys  -s eth2 -c CLOCK_REALTIME --transportSpecific=1 -m --step_threshold=1000 -w
```

Linux 中的调度算法
Linux 的应用程序通过socket 接口实现通信。网络协议栈处理数据包。网络协议有TCP/IP,UDP、IP和原始的以太网数据包和硬件控制。网络栈通过调度，类和过滤器等来配置。这些机制已经建立了许多年。由于TSN 是以太网的扩展，因此，TSN 的Linux 实现仍然使用这些机制。TSN直接将它集成到Linux 网络栈中，基本原理如下图所示：

![img](https://i-blog.csdnimg.cn/blog_migrate/9aef6dafbfb8e40c07ed6e690dcf6cac.png)

 Linux内核控制网络硬件的访问，保证数据包在正确的时间发送。这就是时间隙管理组件（Time Slot Management component）。然而内核需要正确的配置。由于需求来自于应用程序， 协议栈通过Qdiscs配置。

        Qdisc 对应一个数据包调度器，作为TSN 实现的一部分，各种Qdiscs 开发了出来，并且集成到主流Linux内核中。
数据包调度方法

  TSN 开发了几种数据包调度算法：

- MQPRIO

  MQPRIO qdisc 是一个简单的队列规则，可以通过优先级以及优先级对应的流量等级，将不同流量映射到硬件队列的区间。连续的流量等级可以1对1的映射到硬件的队列上。

- ETF (Earliest TxTime First) 

  ETF会在发送时间(txtime)到达之前的一段时间(delta) 缓存数据包，这个时间可以由delta选项控制。

- ETS

  ETS基于预先设定的周期性门控制列表，动态地为出口队列提供开/关控制的机制。ETS定义了一个时间窗口，是一个时间触发型网络（Time-trigged）。这个窗口在这个机制中是被预先确定的。这个门控制列表被周期性的扫描，并按预先定义的次序为不同的队列开放传输端口。

在Linux 中 ETS 被称为Time Aware Priority Shaper (TAPRIO)。我们·这里主要采用taprio 

Linux 中的配置方法
      使用Linux 原有的tc 命令来设置Qdiscs。

tc 命令

       Linux操作系统中的流量控制器TC（Traffic Control）在2001 年被导入，用于Linux内核的流量控制，主要是通过在输出端口处建立一个队列来实现流量控制。
```sh
tc qdisc ... dev dev parent classid [ handle major: ] taprio

       num_tc tcs

               map P0 P1 P2 ...  queues count1@offset1 count2@offset2

       ...

               base-time base-time clockid clockid

               sched-entry <command 1> <gate mask 1> <interval 1>

               sched-entry <command 2> <gate mask 2> <interval 2>

               sched-entry <command 3> <gate mask 3> <interval 3>

               sched-entry <command N> <gate mask N> <interval N>
```

num_tc：使用的流量等级，最大支持16个等级

map: 将优先级 (VLAN PRI) 0 -15映射到特定的流量等级

queues: 为每个流量等级提供队列的数量和范围。每个流量等级的队列范围不能重叠且必须连续

base-time：

指定调度的起始时刻，单位是纳秒，参考时钟由clockid指定。如果'base-time'是过去的时间，那么调度开始时间为base-time + (N * cycle-time)，其中N比现在时刻大的最小整数。"cycle-time" 所有调度项目的时间总和，也就是调度周期

一般默认设置为0（因为时钟是同步的，所以在所有设备上设置一个相同的值即可）

clockid：指定qdisc参考的内部时钟

sched-entry：

门控制的调度项目，sched-entry <command> <gatemask> <interval>

<command> 只支持"S"

<gate mask>  每一位（ bit）对应流量等级的开关状态（例如：bit 0 代表流量等级0/TC0）0  表示关闭，1表示打开

<interval>标志当前门控状态持续的时间

flags：

0x01:  txtime支持模式，qdisc通过发送时间戳判断数据包的发送周期

0x02:  full-offload模式, GCL直接传递给网卡，并且交给网卡执行，在这个模式下不需要设置clockid，默认网卡参考的时钟是/dev/ptpN (N 可以通过如下指令获取 ethtool -T eth0 | grep 'PTP Hardware Clock' ) (如果设备仅作为bridge，只需要通过ptp4l同步/dev/ptpN即可，并不需要phc2sys将/dev/ptpN同步到系统时钟)：

例子
![img](https://i-blog.csdnimg.cn/blog_migrate/4115376c093ec844633f112e0f52191b.png)

## 配置与使用

### 系统配置

​        在本实验中，我们使用一台DELL 的台式机和一台Atom x6000e 处理器为基础的工业电脑Up Squared 6000 。Dell 电脑中扩展了一个i210 网卡，它支持tsn。而Atom x6000e 带有两个千兆网口，并且支持tsn。两台机器通过网线直连。均运行ubuntu OS。

#### Step 1 VLAN 的配置

加载 8021q模块到内核:

```sh
sudo modprobe 8021q
```

**添加VLAN** 

```sh
sudo ip link add link enp4s0 name enp4s0.1 type vlan id 1 egress-qos-map 2:2 3:3
```

egress-qos-map 参数定义了linux 内部的包优先级SO_PRORITY到VLAN PCP 的映射关系。

**配置VLAN IP:**

```sh
sudo ip addr add 10.0.0.1/24 dev enp4s0.10
```

**激活VLAN:**

```sh
sudo ifconfig enp1s0.10 up
```

验证

可以使用 ifconfig 和ping 命令验证设置的VLAN

```sh
yao@yao-UPN-EHL01:~$ ping 10.0.0.1
PING 10.0.0.1 (10.0.0.1) 56(84) bytes of data.
64 bytes from 10.0.0.1: icmp_seq=1 ttl=64 time=0.055 ms
64 bytes from 10.0.0.1: icmp_seq=2 ttl=64 time=0.052 ms
64 bytes from 10.0.0.1: icmp_seq=3 ttl=64 time=0.051 ms
64 bytes from 10.0.0.1: icmp_seq=4 ttl=64 time=0.052 ms
```

注意：在本次实验中，一台设备的VLAN 设为 10,0.0.1 另一台设为 10.0.0.2.

#### Step 2 LinuxPTP 运行

主设备

```sh
sudo ptp4l  -i enp1s0  -m -H
```

从设备

```sh
sudo ptp4l  -i enp4s0  -m -H -s
```

与系统时钟建立联系

```sh
sudo phc2sys -m -s enp4s0 -w
```

#### 注意：linuxPTP 需要关闭防火墙！

```sh
ufw disbale
```

#### 运行的结果

![img](https://i-blog.csdnimg.cn/blog_migrate/944640174b50ff075e789bf95a09be52.png)

表明同步效果相当好。抖动12us ,时延21us

#### Step 3 在Linux 中配置整形和调度算法

```sh
tc qdisc replace dev enp4s0 parent root handle 100 taprio \
   num_tc 3 \
   map 0 0 1 2 0 0 0 0 0 0 0 0 0 0 0 0 \
   queues 1@0 1@1 1@2 \
   base-time 0000 \
   sched-entry S 4 125000 \
   sched-entry S 2 125000 \
   sched-entry S 1 250000 \
   clockid CLOCK_TAI
```

参数：

num_tc=3

时间片数量为3。

流量等级映射到VLAN优先级（PRI）

| PRI  |  0   |  1   |  2   | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   | 12   | 13   | 14   | 15   |
| ---- | :--: | :--: | :--: | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| TC   |  0   |  0   |  1   | 2    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    |

表示优先等级为2 的数据包映射到TC1，而优先等级3 的数据包映射到TC3。

queues 1@0 1@1 1@2

该参数表示映射时间片（TC）到硬件队列 。（注意Intel i210 默认硬件队列为4 有的文章中提到能够使用（ethtool -L  tx 3 来设置硬件队列数量，但是我尝试该条命令不成功） 

流量等级0 <-> 队列1,
流量等级1 <-> 队列2,
流量等级2 <-> 队列3
这样 PRI=2的数据包会通过队列2发送，这样 PRI=3的数据包会通过队列3发送。
调度方法
每个调度周期：

打开TC2（bit = 2，b100）对应的队列125000纳秒；

打开TC1 （bit = 1，b010）对应的队列125000纳秒,

打开TC0 （bit = 0，b001）对应的队列250000纳秒

网卡硬件offload： flags 2 （因此不需要clockid）

对应的队列结构。

![img](https://i-blog.csdnimg.cn/blog_migrate/e573a2b32313373edd0ada27225a5788.png)

使用程序测试：

```sh
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
 
int main(void)
{
	int ret = -1;
	 
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		printf("socket open err.");
		return -1;
	}
 
   
    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;                               
    local_addr.sin_addr.s_addr = inet_addr("10.0.0.1");       
    local_addr.sin_port = htons(6000);                              
    bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)); 
    int opt = 2; 
    setsockopt(sockfd , SOL_SOCKET , SO_PRIORITY , &opt , sizeof(opt));
   
    struct sockaddr_in sock_addr = {0};	
	sock_addr.sin_family = AF_INET;                         
	sock_addr.sin_port = htons(6000);						 
	sock_addr.sin_addr.s_addr = inet_addr("10.0.0.2");	 
	char sendbuf[]={"hello world, I am a UDP socket."};
    int cnt = 10;
    while(1)
    {
        ret = sendto(sockfd, sendbuf, sizeof(sendbuf)-1, 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	    //printf("ret = %d \n",ret);
        usleep(20000);
    }
 
   
    /*struct sockaddr_in recv_addr;
    socklen_t addrlen = sizeof(recv_addr);
    char recvbuf[1024] = {0};
    while(1)
    {
        ret = recvfrom(sockfd, recvbuf, 1024, 0,(struct sockaddr*)&recv_addr,&addrlen);  
        printf("[recv from %s:%d]%s \n",inet_ntoa(*(struct in_addr*)&recv_addr.sin_addr.s_addr),ntohs(recv_addr.sin_port),recvbuf);
    }*/
    
   
    close(sockfd);
}
```

在两台机器中，一个运行send另一个运行recieve。程序要略作修改。在wireshark 中可以检测到VLAN 中的内容。

同步情况的测试

  可以编写一个程序产生一个脉冲信号，通过示波器测量。

tsn 的性能，可以使用一个软件发送大量非实时信息，看实时数据是否受到影响。

结束语
         本文介绍了在标准的Linux OS 中，配置时间敏感网络的方法。下一步，要结合tsn 交换机做进一步测试。本人也是初学，敬请高人点拨。

原文链接：https://blog.csdn.net/yaojiawan/article/details/132016400