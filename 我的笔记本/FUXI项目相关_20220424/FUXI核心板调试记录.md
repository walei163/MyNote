# FUXI——万力达DTU-2660板子调试V1.0

### 2022-09-09：**调试基于GPMC总线的FPGA驱动**

### 一、将GPMC驱动编译到内核代码树中：

与普通的驱动不同，fuxi_gpmc.c的内核代码树配置需要按照以下几个步骤进行：

1、将fuxi_gpmc.c复制到drivers/memory目录下。

2、修改arch/csky/Kconfig文件，添加FUXI_GPMC宏定义，如下所示：

```sh
config CPU_CK860
	bool "CSKY CPU ck860"
	select CPU_HAS_TLBI
	select CPU_HAS_CACHEV2
	select CPU_HAS_LDSTEX
	select CPU_HAS_FPUV2
	select FUXI_GPMC
endchoice
```

3、配置内核：

```sh
make ARCH=csky CROSS_COMPILE=csky-abiv2-linux- menuconfig
```

选择：



“Memory Controller drivers”选中后，退出配置菜单，重新编译内核，则fuxi_gpcm.c就编译进内核了。如下所示：

```sh
$ ./sh_csky 
  DTC     arch/csky/boot/dts/fx6evb_860mp_sanway_tf.dtb
  DTC     arch/csky/boot/dts/fx6evb_860mp_sanway_gpmc.dtb
  DTC     arch/csky/boot/dts/fx6evb_860mp.dtb
  DTC     arch/csky/boot/dts/fx6evb_860mp_PDM.dtb
  DTC     arch/csky/boot/dts/fx6evb_860mp_sanway_spi.dtb
  CALL    scripts/checksyscalls.sh
  CHK     include/generated/compile.h
  CC      drivers/memory/fuxi-gpmc.o
  AR      drivers/memory/built-in.a
  AR      drivers/built-in.a
  GEN     .version
  CHK     include/generated/compile.h
  UPD     include/generated/compile.h
  CC      init/version.o
  AR      init/built-in.a
  AR      built-in.a
  LD      vmlinux.o
  MODPOST vmlinux.o
WARNING: vmlinux: 'memcpy' exported twice. Previous export was in vmlinux
WARNING: vmlinux.o (.csky_stack_size): unexpected non-allocatable section.
Did you forget to use "ax"/"aw" in a .S file?
Note that for example <linux/init.h> contains
section definitions for use in .S files.

  KSYM    .tmp_kallsyms1.o
  KSYM    .tmp_kallsyms2.o
  LD      vmlinux
  SYSMAP  System.map
  OBJCOPY arch/csky/boot/Image
  Building modules, stage 2.
  MODPOST 14 modules
WARNING: vmlinux: 'memcpy' exported twice. Previous export was in vmlinux
  Kernel: arch/csky/boot/Image is ready
  GZIP    arch/csky/boot/zImage
  Kernel: arch/csky/boot/zImage is ready
```

### 二、修改DTB设备文件树：

1、在fx6_860mp.dtsi文件中添加如下代码：

```c
pwm3: xpwm@a0109000 {
	compatible = "sec-chip,xpwm";
	reg = <0xa0109000 0x1000>;
	clocks = <&dummy_apb>;
	clock-names = "xpwm";
	pinctrl-names = "default";
	pinctrl-0 = <&pwm3_pins>;
	status = "disabled";
};
gpmc: gpmc@a1d00000 {
	compatible = "sec-chip,gpmc";
	reg = <0xa1d00000 0x1000>;
	clocks = <&dummy_apb>;
	clock-names = "gpmc";
	pinctrl-names = "default";
	pinctrl-0 = <&gpmc_pins>;
	status = "disabled";
};
sysctrl: system-controller@0xa0202000 {
	compatible = "syscon";
	reg = <0xa0202000 0x1000>;
};
```

2、在fx6evb_860mp_sanway.dts文件中添加如下代码：

```c
&gpmc {
	status = "okay";
	#address-cells = <1>;
	#size-cells = <1>;
	gpmc,num-cs = <4>;
	ranges = <0x0 0xA1A00000 0x10000>;
	sram0@0 {

		interrupt-parent = <&port5>;
		interrupts = <9 IRQ_TYPE_EDGE_RISING>;
		eeprom-wp-gpios = <&port2 19 GPIO_ACTIVE_LOW>;	//lcd_d0，拉低为允许写入EEPROM 

		compatible = "gpmc,sram0";
		#address-cells = <1>;
		#size-cells = <1>;
		gpmc,cs = <0>;
		gpmc,reg_sel = <0>;
		gpmc,mem_type ="sram";
		gpmc,width = <16>;
		reg = <0 0x10000>;	//0x10000: 65535, 64KB

		gpmc,t_rc = <14>;
		gpmc,t_as = <1>;
		gpmc,t_wr = <1>;
		gpmc,t_wp = <11>;
		gpmc,t_bta = <5>;
	};
};

&spi2 {
	status = "okay";

	spidev2: spidev@0 {
		compatible = "sanway,spi_fpga_cmd";
		spi-max-frequency = <20000000>;
		reg = <0>;
	};

	spidev3: spidev@1 {
		compatible = "rohm,dh2228fv";
		spi-max-frequency = <20000000>;
		reg = <1>;
	};

};
```

完整的文件压缩包下载:

[dts_fuxi_fx6evb_860mp_sanway_gpmc_20220911.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/1d46f034-35ce-465f-b205-4fbd2c35a5cf/dts_fuxi_fx6evb_860mp_sanway_gpmc_20220911.tar.gz)

### 三、测试驱动：

1、将驱动编译为模块方式，insmod加载，但是驱动代码没有初始化，经研究源代码发现，probe函数中没有为gpmc建立内存节点，需要修改驱动代码。在probe函数的结尾，添加of_platform_populate，具体如下：

```c
return of_platform_populate(dev->of_node, NULL, NULL, dev);
```

完整驱动代码下载：

[fuxi_gpmc_new_20220911.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/20babd5b-b113-4829-8eac-2a9b8236ee60/fuxi_gpmc_new_20220911.tar.gz)

2、此时FPGA驱动可以正确加载了，但是读写GPMC总线仍然不对，读出的数据始终为0。后经过仔细研究FPGA代码发现FUXI的板子GPMC总线这块有以下几个问题：

