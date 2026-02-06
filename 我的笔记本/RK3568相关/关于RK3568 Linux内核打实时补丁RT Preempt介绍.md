# 关于RK3568 Linux内核打实时补丁RT Preempt介绍

【前言】在工业产品中使用操作系统，一般都需要使用实时性较强的操作系统，而众所周知，linux系统是基于时间片划分的非实时系统，其实时性难以满足工业化对时效性的要求，因此很多应用场景中无法使用linux操作系统。当然这一局限性已经有所改善，目前linux社区已经增加了众多版本的实时补丁，只要给linux内核打上实时补丁，其实时性会得到大幅度提升。“实时补丁”的主要工作就是针对Linux系统的优先级倒置、自旋锁等问题进行改进，以达到实时操作系统的要求。

## 1、环境说明

1.1、硬件平台：M3568-4GF16GLI-T（RK3568）

1.2、Linux内核版本：Linux4.19.193

## 2、下载RT Preempt补丁

```sh
Index of /pub/linux/kernel/projects/rt/
https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/
```

根据实际的Linux内核版本下载对应的RT Preempt补丁。博主使用的Linux内核版本为4.19.193，故在这里我们下载的补丁文件为：patch-4.19.193-rt81.patch.gz。（当然也可以下载其它压缩格式的文件）
![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/555daf5b643d76fb0f1945c3fa1df35d.png)

## 3、打补丁操作

### 3.1、解压补丁包

输入以下命令解压你刚下载的补丁包：

```sh
gunzip patch-4.19.193-rt81.patch.gz
```

### 3.2、将RT补丁打进内核文件

将解压出来的补丁文件patch-4.19.193-rt81.patch.gz复制到SDK包中的kernel目录中。

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/d49bb340fa41b613abe731d833ba0575.png)

随后执行如下指令：

```sh
sudo patch -p1 < patch-4.19.193-rt81.patch
```

至此完成打补丁工作。

> [!note]
>
> **注意：**直接从官网下载的补丁包，打入RK3568内核后，会出现很多错误，其原因是有些修改并未patch进内核源码中，详情请参见第6节。

## 4、配置并编译Linux内核

在kernel目录下，执行make ARCH=arm64 menuconfig 进入内核menuconfig菜单（执行前需保证内核根目录下的.config是arm64架构）。

进入   > General setup > Preemption Model

选择   (X) Fully Preemptible Kernel (RT) 

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/86bfe99b38a2caf2db86f3e9d7eeb3e4.png)

保存配置文件。

执行：

```sh
make ARCH=arm64 savedefconfig
```

导出配置文件，其命名为`defconfig`，用`defconfig`文件复制至`arch/arm64/configs/rockchip_linux_defconfig`文件（此处要根据你的实际情况去替换，博主的内核配置文件为`arch/arm64/configs/rockchip_linux_defconfig`）。

使用 ：

```sh
./build.sh kernel
```

指令去编译内核。（瑞芯微官方提供的内核编译脚本）

## 5、测试linux的实时性

前面我们已经生成了实时的linux内核镜像，接下来我们把实时内核烧写到开发板上，进行实时性性能测试。这里需要引入一个测试linux性能的工具`cyclictest`。

### 5.1、获取cyclictest工具

获取`cyclictest`工具有2种方式，一种是通过编译buildroot的方式获取，另外一种是下载cyclictest源码自行编译获取。

#### 5.1.1、通过buildroot获取**cyclictest**工具

**配置rt-tests选项方式一**：

进入buildroot根目录下，通过打开menuconfig菜单来配置rt-tests选项。操作方法与kernel的配置类似。

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/5bc8209c2ba327759dae79f94c7155c7.png)

**配置rt-tests选项方式二**：

直接在配置文件中添加rt-tests配置，如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/0cbf60ad9f441a6ee632f7f9d0c8fda4.png)

博主的配置文件为：`configs/rockchip_rk3568_defconfig`。（此处要根据自己的实际环境去找到对应的根文件系统的配置文件去修改） 

通过上述任一种方式配置好rt-tests选项后，使用 `./build.sh buildroot`指令去编译buildroot。（瑞芯微官方提供的根文件系统编译脚本）编译结束后根文件系统就自带了cyclictest工具。

#### 5.1.2、通过**cyclictest源码获取cyclictest**工具

##### 5.1.2.1、下载cyclictest源码

在ubuntu上下载rt_tests源码：

```sh
git clone git://git.kernel.org/pub/scm/utils/rt-tests/rt-tests.git
```

至此从Git 仓库中拉取了**cyclictest**工具的源码包。如下所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/c846bf326b3d154599b740917af407a0.png)

##### 5.1.2.2、切换分支

切换git分支进到源码的v1.0版本，具体操作如下：

```sh
cd rt-tests/
git checkout stable/v1.0
```

##### 5.1.2.3、修改Makefile

​    打开Makefile，指定交叉编译工具链和平台架构，其修改内容如下所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/65448d3982aaf84b4990409bf4dcbe5e.png)

##### 5.1.2.4、编译cyclictest源码

执行如下命令编译cyclictest源码：

```sh
make clean
make all NUMA=0  或 make
```

编译结束后，会在当前目录下生成cyclictest工具，如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/01d936ab6a466367f9060a66206b2ba1.png)

### 5.2、利用cyclictest进行性能测试

#### 5.2.1、cyclictest常用参数

| -p   | --prio=PRIO       | 最高优先级线程的优先级 ，使用方法为： -p 90 或 --prio=90     |
| ---- | ----------------- | ------------------------------------------------------------ |
| -m   | --mlockall        | 锁定当前和将来的内存分配。                                   |
| -c   | --clock=CLOCK     | 选择时钟，cyclictest -c 0     0 = CLOCK_MONOTONIC (默认)       1 = CLOCK_REALTIME |
| -i   | --interval=INTV   | 基本线程间隔，默认为1000（单位为us）                         |
| -l   | --loops=LOOPS     | 循环的次数，默认为0（无穷个），与 -i 间隔数结合可大致算出整个测试的时间，比如 -i 1000  -l 1000000 ,总的循环时间为1000*1000000=1000000000 us =1000s ，所以大致为16分钟多点。 |
| -n   | --nanosleep       | 使用 clock_nanosleep                                         |
| -h   | --histogram=US    | 在执行完后在标准输出设备上画出延迟的直方图（很多线程有相同的权限）US为最大的跟踪时间限制。 |
| -q   | --quiet           | 使用-q 参数运行时不打印信息，只在退出时打印概要内容，结合-h HISTNUM参数会在退出时打印HISTNUM 行统计信息以及一个总的概要信息。 |
| -t   | --threads         | 运行的线程数量，cyclictest t 15                              |
| -f   | --ftrace          | ftrace函数跟踪（通常与-b 配套使用，其实通常使用 -b 即可，不使用 -f ） |
| -b   | --breaktrace=USEC | 当延时大于USEC指定的值时，发送停止跟踪。USEC，单位为微秒（us） |

#### 5.2.2、cyclictest测试

如果是通过cyclictest源码编译生成的成cyclictest工具，则需要将其下载到开发板上，并给予其可执行权限；如果通过配置buildroot来生成的cyclictest工具，则无需其它操作，直接使用即可。接下来就使用cyclictest命令来进行linux内核实时性能测试，其测试命令为：

```sh
cyclictest -t 20 -p 80 -n -i 10000 -l 500000          (cyclictest路径在环境变量中)
或
./cyclictest -t 20 -p 80 -n -i 10000 -l 500000        (cyclictest路径在被执行的当前目录下)
```

#### 5.2.3、运行结果

##### 5.2.3.1、非实时系统测试结果

执行完上述指令后，其测试结果如下图： （运行时间大约83.3分钟）

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/c1026ae1b3dd346b0c15979b2a122e71.png)

##### 5.2.3.2、实时系统测试结果

执行完上述指令后，其测试结果如下图： （运行时间大约83.3分钟）

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/80a5cdb302489bbd6db844a29227f769.png)

##### 5.2.3.3、运行结果含义