```c
1、设置中是按照段地址gpcm_ncs0来访问，实际发出的是gpcm_ncs1。将地址译码中的0和1调换一下：

parameter ADC_BANK = 1, MISC_BANK = 0;
此时可以正常读数据了。

2、GPMC写数据仍旧不正常，发现不管怎么写，gpcm_nwe始终未拉低。目前尚不清楚原因。解决方法：

结合原来的SPI读写方案，用SPI_CMD那1路来对FPGA片内寄存器进行写操作，而用GPMC来进行寄存器和FIFO的读操作，修改FPGA代码后，读写正常了。

3、地址线ADDR0在8位模式下与16模式该如何区分：

经过实测，地址线ADDR0在16位模式下自动变为ADDR1，因此译码时不管8位还是16位模式，ADDR0均需要参与译码。
```

完整的新的添加了GPMC读FPGA的verilog源码下载：

[dtu_2660_fuxi_gpmc_new_20220911.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/3f0854c5-5b96-41f2-8011-6e1797cd9631/dtu_2660_fuxi_gpmc_new_20220911.tar.gz)

**3、GPMC总线寄存器和时序设置：**

按照DTB中的设置，需要做以下几个方面的设置：

3.1、芯片选择寄存器：

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/7e57adc9-3c68-4263-8ea3-2cd8d7cd403a/Untitled.png)

```c
GPMC的地址映射空间起始地址为：0xa1a00000，大小最大为3MB（具体说明见用户手册）。当前dtb中映射了64KB：
ranges = <0x0 0xA1A00000 0x10000>;
```

3.2、地址屏蔽寄存器SMSKRn：

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/428a9597-8b9d-4dae-83f9-2a29acb6156d/Untitled.png)

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/9e7febcf-a84d-4453-b67c-87ed04b3ba56/Untitled.png)

对应该寄存器，DTB中的设置为：

```c
gpmc,reg_sel = <0>;
gpmc,mem_type ="sram";
reg = <0 0x10000>;	//0x10000: 65535, 64KB
```

3.3、静态存储器计时寄存器：SMTMGR_SETn:

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/d101e6db-b417-4f9e-a7a8-307df603a547/Untitled.png)

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/0df62e16-0f13-4819-b2d7-9dffbd92e447/Untitled.png)

对应该寄存器，DTB中的设置为：

```c
gpmc,t_rc = <14>;
gpmc,t_as = <1>;
gpmc,t_wr = <1>;
gpmc,t_wp = <11>;
gpmc,t_bta = <5>;
```

3.4、静态存储器控制寄存器：SMCTLR：

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/938615fd-0d53-4498-bac8-8e4d260e257c/Untitled.png)

与该寄存器对应的DTB中的设置为：

```c
gpmc,width = <16>;
```

4、FPGA的Linux驱动代码：

本次驱动代码做了几点变动：

4.1 将SPI驱动和GPMC驱动统一到了一个代码模块中进行注册。

4.2 将中断注册从open函数中移到了probe函数中。

4.3 其它接口如ioctl均保持与原来一致。但是应用程序中的signal回调函数中的读写需要做一些修改。

完整的驱动代码下载：

[cpld_dtu_2660_fuxi_gpmc.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/9c86ef2a-c571-4071-8725-ca740c5be88b/cpld_dtu_2660_fuxi_gpmc.tar.gz)

测试程序下载：

[test_mmap.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/e4d16dbe-eb2f-438f-beef-581f6dc8fc93/test_mmap.tar.gz)

注意驱动模块加载时的参数：

```bash
insmod dtu_2660_fuxi_gpmc.ko adc_channels_number=23
```

以及测试程序运行时的加载参数：

```bash
./test_dtu2660_fuxi_gpmc 23 250 460
```

2022-06-23：

1. 硬件基于：FUXI-H_PDMOD_V1.0.1 20220302，软件基于SDK2.4版本的Linux系统烧写刷机步骤：

   1. 需要一张TF卡，fat32分区，卡上要有内核映像、设备文件树等文件。

   2. 新的核心板默认u-boot不支持fat32分区读写，因此需要先启动系统进入Linux下，刷入一个支持读写fat32分区的u-boot。Linux下的刷写命令为：

      由于原厂的u-boot烧写映像更新到了5月26日，而非4月11日的映像，导致U-boot下不再支持TF卡，无法进行烧写。

      需要先启动到Linux系统下，然后运行：

      ```bash
      rz
      ```

      通过串口的ZMODEM协议将旧的u-boot-spl.brn和u-boot.img上传到文件系统下，然后再用dd命令烧写到u-boot分区：

      U-boot.img烧写分区：

      ```bash
      dd if=u-boot-spl.brn of=/dev/mtdblock0
      dd if=u-boot.img of=/dev/mtdblock6
      ```

      重启到u-boot下，就可以用TF卡访问了。接下来就可以在u-boot下更新DTB和内核等。

   3. 启动系统进入u-boot下，假设内核映像文件名：uImage，设备文件树文件名：fx6evb_860mp_sanway.dtb，运行以下命令：

      ```bash
      mmc dev 0
      fatls mmc 0
      fatload mmc 0 0x08000000 fx6evb_860mp_sanway.dtb
      sf probe 7:0
      sf update 0x08000000 0x480000 0x80000
      ```

      以上是烧写DTB设备文件树

      ```bash
      mmc dev 0
      fatls mmc 0
      fatload mmc 0 0x08000000 uImage
      sf probe 7:0
      sf update 0x08000000 0x500000 0x900000
      ```

      以上是烧写内核映像

      ```bash
      mmc dev 0
      fatls mmc 0
      fatload mmc 0 0x08000000 uImage
      sf probe 7:0
      sf update 0x08000000 0x300000 0x100000
      ```

      以上是烧写u-boot.img

   4. 可以

2. 其它说明：

关于SPI Flash的分区：

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/0a9296b9-2952-46cd-bab7-68324969b5eb/Untitled.png)

# 1、硬件设计

[DTU_2660_FUXI_V1.0-20220305.pdf](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/7ff9ee5c-23e8-4e9b-8140-10a7b22fc33a/DTU_2660_FUXI_V1.0-20220305.pdf)

# 2、调试记录

## 1）问题：

1、上电后不启动：

经查是复位按钮封装搞错了，导致一直处于复位状态，将复位按钮从板子上去掉，上电调试窗口会打印数据。

2、板子一直在复位：

FUXI V1.0.1核心板默认看门狗是打开的，需要将J4短接，然后重启，则可以正常启动了。

3、Linux下无法像板子上传文件：

查看启动日志：

```bash
[    0.000000] Linux version 4.19.15 (vmuser@ubuntu) (gcc version 6.3.0 (C-SKY Tools V3.10.17 Glibc-2.28.9000 Linux-4.19.15 abiv2 B20191123)) #1 SMP Fri Mar 11 15:32:18 +08 2022
[    0.000000] Phys. mem: 256MB
[    0.000000] random: get_random_bytes called from start_kernel+0x50/0x470 with crng_init=0
[    0.000000] percpu: Embedded 14 pages/cpu @(ptrval) s25612 r8192 d23540 u57344
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 64960
[    0.000000] Kernel command line: console=ttyS0,115200 init=/sbin/init root=/dev/mmcblk1p1 rw rootwait rootfstype=ext4 clk_ignore_unused loglevel=7
[    0.000000] Dentry cache hash table entries: 32768 (order: 5, 131072 bytes)
[    0.000000] Inode-cache hash table entries: 16384 (order: 4, 65536 bytes)
[    0.000000] Sorting __ex_table...
[    0.000000] Memory: 252360K/262144K available (4894K kernel code, 210K rwdata, 1496K rodata, 176K init, 315K bss, 9784K reserved, 0K cma-reserved, 0K highmem)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=2, Nodes=1
[    0.000000] rcu: Hierarchical RCU implementation.
[    0.000000] rcu: 	RCU restricting CPUs from NR_CPUS=8 to nr_cpu_ids=2.
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=2
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] clocksource: mptimer clocksource: mask: 0xffffffffffffff max_cycles: 0x159f229905, max_idle_ns: 440795206165 ns
[ 2920.577754] sched_clock: 56 bits at 93MHz, resolution 10ns, wraps every 4398046511103ns
[    4.476823] Console: colour dummy device 80x25
[    4.476868] Calibrating delay loop... 497.66 BogoMIPS (lpj=995328)
[    4.508812] pid_max: default: 32768 minimum: 301
[    4.508952] Mount-cache hash table entries: 1024 (order: 0, 4096 bytes)
[    4.508970] Mountpoint-cache hash table entries: 1024 (order: 0, 4096 bytes)
[    4.510492] ASID allocator initialised with 4096 entries
[    4.510604] rcu: Hierarchical SRCU implementation.
[    4.511103] smp: Bringing up secondary CPUs ...
[    4.511573] CPU1 Online: csky_start_secondary...
[    4.511713] smp: Brought up 1 node, 2 CPUs
[    4.512244] devtmpfs: initialized
[    4.515361] Duplicate name in lcdc@a0003000, renamed to "display#1"
[    4.519132] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    4.519161] futex hash table entries: 512 (order: 3, 32768 bytes)
[    4.519310] pinctrl core: initialized pinctrl subsystem
[    4.520151] NET: Registered protocol family 16
[    4.520414] audit: initializing netlink subsys (disabled)
[    4.520662] audit: type=2000 audit(0.044:1): state=initialized audit_enabled=0 res=1
[    4.521513] DMA: preallocated 256 KiB pool for atomic coherent pool
[    4.521526] DMA: vaddr: 0xc001b000 phy: 0xf900000,
[    4.535549] dw_dmac a0001000.dma-controller: DesignWare DMA Controller, 4 channels
[    4.535974] dw_dmac a000d000.dma-controller: DesignWare DMA Controller, 4 channels
[    4.536430] SCSI subsystem initialized
[    4.536688] usbcore: registered new interface driver usbfs
[    4.536747] usbcore: registered new interface driver hub
[    4.536840] usbcore: registered new device driver usb
[    4.538536] clocksource: Switched to clocksource mptimer clocksource
[    4.538733] VFS: Disk quotas dquot_6.6.0
[    4.538810] VFS: Dquot-cache hash table entries: 1024 (order 0, 4096 bytes)
[    4.546381] NET: Registered protocol family 2
[    4.547189] tcp_listen_portaddr_hash hash table entries: 512 (order: 0, 6144 bytes)
[    4.547221] TCP established hash table entries: 2048 (order: 1, 8192 bytes)
[    4.547247] TCP bind hash table entries: 2048 (order: 2, 16384 bytes)
[    4.547274] TCP: Hash tables configured (established 2048 bind 2048)
[    4.547408] UDP hash table entries: 256 (order: 1, 8192 bytes)
[    4.547434] UDP-Lite hash table entries: 256 (order: 1, 8192 bytes)
[    4.547602] NET: Registered protocol family 1
[    4.548142] RPC: Registered named UNIX socket transport module.
[    4.548158] RPC: Registered udp transport module.
[    4.548166] RPC: Registered tcp transport module.
[    4.548173] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    4.549436] workingset: timestamp_bits=14 max_order=16 bucket_order=2
[    4.557873] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    4.559428] ntfs: driver 2.1.32 [Flags: R/O].
[    4.559788] jffs2: version 2.2. (NAND) © 2001-2006 Red Hat, Inc.
[    4.560234] romfs: ROMFS MTD (C) 2007 Red Hat, Inc.
[    4.560338] fuse init (API version 7.27)
[    4.565974] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 250)
[    4.565993] io scheduler noop registered
[    4.566004] io scheduler deadline registered (default)
[    4.566313] io scheduler cfq registered
[    4.566325] io scheduler mq-deadline registered (default)
[    4.566335] io scheduler kyber registered
[    4.567504] pinctrl-single a0202060.pinctrl: 256 pins, size 128
[    4.626708] Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
[    4.628406] console [ttyS0] disabled
[    4.628471] a0113000.serial: ttyS0 at MMIO 0xa0113000 (irq = 44, base_baud = 5859375) is a 16550A
[    5.082737] console [ttyS0] enabled
[    5.087227] a0114000.serial: ttyS1 at MMIO 0xa0114000 (irq = 45, base_baud = 5859375) is a 16550A
[    5.097014] a0115000.serial: ttyS2 at MMIO 0xa0115000 (irq = 46, base_baud = 5859375) is a 16550A
[    5.106812] a0116000.serial: ttyS3 at MMIO 0xa0116000 (irq = 47, base_baud = 5859375) is a 16550A
[    5.126470] brd: module loaded
[    5.138315] loop: module loaded
[    5.141846] sc-co a038e000.co: co0 probe ...
[    5.146457] sc-co a038e000.co: match smac co co8030
[    5.151508] sc-co a038f000.co: co1 probe ...
[    5.156127] sc-co a0108000.co: co2 probe ...
[    5.160717] sc-co a0924000.co: co3 probe ...
[    5.166894] sc-qspi a0992000.qspi: dma1chan0 for rx
[    5.171837] sc-qspi a0992000.qspi: dma1chan1 for tx
[    5.176822] sc-qspi a0992000.qspi: mx25l6405d (8192 Kbytes)
[    5.182445] 1 fixed-partitions partitions found on MTD device a0992000.qspi
[    5.189431] Creating 1 MTD partitions on "a0992000.qspi":
[    5.194854] 0x000000000000-0x000000a00000 : "rootfs"
[    5.199842] mtd: partition "rootfs" extends beyond the end of device "a0992000.qspi" -- size truncated to 0x800000
[    5.211229] sc-qspi a0992000.qspi: mtd registerd, cs: 0, bus width: 4
[    5.218022] sc-qspi a0993000.qspi: dma1chan2 for rx
[    5.222952] sc-qspi a0993000.qspi: dma1chan3 for tx
[    5.228159] sc-qspi a0993000.qspi: gd25q128 (16384 Kbytes)
[    5.233735] 10 fixed-partitions partitions found on MTD device a0993000.qspi
[    5.240807] Creating 10 MTD partitions on "a0993000.qspi":
[    5.246326] 0x000000000000-0x000000100000 : "fsbl"
[    5.252091] 0x000000100000-0x000000140000 : "nvm"
[    5.257684] 0x000000140000-0x000000160000 : "baremetal803.0"
[    5.264213] 0x000000160000-0x000000180000 : "baremetal803.1"
[    5.270745] 0x000000180000-0x000000200000 : "bootloader env"
[    5.277251] 0x000000200000-0x000000300000 : "bootloader"
[    5.283443] 0x000000300000-0x000000400000 : "baremetal1"
[    5.289597] 0x000000400000-0x000000480000 : "baremetal2"
[    5.295758] 0x000000480000-0x000000500000 : "device tree"
[    5.302001] 0x000000500000-0x000001000000 : "kernel"
[    5.307900] sc-qspi a0993000.qspi: mtd registerd, cs: 0, bus width: 4
[    5.316347] libphy: Fixed MDIO Bus: probed
[    5.320895] CAN device driver interface
[    5.326416] stmmaceth a0580000.ethernet: snps,phy-addr property is deprecated
[    5.333688] stmmaceth a0580000.ethernet: PTP uses main clock
[    5.339396] stmmaceth a0580000.ethernet: no reset control found
[    5.345646] stmmaceth a0580000.ethernet: User ID: 0x10, Synopsys ID: 0x37
[    5.352484] stmmaceth a0580000.ethernet: 	DWMAC1000
[    5.357386] stmmaceth a0580000.ethernet: DMA HW capability register supported
[    5.364543] stmmaceth a0580000.ethernet: RX Checksum Offload Engine supported
[    5.371698] stmmaceth a0580000.ethernet: COE Type 2
[    5.376593] stmmaceth a0580000.ethernet: TX Checksum insertion supported
[    5.383312] stmmaceth a0580000.ethernet: Wake-Up On Lan supported
[    5.389423] stmmaceth a0580000.ethernet: Enhanced/Alternate descriptors
[    5.396055] stmmaceth a0580000.ethernet: Enabled extended descriptors
[    5.402520] stmmaceth a0580000.ethernet: Ring mode enabled
[    5.408027] stmmaceth a0580000.ethernet: Enable RX Mitigation via HW Watchdog Timer
[    5.470606] libphy: stmmac: probed
[    5.474032] mdio_bus stmmac-0:00: attached PHY driver [unbound] (mii_bus:phy_addr=stmmac-0:00, irq=POLL)
[    5.483558] mdio_bus stmmac-0:02: attached PHY driver [unbound] (mii_bus:phy_addr=stmmac-0:02, irq=POLL)
[    5.493921] stmmaceth a0582000.ethernet: snps,phy-addr property is deprecated
[    5.501164] stmmaceth a0582000.ethernet: PTP uses main clock
[    5.506855] stmmaceth a0582000.ethernet: no reset control found
[    5.513003] stmmaceth a0582000.ethernet: User ID: 0x10, Synopsys ID: 0x37
[    5.519844] stmmaceth a0582000.ethernet: 	DWMAC1000
[    5.524758] stmmaceth a0582000.ethernet: DMA HW capability register supported
[    5.531915] stmmaceth a0582000.ethernet: RX Checksum Offload Engine supported
[    5.539070] stmmaceth a0582000.ethernet: COE Type 2
[    5.543962] stmmaceth a0582000.ethernet: TX Checksum insertion supported
[    5.550679] stmmaceth a0582000.ethernet: Wake-Up On Lan supported
[    5.556789] stmmaceth a0582000.ethernet: Enhanced/Alternate descriptors
[    5.563420] stmmaceth a0582000.ethernet: Enabled extended descriptors
[    5.569875] stmmaceth a0582000.ethernet: Ring mode enabled
[    5.575378] stmmaceth a0582000.ethernet: Enable RX Mitigation via HW Watchdog Timer
[    5.637943] libphy: stmmac: probed
[    5.641381] stmmaceth a0582000.ethernet: No PHY found
[    5.646543] stmmaceth a0582000.ethernet: stmmac_dvr_probe: MDIO bus (id: 1) registration failed
[    5.655945] dwc2 a0006000.usb: a0006000.usb supply vusb_d not found, using dummy regulator
[    5.664374] dwc2 a0006000.usb: Linked as a consumer to regulator.0
[    5.670592] dwc2 a0006000.usb: a0006000.usb supply vusb_a not found, using dummy regulator
[    5.690551] dwc2 a0006000.usb: dwc2_check_params: Invalid parameter lpm=1
[    5.697374] dwc2 a0006000.usb: dwc2_check_params: Invalid parameter lpm_clock_gating=1
[    5.705310] dwc2 a0006000.usb: dwc2_check_params: Invalid parameter besl=1
[    5.712201] dwc2 a0006000.usb: dwc2_check_params: Invalid parameter hird_threshold_en=1
[    5.720474] dwc2 a0006000.usb: DWC OTG Controller
[    5.725241] dwc2 a0006000.usb: new USB bus registered, assigned bus number 1
[    5.732334] dwc2 a0006000.usb: irq 57, io mem 0xa0006000
[    5.737804] dwc2 a0006000.usb: Linked as a consumer to regulator.1
[    5.744865] hub 1-0:1.0: USB hub found
[    5.748703] hub 1-0:1.0: 1 port detected
[    5.753314] usbcore: registered new interface driver usb-storage
[    5.759668] i2c /dev entries driver
[    5.764204] sdhci: Secure Digital Host Controller Interface driver
[    5.770422] sdhci: Copyright(c) Pierre Ossman
[    5.806225] mmc0: SDHCI controller on a0002000.sdhci [a0002000.sdhci] using ADMA
[    5.844958] mmc1: SDHCI controller on a0004000.sdhci [a0004000.sdhci] using ADMA
[    5.852555] sdhci-pltfm: SDHCI platform and OF driver helper
[    5.858959] usbcore: registered new interface driver usbhid
[    5.864564] usbhid: USB HID core driver
[    5.870175] IPVS: Registered protocols ()
[    5.874446] IPVS: Connection hash table configured (size=4096, memory=32Kbytes)
[    5.882022] IPVS: ipvs loaded.
[    5.885525] ipt_CLUSTERIP: ClusterIP Version 0.8 loaded successfully
[    5.892643] NET: Registered protocol family 10
[    5.898495] Segment Routing with IPv6
[    5.902732] sit: IPv6, IPv4 and MPLS over IPv4 tunneling driver
[    5.909489] NET: Registered protocol family 17
[    5.914071] bridge: filtering via arp/ip/ip6tables is no longer available by default. Update your scripts to load br_netfilter if you need this.
[    5.927217] can: controller area network core (rev 20170425 abi 9)
[    5.933694] NET: Registered protocol family 29
[    5.938218] can: raw protocol (rev 20170425)
[    5.942528] can: broadcast manager protocol (rev 20170425 t)
[    5.948215] can: netlink gateway (rev 20170425) max_hops=1
[    5.953869] 8021q: 802.1Q VLAN Support v1.8
[    5.960588] rtc-ds3232: probe of 0-0068 failed with error -121
[    5.967848] lm75: probe of 1-004d failed with error -121
[    5.973399] pinctrl-single a0202060.pinctrl: pin PIN33 already requested by a0004000.sdhci; cannot claim for a011d000.i2c
[    5.984408] pinctrl-single a0202060.pinctrl: pin-33 (a011d000.i2c) status -22
[    5.991584] pinctrl-single a0202060.pinctrl: could not request pin 33 (PIN33) from group pinctrl_i2c2_pins  on device pinctrl-single
[    6.003539] i2c_designware a011d000.i2c: Error applying setting, reverse things back
[    6.011347] i2c_designware: probe of a011d000.i2c failed with error -22
[    6.018172] pinctrl-single a0202060.pinctrl: pin PIN35 already requested by a0004000.sdhci; cannot claim for a0107000.i2c
[    6.029179] pinctrl-single a0202060.pinctrl: pin-35 (a0107000.i2c) status -22
[    6.036349] pinctrl-single a0202060.pinctrl: could not request pin 35 (PIN35) from group pinctrl_i2c3_pins  on device pinctrl-single
[    6.048294] i2c_designware a0107000.i2c: Error applying setting, reverse things back
[    6.056091] i2c_designware: probe of a0107000.i2c failed with error -22
[    6.062909] hctosys: unable to open rtc device (rtc0)
[    6.068202] clk: Not disabling unused clocks
[    6.073090] mmc1: new high speed MMC card at address 0001
[    6.078619] ttyS0 - failed to request DMA
[    6.079728] mmcblk1: mmc1:0001 88A398 7.28 GiB 
[    6.088099] mmcblk1boot0: mmc1:0001 88A398 partition 1 4.00 MiB
[    6.094903] mmcblk1boot1: mmc1:0001 88A398 partition 2 4.00 MiB
[    6.101037] mmcblk1rpmb: mmc1:0001 88A398 partition 3 4.00 MiB, chardev (246:0)
[    6.109654]  mmcblk1: p1
[    6.174307] random: fast init done
[    6.181007] EXT4-fs (mmcblk1p1): recovery complete
[    6.186266] EXT4-fs (mmcblk1p1): mounted filesystem with ordered data mode. Opts: (null)
[    6.194448] VFS: Mounted root (ext4 filesystem) on device 179:1.
[    6.200979] devtmpfs: mounted
[    6.204049] Freeing unused kernel memory: 176k freed
[    6.209040] This architecture does not have kernel memory protection.
[    6.215501] Run /sbin/init as init process
[    6.462996] systemd[1]: System time before build time, advancing clock.
[    6.476451] systemd[1]: Failed to lookup module alias 'autofs4': Function not implemented
[    6.501656] systemd[1]: systemd 244 running in system mode. (-PAM -AUDIT -SELINUX -IMA -APPARMOR -SMACK +SYSVINIT +UTMP -LIBCRYPTSETUP -GCRYPT -GNUTLS -ACL -XZ -LZ4 -SECCOMP +BLKID -ELFUTILS +KMOD -IDN2 -IDN -PCRE2 default-hierarchy=hybrid)
[    6.523861] systemd[1]: Detected architecture csky.
```