| T： 0    | 序号为0的线程                               |
| -------- | ------------------------------------------- |
| P：80    | 线程优先级为80                              |
| C：9397  | 计数器。线程的时间间隔每达到一次，计数器加1 |
| I：10000 | 时间间隔为10000微秒(us)                     |
| Min:     | 最小延时(us)                                |
| Act:     | 最近一次的延时(us)                          |
| Avg：    | 平均延时(us)                                |
| Max：    | 最大延时(us)                                |

> [!note]
>
> 在我的4.19.232的内核配置中，编译选项为：
>
> Preemption Model (Preemptible Kernel (Low-Latency Desktop))  --->
>
> Timer frequency (1000 HZ)  ---> 
>
> 在这种情况下，运行`cyclictest`获得的测试结果为：

```sh
root@ECU-4000S:~# cyclictest -t 20 -p 80 -n -i 10000 -l 500000
# /dev/cpu_dma_latency set to 0us
policy: fifo: loadavg: 0.39 0.25 0.23 1/207 16233          

T: 0 (16214) P:80 I:10000 C:    449 Min:     13 Act:   39 Avg:   55 Max:     252
T: 1 (16215) P:80 I:10500 C:    427 Min:     13 Act:   43 Avg:   60 Max:     175
T: 2 (16216) P:80 I:11000 C:    408 Min:     12 Act:   41 Avg:   58 Max:     305
T: 3 (16217) P:80 I:11500 C:    390 Min:     13 Act:   41 Avg:   58 Max:     303
T: 4 (16218) P:80 I:12000 C:    374 Min:     12 Act:   37 Avg:   53 Max:     104
T: 5 (16219) P:80 I:12500 C:    359 Min:     13 Act:   42 Avg:   57 Max:     245
T: 6 (16220) P:80 I:13000 C:    345 Min:     13 Act:   42 Avg:   56 Max:     361
T: 7 (16221) P:80 I:13500 C:    332 Min:     13 Act:   42 Avg:   60 Max:     357
T: 8 (16222) P:80 I:14000 C:    320 Min:     13 Act:   49 Avg:   53 Max:     153
T: 9 (16223) P:80 I:14500 C:    309 Min:     13 Act:   45 Avg:   73 Max:     339
T:10 (16224) P:80 I:15000 C:    299 Min:     13 Act:   42 Avg:   56 Max:     262
T:11 (16225) P:80 I:15500 C:    289 Min:     12 Act:   47 Avg:   55 Max:     287
T:12 (16226) P:80 I:16000 C:    280 Min:     13 Act:   46 Avg:   56 Max:     286
T:13 (16227) P:80 I:16500 C:    272 Min:     13 Act:   42 Avg:   57 Max:     329
T:14 (16228) P:80 I:17000 C:    264 Min:     12 Act:   40 Avg:   55 Max:      93
T:15 (16229) P:80 I:17500 C:    256 Min:     14 Act:   41 Avg:   62 Max:     342
T:16 (16230) P:80 I:18000 C:    249 Min:     14 Act:   41 Avg:   56 Max:     278
T:17 (16231) P:80 I:18500 C:    242 Min:     14 Act:   63 Avg:   56 Max:     155
T:18 (16232) P:80 I:19000 C:    236 Min:     13 Act:   42 Avg:   58 Max:     303
T:19 (16233) P:80 I:19500 C:    230 Min:     14 Act:   41 Avg:   54 Max:     116
```

> [!note]
>
> 在我的5.10.110的内核配置中，编译选项为：
>
> Preemption Model (Preemptible Kernel (Low-Latency Desktop))  --->
>
> Timer frequency (1000 HZ)  ---> 
>
> 在这种情况下，运行`cyclictest`获得的测试结果为：