发现默认系统中是配置了USB驱动的，因此可以在板子上将USB0这一路引出来，外接U盘进行调试：

用1个USB TYPE-A接口，焊接CE10的正负极作为USB TYPE-A的电源和地，同时将板子上编号TP1焊接USB_D+，TP2焊接USB_D-，即可使用了。

4、默认网络不通的情况下如何调试：

USB可以使用之后，可以外接一个带有网口的USB HUB设备，然后在内核中编译模块：usbnet.ko，ax88179_178a.ko，拷贝到U盘上，然后复制到文件系统中，然后再插入USB HUB设备，并在文件系统下输入：

```bash
insmod usbnet.ko
insmod ax88179_178a.ko
```

此时就可以使用USB网卡来调试了。

USB网卡通了之后，可以使用nfs网络文件系统来挂载虚拟机上的文件系统进行调试：

```bash
busybox mount  -t nfs -o nolock 192.168.1.177:/mnt/fuxi_ck860fv /mnt/nfs/
```

**注意：FUXI板子上的nfs系统挂载时，需要加上前缀：busybox。**

5、DTB修改说明：

1）默认板子上需要配置cmdline参数为：

```bash
chosen {
	bootargs = "console=ttyS0,115200 init=/sbin/init root=/dev/mmcblk1p1 rw rootwait rootfstype=ext4 clk_ignore_unused loglevel=7";
};

&sdio0 {
	status = "okay";
};

&sdio1 {
	status = "okay";
};
```