```sh
root@ECU-4000S:~# cyclictest -t 20 -p 80 -n -i 10000 -l 500000
# /dev/cpu_dma_latency set to 0us
policy: fifo: loadavg: 0.21 0.10 0.03 1/215 1257          

T: 0 ( 1238) P:80 I:10000 C:   1891 Min:     13 Act:   16 Avg:   63 Max:     577
T: 1 ( 1239) P:80 I:10500 C:   1801 Min:     14 Act:  118 Avg:   92 Max:     462
T: 2 ( 1240) P:80 I:11000 C:   1719 Min:     12 Act:   15 Avg:   61 Max:     509
T: 3 ( 1241) P:80 I:11500 C:   1644 Min:     13 Act:   19 Avg:   62 Max:     460
T: 4 ( 1242) P:80 I:12000 C:   1576 Min:     12 Act:   70 Avg:   60 Max:     519
T: 5 ( 1243) P:80 I:12500 C:   1513 Min:     13 Act:   69 Avg:   64 Max:     528
T: 6 ( 1244) P:80 I:13000 C:   1455 Min:     13 Act:   75 Avg:   60 Max:     564
T: 7 ( 1245) P:80 I:13500 C:   1401 Min:     13 Act:   96 Avg:   59 Max:     556
T: 8 ( 1246) P:80 I:14000 C:   1351 Min:     13 Act:   68 Avg:   58 Max:     535
T: 9 ( 1247) P:80 I:14500 C:   1304 Min:     11 Act:   18 Avg:   61 Max:     595
T:10 ( 1248) P:80 I:15000 C:   1261 Min:     13 Act:   58 Avg:   59 Max:     525
T:11 ( 1249) P:80 I:15500 C:   1220 Min:     14 Act:   72 Avg:   66 Max:     472
T:12 ( 1250) P:80 I:16000 C:   1182 Min:     13 Act:   73 Avg:   65 Max:     546
T:13 ( 1251) P:80 I:16500 C:   1146 Min:     15 Act:   24 Avg:   60 Max:     508
T:14 ( 1252) P:80 I:17000 C:   1112 Min:     13 Act:   20 Avg:   63 Max:     708
T:15 ( 1253) P:80 I:17500 C:   1080 Min:     13 Act:   73 Avg:   61 Max:     474
T:16 ( 1254) P:80 I:18000 C:   1050 Min:     13 Act:   97 Avg:   59 Max:     395
T:17 ( 1255) P:80 I:18500 C:   1022 Min:     11 Act:   19 Avg:   66 Max:     458
T:18 ( 1256) P:80 I:19000 C:    995 Min:     13 Act:   16 Avg:   66 Max:     742
T:19 ( 1257) P:80 I:19500 C:    969 Min:     12 Act:  217 Avg:   64 Max:     546
```

> [!important]
>
> **根据4.19.232和5.10.110内核实时性对比，感觉还是4.19.232的实时性会更好一些。因此，如果要设计实时性较强的平台或者系统，建议首选4.19.232。**

> [!note]
>
> 测试中还发现，如果在系统中先运行一个
>
> 死循环程序，并且将其用`taskset`绑定到具体的一个CPU上，再运行`cyclictest`测试，反而延时效果会更好：

这个死循环程序：`test_rt`的代码如下：

```c
#include <stdio.h>
void main(int argc, char* argv)
{
	for(int i = 0; i<=10000000000000; i++) {
		if(i == 100000000) {
			i = 0;
			printf("program is running!\n");
		}
	}
}
```

将其编译后在ECU4000S上的运行如下：

```sh
taskset -c 1 ./test_rt
```

也就是将其绑定到CPU1上运行。此时如果用top命令观察，可以看到：

```sh
Mem: 250520K used, 3613996K free, 2632K shrd, 11352K buff, 20356K cached
CPU0:  0.4% usr  5.8% sys  0.0% nic 92.6% idle  0.0% io  0.0% irq  1.0% sirq
CPU1:  100% usr  0.0% sys  0.0% nic  0.0% idle  0.0% io  0.0% irq  0.0% sirq
CPU2:  0.4% usr  1.4% sys  0.0% nic 98.1% idle  0.0% io  0.0% irq  0.0% sirq
CPU3:  0.8% usr  6.5% sys  0.0% nic 92.5% idle  0.0% io  0.0% irq  0.0% sirq
Load average: 1.12 1.16 0.93 4/213 2026
  PID  PPID USER     STAT   VSZ %VSZ CPU %CPU COMMAND
 2008  1142 root     R     1376  0.0   1 24.0 ./test_rt
```

CPU1的usr资源占用率达到了100%。然后再运行`cyclictest`程序：