也就是需要将SDIO0和SDIO1都打开，其中SDIO0对应TF卡槽，SDIO1对应eMMC，Linux系统启动时需要从mmcblk1p1加载根文件系统，这里默认因为原来的dtb里面没有打开SDIO0，导致SDIO1的编号对应的mmcblk1变为了mmcblk0，导致我编译的dtb烧写进去之后系统无法启动。

最新的bootargs：

```bash
bootargs = "console=ttyS0,115200 init=/sbin/init root=/dev/mmcblk1p2 rw rootwait rootfstype=ext4 idle=poll isolcpus=1 clk_ignore_unused loglevel=7";
```

2）将SDIO0（也就是TF卡那一路）修改为可支持热插拔的TF卡。需要修改文件：fx6_860mp.dtsi：

```c
sdio0: sdhci@a0002000{
	compatible = "scmmc";
	reg = <0xa0002000 0x1000>;
	interrupt-parent = <&intc>;
	interrupts = <54>;
	//non-removable;
	broken-cd;
	bus-width = <4>;
	clocks = <&sdclk>, <&hclk>;
	clock-names = "sdclk1x", "ahbclk";
	pinctrl-names = "default";
	pinctrl-0 = <&sdio0_pins>;
	status = "disabled";
};

sdio0_pins: pinctrl_sdio0_pins {
	pinctrl-single,bits = <
		0x04 0x10000000 0xf0000000	/* SDHC0_CLK */
		0x08 0x00000001 0x0000000f	/* SDHC0_CMD */
		0x08 0x00000010 0x000000f0	/* SDHC0_DAT0 */
		0x08 0x00000100 0x00000f00	/* SDHC0_DAT1 */
		0x08 0x00001000 0x0000f000	/* SDHC0_DAT2 */
		0x08 0x00010000 0x000f0000	/* SDHC0_DAT3 */
		0x0c 0x00000010 0x000000f0	/* SDHC0_SDCD */
	>;
};
```