```sh
taskset -c 3 cyclictest -p 80 -t 16 -d 500 -i 1000 -D 48h -m -a -n
```

可以看到程序运行输出信息为：

```sh
root@ECU-4000S:~# taskset -c 3 cyclictest -p 80 -t 16 -d 500 -i 1000 -D 48h -m -a -n
# /dev/cpu_dma_latency set to 0us
policy: fifo: loadavg: 1.36 1.26 1.00 2/213 2048          

T: 0 ( 2011) P:80 I:1000 C: 155444 Min:      9 Act:   11 Avg:   11 Max:     111
T: 1 ( 2012) P:80 I:1500 C: 103624 Min:      9 Act:   10 Avg:   11 Max:      57
T: 2 ( 2013) P:80 I:2000 C:  77718 Min:      9 Act:   11 Avg:   11 Max:      51
T: 3 ( 2014) P:80 I:2500 C:  62167 Min:      9 Act:   12 Avg:   12 Max:      59
T: 4 ( 2015) P:80 I:3000 C:  51803 Min:      9 Act:   11 Avg:   13 Max:     131
T: 5 ( 2016) P:80 I:3500 C:  44399 Min:      9 Act:   11 Avg:   12 Max:      48
T: 6 ( 2017) P:80 I:4000 C:  38847 Min:      9 Act:   11 Avg:   12 Max:      33
T: 7 ( 2018) P:80 I:4500 C:  34528 Min:      9 Act:   13 Avg:   13 Max:      59
T: 8 ( 2019) P:80 I:5000 C:  31073 Min:      9 Act:   14 Avg:   13 Max:      93
T: 9 ( 2020) P:80 I:5500 C:  28247 Min:     10 Act:   14 Avg:   13 Max:      35
T:10 ( 2021) P:80 I:6000 C:  25891 Min:     10 Act:   11 Avg:   13 Max:      31
T:11 ( 2022) P:80 I:6500 C:  23898 Min:     10 Act:   14 Avg:   13 Max:      49
T:12 ( 2023) P:80 I:7000 C:  22189 Min:      9 Act:   12 Avg:   15 Max:     118
T:13 ( 2024) P:80 I:7500 C:  20709 Min:     11 Act:   14 Avg:   13 Max:      29
T:14 ( 2025) P:80 I:8000 C:  19413 Min:     10 Act:   14 Avg:   14 Max:      28
T:15 ( 2026) P:80 I:8500 C:  18271 Min:     11 Act:   16 Avg:   14 Max:      39
```

##### 5.2.3.4、测试结果分析

在线程数和运行时间等条件均相同的情况下：

（1）对于非实时系统：最小时延在20us左右，平均时延在65us左右，最大时延在3500us左右。

**（2）对于打了RT补丁的实时系统：最小时延在10us左右，平均时延在15us左右，最大时延在300us左右。**

通过测试数据分析，打了RT补丁的Linux系统其时延性得到了大幅度改善。所以对于实时性要求较高的系统，还是需要给linux内核打上实时的补丁。 

## 6、遇到的问题

### 6.1、报类似**Hunk #6 FAILED at 107.** 错误

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/09a603c37e94985a760513024a1b4830.png)

#### 6.1.1、错误原因：

这个错误信息表明在尝试修改（patching）arch/arm64/crypto/Kconfig文件时，第6个补丁（Hunk）失败了。具体来说，补丁在第107行处失败。由于一个补丁中有多个失败，所以总共有6个补丁失败。错误信息还指出，这些失败的补丁将被保存到arch/arm64/crypto/Kconfig.rej文件中。 这通常意味着在尝试应用一个补丁或进行某种代码更改时，有一些更改与原始文件不匹配，因此没有被成功应用。这可能是由于文件已经更改、有冲突或其他原因。

> [!note]
>
> **注意：**所有未patch进kernel源码的内容会保存在xxx.rej文件中（xxx指代具体的kernel文件名）。例如下文的例子，未patch进内核文件的内容保存在了`arch/arm64/crypto/Kconfig.rej`文件中，而源文件名`为arch/arm64/crypto/Kconfig`。

**6.1.2、解决办法**：（目前只通过如下办法解决该问题，如有更好的方法欢迎评论区讨论!）

打开`arch/arm64/crypto/Kconfig.rej`文件，查看具体是哪一行和哪些更改被拒绝。博主的情况如下：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/89ab7ec5032cd981348d02212912025f.png)

根据上述没有正常patch进源文件的内容手动修改。从arch/arm64/crypto/Kconfig文件中找到对应的位置手动修改成arch/arm64/crypto/Kconfig.rej文件中对应指定的内容。具体如下所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/a74d6af0e64540eb6d0aa3b95d968204.png)

其它类似该问题的解决方法参考上文。

> [!note]
>
> **注意：**可以使用命令：**find . -name "\*.rej" -print** 查看当前kernel下有多少文件没有被patch进内核源码，然后可以逐一手动去修改需要patch的内容。

### 6.2、error: too few arguments to function 'try_to_wake_up'

具体报错情况如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/a28a185f80f2b8e3c1dc0e7276eb5c8b.png)

细究patch后的kernel/sched/core.c文件第2805行代码，调用try_to_wake_up();函数时确实是少了一个参数。通过对try_to_wake_up();函数的参数研究并分析该函数的调用前后文情况，博主目前将该函数缺少的第四个参数传一个1进去（并未深入研究其内部逻辑，如有大佬发现有不妥，恳请指正）。修改后的代码如下图所示：
![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/2aab82544779cb5f75ea302477396174.png)

### 6.3、error:'struct zram_table_entry' has no member named 'value'

具体报错情况如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/3928942bf86189c2d60762f43b7786c0.png)

上述错误显示 'struct  zram_table_entry'结构体中没有‘value’这个变量。分析发现patch-4.19.193-rt81.patch文件中，生成补丁的源码文件中'struct zram_table_entry'结构体中使用的是value变量，如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/b60cff50bc90a077f39dde2eca858c0a.png)

在博主的内核源码中，该结构体中使用的是flags变量，如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/3caa5cce36e86fb28c68ea0c73519c45.png)

故此处针对上述错误的**解决办法**为： 将patch进博主内核源码的相关value变量改成flags变量。修改后如下图所示： （文件：drivers/block/zram/zram_drv.c）

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/b68c02f8124c262d2072ae122aedb380.jpeg)

### 6.4、error: 'struct task_struct' has no member named 'cpus_allowed';

具体报错情况如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/6531ff698d940f908dba1fda971a0d6c.png)

cpus_allowed变量在patch补丁中是被屏蔽的，但新增了同数据类型的cpus_ptr变量。通过对for_each_cpu_and();和cpumask_test_cpu();各自参数的研究，分析函数被调用前后文情况和patch文件的如下部分内容：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/2ba74dedc20e0fea6bea0db50dca87a9.png)

针对上述问题的**解决办法**：用patch后新增加的cpus_ptr变量去替换cpus_allowed变量。（目前不知道这样修改是否有什么风险，恳请知道的大佬留言指正！）

具体修改如下图所示：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/d7eb5b5536fda5a8cd2aa8eb8632cd61.png)

## 7、参考文献

### 7.1、RT Preempt相关

[altimeter:documentation:howto:applications:start [Wiki] (linuxfoundation.org)](https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/start)

[实时Linux内核（PREEMPT_RT）的编译安装以及测试_实时内核-CSDN博客](https://blog.csdn.net/v6543210/article/details/80941906)

[stm32mp157 rt-preempt测试-CSDN博客](https://blog.csdn.net/puweixii/article/details/129262366)

[linux打实时补丁以及实时性能测试_linux cyclictest测试实时补丁-CSDN博客](https://blog.csdn.net/m0_37765662/article/details/109595439)

### 7.2、cyclictest测试工具相关

[cyclictest 测试以及原理浅谈-CSDN博客](https://blog.csdn.net/longerzone/article/details/16897655)

[实时性测试：cyclictest详解 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/336381111)

[编译RT-tests进行Linux实时性测试-CSDN博客](https://blog.csdn.net/kl1125290220/article/details/78560220)

【结束语】因技术能力有限，文章如有不妥之处，恳请各位技术大佬留言指正！ 
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/m0_56121792/article/details/134897863