**修改完成后TF卡可以直接挂载使用，但是挂载之后显示系统只能以只读方式挂载。检查硬件原理图发现：**

**CON1B插座上的B13脚：SDHC0_SDWP悬空了，将该脚接地，则系统可以以读写方式挂载了。**

完成了这些工作之后，则可以实现通过修改U-boot环境变量，来直接从TF卡启动整个系统：

（1）将TF卡分成两个分区，1个为FAT32分区，存放内核文件和DTB文件；另一个为EXT4分区，存放rootfs根文件系统。

以8GB大小的TF卡为例：

mmcblk0p1是FAT32分区，大小500MB；

mmcblk0p2是EXT4分区，大小是7GB。

（2）需要修改DTB设备文件树，将chosen中的字段改为：

```bash
chosen {
	bootargs = "console=ttyS0,115200 init=/sbin/init root=/dev/mmcblk0p2 rw rootwait rootfstype=ext4 clk_ignore_unused loglevel=7";
};
```

重新编译DTB文件，作为新的加载DTB。

（2）按照第7节的描述，配置U-boot环境变量。然后启动系统。这样就可以直接从TF卡加载文件系统了。

3）还需要修改SDIO1的dtb pinctrl配置，否则会和I2C2和I2C3冲突：

```c
sdio1_pins: pinctrl_sdio1_pins {
	pinctrl-single,bits = <
		0x0c 0x00001000 0x0000f000	/* SDHC1_CLK */
		0x0c 0x00010000 0x000f0000	/* SDHC1_CMD */
		0x0c 0x00100000 0x00f00000	/* SDHC1_DAT0 */
		0x0c 0x01000000 0x0f000000	/* SDHC1_DAT1 */
		0x0c 0x10000000 0xf0000000	/* SDHC1_DAT2 */
		0x10 0x00000001 0x0000000f	/* SDHC1_DAT3 */
		0x10 0x00100000 0x00f00000	/* SDHC1_SDCD */
		0x10 0x01000000 0x0f000000	/* SDHC1_SDWP */
	>;
};
```

4）增加UART、I2C2、I2C3等配置：

完整的dtb文件链接：

[fx6evb_860mp_sanway.dts](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/0afe2671-5b35-4b97-adf1-6692c611244d/fx6evb_860mp_sanway.dts)

[fx6_860mp.dtsi](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/5cb010fe-f78a-463b-b790-489e4d99de32/fx6_860mp.dtsi)

**6、可以在U-boot下通过TF卡来更新各个分区的内容：**

经过验证实际发现，将TF卡插入到卡槽中，可以直接在U-boot下进行除rootfs之外各个分区的更新。TF卡上需要有一个fat32的分区，然后将文件复制到fat32分区上。以dtb更新为例：

```bash
mmc dev 0
fatls mmc 0
fatload mmc 0 0x08000000 fx6evb_860mp_sanway.dtb
sf probe 7:0
sf update 0x08000000 0x480000 0x80000
```

烧写内核映像：

```bash
mmc dev 0
fatls mmc 0
fatload mmc 0 0x08000000 uImage
sf probe 7:0
sf update 0x08000000 0x500000 0x900000
```

具体SPI Flash各个分区烧写地址可以参照这篇文档：

[FUXI-H2 开发板快速入门手册_V1.3.pdf](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/3cc04d10-1d7e-4b1b-addd-6344158b69b2/FUXI-H2_开发板快速入门手册_V1.3.pdf)

**7、经过再仔细研究，发现板子的根文件系统是在SDIO1上，对应设备mmcblk1p1，而TF卡是在SDIO0上，对应设备，mmcblk0，我们可以在U-boot下设计环境变量，使得系统直接从TF上的DTB和uImage启动：**

```bash
setenv dtb_file_name "fx6evb_860mp_sanway.dtb"
setenv kernel_file_name "uImage"
setenv mmc_tf_dev "0"
setenv mmc_load_kernel 'fatload mmc ${mmc_tf_dev} 0x08000000 ${kernel_file_name}; cp.b 0x08000000 ${load_kernel_addr} ${filesize}'
setenv mmc_load_dtb 'fatload mmc ${mmc_tf_dev} 0x08000000 ${dtb_file_name}; cp.b 0x08000000 ${load_fdt_addr} ${filesize}'
setenv boot_tf_card 'mmc dev ${mmc_tf_dev}; run mmc_load_kernel; run mmc_load_dtb; bootm ${load_kernel_addr}'
saveenv
```

**注意TF卡上的分区需要为fat32分区，里面的文件是dtb和内核映像。**

上面中的几个变量说明：

mmc_tf_dev：TF卡的mmc编号，为0；如果是板子上自带的eMMC，则编号为1。

load_kernel_addr：这个是bootm跳转时的内核地址，在0x80000000处。

load_fdt_addr：这个是内核加载dtb文件的内存地址，在0x81000000处。

mmc_load_kernel：将内核从TF卡上复制到内存中，此处需要注意，不能直接将内核映像复制到$load_kernel_addr处，如果这样操作，U-boot将会提示出错：

```c
** Reading file would overwrite reserved memory **
```

因此，需要先复制到低端内存地址：0x08000000处，然后再用cp.b命令从低端地址：0x08000000复制到高端内存地址：0x80000000处。

dtb的内存加载与上面相同，也不能直接复制到高端内存地址空间。

当内存复制完成后，就可以执行跳转指令bootm了。最后启动的命令就为：

```c
setenv boot_tf_card 'mmc dev ${mmc_tf_dev}; run mmc_load_kernel; run mmc_load_dtb; bootm ${load_kernel_addr}'
```

需要运行的时候，可以运行：

```c
run boot_tf_card
```

或者直接修改bootcmd，将其作为默认启动方式（注意此时TF卡上的FAT32分区中需要有一个叫boot_from_sd的文本文件）：

```bash
setenv bootcmd 'if test -e mmc 0:1 boot_from_sd; then run boot_tf_card; else run os_load; fi;'
```

**这样当插入SD卡的时候，如果SD卡上有DTB文件和uImage内核映像，且还有叫boot_from_sd的文件，则U-boot会切换到SD卡运行。**

u-boot.env我已经编译了u-boot下的工具fw_printenv，可以用来查看、设置和备份u-boot的环境变量。在u-boot的源代码中，可以看到环境变量的存储位置：

```bash
/****************************************
 *env config, refer to kernel partition
 ****************************************/
#define CONFIG_ENV_OFFSET	0x180000
#define CONFIG_ENV_SIZE		0x80000
#define CONFIG_ENV_SECT_SIZE	0x1000
```

可以修改fw_env.config文件，以使得可以直接在Linux下访问和操作env环境变量。**（待测试）**

我设想应该可以使用原来AM335x上的采用uEnv.txt来加载内核的方法，可以参照这篇文档：

[**orangepi制作sd卡启动镜像，并通过uenv加入动态更新uboot和内核功能**](https://www.notion.so/orangepi-sd-uenv-uboot-340457978b2246f0af317ad1a93609e6?pvs=21)

8、由从TF卡启动可以联想，如果将板子上自带的eMMC上独立分出一个fat32分区，并将内核映像和dtb文件放在里面，就可以直接从eMMC来加载Linux，调试也会更加方便，因为不需要再向QSPI Flash中烧写了。

可以使用sfdisk命令来对板子上的mmcblk1重新进行分区：

关于sfdisk使用的方法可以参照文档：

[**Linux sfdisk 命令**](https://www.notion.so/Linux-sfdisk-04b6dc82877845568746b824dea80cee?pvs=21)

另外系统中已经内置了fdisk命令，可以直接用fdisk来交互对系统进行分区。

9、系统安装、设置telnetd和vsftpd：

1）已经交叉编译了utelnetd和vsftpd，可以直接运行。运行telnetd：

```bash
utelnetd -d
```

2）将之前ARM系统上的vsftpd配置文件复制到/etc目录下，但是还需要做以下几个事情：

（1）在/usr/share下建立empty目录：

```bash
mkdir /usr/share/empty
```

（2）修改/etc/vsftpd.conf，添加以下内容：

```bash
pasv_enable=NO
#pasv_enable=YES
#pasv_min_port=3000  
#pasv_max_port=4000
port_enable=YES
connect_from_port_20=YES
allow_writeable_chroot=YES
anon_max_rate=0
local_max_rate=0
pasv_promiscuous=YES
seccomp_sandbox=NO
```

10、编译器需要注意的地方：

（1）程序中如果有浮点数计算等，则需要增加编译选项：

```bash
csky-abiv2-linux-gcc -mfdivdu -mhard-float -mdouble-float -march=ck860v -mcpu=ck860fv $1.c -o $1
```

## 2）基于SPI访问的FPGA程序已测试通过(FPGA: PGC4KD-6MBG324)：

10.1 已经调试完成了SPI访问FPGA片内的寄存器和FIFO：

为了便于高效的访问FIFO，我用了两路SPI来操作FPGA，1路用于读写FPGA片内的寄存器，另1路用于读写FPGA片内的FIFO。具体参见源代码：

[dtu_2660_fuxi.rar](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/7c369d13-a816-4e43-8576-65bc142aa05a/dtu_2660_fuxi.rar)

10.2 测试代码：

在应用层做了测试的代码，将SPI2和SPI3通过DTB配置为spidev1.0和spidev2.0，然后直接操作并读写（在实际使用之前，可能需要先重新烧写一下DTB）。具体见源代码：

[测试程序.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/c4553638-cb61-4acb-a298-a1e0457e6040/测试程序.tar.gz)

10.3 驱动代码：

采用低电平中断方式触发，驱动程序代码：

[cpld_dtu2660_fuxi_20220506.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/914bef81-d094-43e7-921c-cae10232b412/cpld_dtu2660_fuxi_20220506.tar.gz)

应用层测试代码：

[test_mmap.tar.gz](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/bafbed8c-37e3-40c1-8da0-81ca2235719a/test_mmap.tar.gz)

驱动代码使用说明：

（1）因为需要操作两个SPI总线，因此内核中做了2个驱动程序，分别为：

- dtu_2660_fuxi_spi_cmd.ko

- dtu_2660_fuxi_spi_fifo.ko

  使用时，需要先插入dtu_2660_fuxi_spi_cmd.ko模块，再插入dtu_2660_fuxi_spi_fifo.ko模块，否则将会出现内核崩溃信息。

（2）dtu_2660_fuxi_spi_fifo.ko模块插入时需要带参数：

```bash
insmod dtu_2660_fuxi_spi_fifo.ko adc_channels_number=16
```

上面的数字表示ADC采样的通道数量，需要根据你的ADC采样的实际通道来设置。如果不带参数，则默认的adc_channels_number为21。

（3）测试程序运行时也需要带参数，如下所示：

```bash
./test_dtu2660_fuxi 16 500 320
```

其中：

- 第一个参数：16需要跟dtu_2660_fuxi_spi_fifo.ko模块的adc_channels_number完全一致。
- 第二个参数：500表示采样频率，单位微秒。一般可以设置的值为：250,500,1000等
- 第三个参数：320表示设置采样多少个字节后产生一次中断。一般可以设置的值需要取第一个参数通道数量的整数倍。例如通道数量16，则320 = 16 * 2 * 10（每个通道是按字读写，换算成字节需要x2），表示采样10次产生一次中断。按照500us一次采样计算，则500 x 10 = 5（ms），也就是说，CPU每5ms将处理一次中断。
- 根据实际运行的状态，一般情况下，比较理想的取值为（其它取值会出现数据丢失或者帧数据不能对齐的情况）：

```bash
./test_dtu2660_fuxi 16 500 320 或者：./test_dtu2660_fuxi 16 250 320
```

（4）与之前的设计相比，FPGA的FIFO清零采用硬件复位FIFO的模式，对应ioctl的宏命令为：

```bash
#define IOREQ_ADC_FIFO_CLEAR_RESET	_IO(ESAM_MAJIC, 0xb4)
```

每次FIFO硬件复位完成后，第一次进中断用SPI读写FIFO时，会出现14个字节的0x00，而非FIFO中的实际数据，因此，在异步通知处理函数中，每次取FIFO数据时，需要增加一个偏移，示意代码如下：

```bash
//获取内核传递的ADC的状态结构值
	ioctl(g_fd, IOREQ_ADC_READ_NOTIFY, &adc_mem);
	INFO_DEBUG("curr_pos: %d, curr_sum: %d\\n", adc_mem.page_pos, adc_mem.page_sum);

	if (adc_mem.page_pos == 0) {
		INFO_NOTICE("MMAP address return 0!\\n");
	}

	tmp = adc_mem.page_pos - adc_mem.page_sum;
	if (tmp < 0) {
		tmp += adc_mem.page_sum;
	}

	tmp += 14;

	//for (i = 0; i < adc_mem.page_sum; i++) {
	for (i = 0; i < line_number; i++) {
		if ((i % line_number) == 0) {
			printf("\\n");
		}

		printf("%02x ", *(p_mmap_base + tmp + i));
	}

	printf("\\n\\n");
```

## 3）烧写文件及烧写方法：

1、首先需要制作一张TF启动卡，将TF卡分2个区，第1个区为fat32，第2个区为ext4，然后将下列烧写文件写入TF对应的分区中：

[fx6evb_860mp_sanway_tf.dtb](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/48571005-c52a-4a41-b08a-6bfefecc9dc7/fx6evb_860mp_sanway_tf.dtb)

将uImage内核映像以及TF卡的dtb文件复制到fat32分区中。

然后将rootfs文件直接解压到TF卡的第2个分区中。这样启动卡就可以使用了。

2、需要改写系统的u-boot env环境变量，可以按照前面描述的第7点直接在U-boot命令行下设置环境变量，也可以直接将已备份好的环境变量直接dd到对应的分区中：

用于TF卡启动的env备份：

[uboot_env_20220425.bak](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/772b6970-36a9-41de-8c59-d48f4dc70748/uboot_env_20220425.bak)

用于eMMC启动的env备份：

[uboot_env_bak_20220429](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/cb48a7a0-d66b-4307-84cb-ae6acae2b272/uboot_env_bak_20220429.txt)

```bash
dd if=uboot_env_bak_20220429 of=/dev/mtdblock5
```

然后将TF卡插入到卡槽中，重启系统，就可以启动了。

3、然后可以用文件系统中自带的fdisk或者sfdisk来对eMMC重新分区，和TF卡一样分成两个区，并且格式化对应分区，可以使用下面的busybox命令来格式化fat32分区：

[busybox](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/e94196b1-b59a-45de-9541-32968f378a3a/busybox.txt)

```bash
./busybox mkfs.vfat /dev/mmcblk1p1
```

之后和TF卡一样，将文件复制到对应分区即可。

## 4）编译器交叉编译说明

**交叉编译 Invalid configuration `xxxx': machine `xxxx' not recognized configure: error**

1.0 下载最新的配置文件：

```bash
[wget](<https://so.csdn.net/so/search?q=wget&spm=1001.2101.3001.7020>) -O config.guess '<http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD>'
wget -O config.sub '<http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD>'
```

（此条摘抄于 原文：https://blog.csdn.net/lile777/article/details/81389098 ）2.0 替换你要[交叉编译](https://so.csdn.net/so/search?q=交叉编译&spm=1001.2101.3001.7020)的工程路径下面的 config.guess 和 config.sub 有的时候不在根目录 使用find命令搜索即可找出 然后进行替换即可。 比如你要交叉编译libupnp 那么就在你下载的upnp的文件夹下面查找。

1.1 csky编译器使用注意：

如果是不带浮点运算的程序编译，需要如以下方式（增加CFLAGS=-mcpu=ck860）：

```bash
csky-abiv2-linux-gcc  -mcpu=ck860 test_app.c -o test_dtu2660_fuxi -lpthread -L .
```

如果是带浮点运算的程序编译，则需要如以下方式（需要增加CFLAGS和LDFLAGS都需要添加，即编译和链接时都需要添加）:

```bash
csky-abiv2-linux-gcc -mfdivdu -mhard-float -mdouble-float -march=ck860v -mcpu=ck860fv $1.c -o $1
```

## 5）关于MTD分区读写的方案：

参考文档：

[通过mtd读写flash_mtd_debug:linux下直接读写flash扇区 及 交叉编译mtd_utils | 学步园_weixin_39996134的博客-CSDN博客](https://blog.csdn.net/weixin_39996134/article/details/111746220?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~default-2-111746220-blog-122343836.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~default-2-111746220-blog-122343836.pc_relevant_default&utm_relevant_index=5)

[linux下spi-nor Flash的操作----备份与还原norflash中的uboot_阿卡基YUAN的博客-CSDN博客](https://blog.csdn.net/qingzhuyuxian/article/details/83476188?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-0-83476188-blog-89245991.topblog&spm=1001.2101.3001.4242.1&utm_relevant_index=3)

## 6）备份和恢复U-boot环境变量：

在U-boot下备份和回复环境变量：

```bash
setenv mmc_tf_dev 0
setenv env_file_name uboot_env.bak
setenv mmc_env_load  'fatload mmc ${mmc_tf_dev} 0x08000000 ${env_file_name}; sf probe 7:0; sf update 0x08000000 0x180000 0x8000'
setenv mmc_env_backup 'sf probe 7:0; sf read 0x08000000 0x180000 0x8000; fatwrite mmc ${mmc_tf_dev} 0x08000000 ${env_file_name} 0x8000'
```

其它说明：

```bash
setenv bootenv uEnv.txt
setenv loadbootenv 'fatload mmc ${mmcdev} ${fileaddr} ${bootenv}'
setenv importbootenv 'echo Importing environment from mmc ...; env import -t -r $loadaddr $filesize'
run importbootenv     //通过这几步将uEnv.txt里的内容导入到env里

'if test -n $uenvcmd; then echo Running uenvcmd ...; run uenvcmd;'
```