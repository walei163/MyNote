stm32mp157c-dk2 u-boot启动与制作
原创 守望者910 最后发布于2019-11-03 23:55:36 阅读数 120 收藏
展开

一.uboot描述

1）支持外设

1. UART
2. SDCard/MMC controller (SDMMC)
3. NAND controller (FMC)
4. NOR controller (QSPI)
5. USB controller (OTG DWC2)
2）支持驱动

1. I2C
2. STPMIC1 (PMIC and regulator)
3. Clock, Reset, Sysreset
4. Fuse
二.启动列表

BootRom => FSBL in SYSRAM => SSBL in DDR => OS (Linux Kernel)

FSBL = 第一阶段 Bootloader
SSBL = 第二阶段Bootloader

1）boot设置

a.安全模式启动

    在这种模式配置下, U-Boot 和安全固件相关联 (TF-A)而且仅仅U-Boot镜像作为第二阶段的bootloader （SSBL）。TF-A二进制文件(tf-a-*.stm32)必须被拷贝到名为“fsbl1”的指定分区。U-boot 二进制文件 (u-boot*.stm32)必须被拷贝到名为“ssbl”的指定分区
    安全启动模式: U-Boot ".stm32" 的二进制扩展文件是SSBL，FSBL有TF-A提供
    配置文件：defconfig_file : stm32mp15_trusted_defconfig
    上电启动：BootRom加载启动第一启动文件 => FSBL = Trusted Firmware-A (TF-A)   第一阶段启动文件；

 接着加载启动第二阶段启动文件=> SSBL = U-Boot  第二阶段文件uboot。

TF-A提供安全外设的全部初始化（如时钟初始化，即使第二阶段也有时钟初始化，但是还是以FSBL初始化为主），并且建立安全监控，uboot运行在正常模式，并且在TF-A安全监控模式下访问安全资源

b.一般模式启动

    编译uboot提供给两个启动阶段所需要的二进制文件：一般启动模式: U-Boot SPL为第一阶段启动文件（FSBL,.stm32格式），即U-Boot SPL (u-boot-spl.stm32-*) 需要拷贝到名字为“fsbl1”的指定分区。U-Boot imgage为第二阶段启动文件（SSBL，U-Boot imgage ），即U-Boot 镜像 (u-boot*.img)必须拷贝到名为“ssbl” 的指定分区。
    一般模式启动:在这个设置中,我们用 U-Boot SPL 作为第一阶段的加载启动(FSBL) ，用U-Boot镜像作为第二阶段的加载启动(SSBL).
    配置文件：defconfig_file :stm32mp15_basic_defconfig
    上电启动：BootRom加载启动第启动文件 => FSBL = u-boot-spl.stm32-*    第一阶段启动文件uboot  SPL

接着启动第二启动项文件=> SSBL =u-boot*.img  第二阶段启动文件

SPL禁止安全模式初始化，uboot运行在安全模式，仅仅通过PSCI（arm定义的电源状态协调接口）支持给内核提供安全监控

uboot支持所有的STM32MP1板子，可以支持引导所有stm32mp1一般的的板子。

每个板子仅仅通过设备树进行配置：

 ev1: eval board with pmic stpmic1 (ev1 = mother board + daughter ed1)
  dts: stm32mp157c-ev1

+ ed1: daughter board with pmic stpmic1
  dts: stm32mp157c-ed1

+ dk1: Discovery board
  dts: stm32mp157a-dk1

+ dk2: Discovery board = dk1 with a BT/WiFI combo and a DSI panel
  dts: stm32mp157c-dk2



三.uboot编译步骤

==================

1）. 安装U-Boot Makefile所需要的工具

     (libssl-dev, swig, libpython-dev...)
   安装ARMv7的32位工具链+ install ARMv7 toolchain for 32bit Cortex-A ( Linaro,
   SDK for STM32MP1, 或者从你的配置安装一些交叉工具链)

2）. 设置编译器，这个也可以就行系统设置/etc/bash.bashrc里面长期设置:

     # export CROSS_COMPILE=/path/to/toolchain/arm-linux-gnueabi-
    编译器已经安装，我电脑执行：source /home/my-linux/STM32MPU_workspace/STM32MP15-Ecosystem-v1.1.0/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-openstlinux_weston-linux-gnueabi

3）. 选择编译输出文件目录

        # export KBUILD_OUTPUT=/path/to/output

        例如:给每一种模式配置新建一个输出目录
        # export KBUILD_OUTPUT=../build/stm32mp15_trusted
        # export KBUILD_OUTPUT=../build/stm32mp15_basic

4）. 配置U-Boot:

        # make  <defconfig_file>

        - 对于安全可靠模式defconfig_file : "stm32mp15_trusted_defconfig"
        - 对于一般模式defconfig_file: "stm32mp15_basic_defconfig"

5）. 配置设备树 和编译 U-Boot image:

        # make DEVICE_TREE=<name> all

 例如:
  a) 安全模式 ev1板子
        # export KBUILD_OUTPUT=stm32mp15_trusted
        # make stm32mp15_trusted_defconfig
        # make DEVICE_TREE=stm32mp157c-ev1 all

  b) 一般模式dk2板子
        # export KBUILD_OUTPUT=stm32mp15_basic
        # make stm32mp15_basic_defconfig
        # make DEVICE_TREE=stm32mp157c-dk2 all

 

四. 输出文件

  BootRom 和 TF-A 所需要的STM32镜像头文件的二进制文件
  SPL所需要的 U-Boot uImage头部文件

 在输出目录 (KBUILD_OUTPUT),
  可以找到所需要的文件:

  a)在安全模式下
   + FSBL = tf-a.stm32 (TF-A源码编译提供)
   + SSBL = u-boot.stm32（uboot编译得到）

  b) 一般模式下
   + FSBL = spl/u-boot-spl.stm32（uboot一般模式编译得到）
   + SSBL = u-boot.img（uboot一般模式编译得到）


五.制作microSD启动镜像

STM32启动最小需要的分区文件:
- GPT分区 (用 gdisk 或者 sgdisk)
- 2 个fsbl分区, 名字 fsbl1 和 fsbl2, 大小至少是256KiB
- 1个ssbl 分区 给U-Boot

然后最小GPT 分区是:
   ----- ------- --------- ---------------
  | 数量 | 名字 | 大小    |  内容     |
   ----- ------- -------- ----------------
  |  1  | fsbl1 | 256 KiB |  TF-A or SPL |
  |  2  | fsbl2 | 256 KiB |  TF-A or SPL |
  |  3  | ssbl  | enought |  U-Boot      |
  |  *  |  -    |  -      |  Boot/Rootfs |
   ----- ------- --------- ---------------

在 extlinux.conf添加引导分区/uboot/include/config_distro_bootcmd.h配置着启动参数，u-boot-2018.11/board/st/stm32mp1/README，有uboot制作sd启动详细描述

   下面是基本的分配情况
    (doc/README.distro for use)

  根据用到的card选择设备
  (/dev/sdx or /dev/mmcblk0)
我用 /dev/mmcblk0作为例子

例如: with gpt table with 128 entries

  a) 删掉先前的格式（制作启动盘需要管理模式）
        # sgdisk -o /dev/<SDCard dev>

  b) 创建最新系统镜像
        # sgdisk --resize-table=128 -a 1 \
                -n 1:34:545             -c 1:fsbl1 \
                -n 2:546:1057           -c 2:fsbl2 \
                -n 3:1058:5153          -c 3:ssbl \
                -p /dev/<SDCard dev>

例如制作步骤：

1）# sgdisk -o /dev/sdc  删除分区，在管理模式下

2）root@vlinux:/dev#

sgdisk --resize-table=128 -a 1 -n 1:34:545 -c 1:fsbl1   -p  /dev/sdc
3）sgdisk --resize-table=128 -a 1 -n 2:546:1057 -c 2:fsbl2  -p  /dev/sdc
4）sgdisk --resize-table=128 -a 1 -n 3:1058:5153 -c 3:ssbl -p /dev/sdc
5）sgdisk --resize-table=128 -a 1 -n 4:5154:136225 -c 4:bootfs -p /dev/sdc
6）sgdisk --resize-table=128 -a 1 -n 5:136226:168993 -c 5:vendorfs -p /dev/sdc
7）sgdisk --resize-table=128 -a 1 -n 6:168994:1705857 -c 6:rootfs -p /dev/sdc
8）sgdisk --resize-table=128 -a 1 -n 7:1705858:15759324 -c 7:userfs -p /dev/sdc
分区完后,分别对4.5.6.7分区格式化成ext4格式

mkfs -t ext4 /dev/sdc4

mkfs -t ext4 /dev/sdc5

mkfs -t ext4 /dev/sdc6

mkfs -t ext4 /dev/sdc7

Number  Start (sector)    End (sector)  Size       Code  Name
   1              34             545   256.0 KiB   8300  fsbl1
   2             546            1057   256.0 KiB   8300  fsbl2
   3            1058            5153   2.0 MiB     8300  fsbl3
   4            5154          136225   64.0 MiB    8300  fsbl4
   5          136226          168993   16.0 MiB    8300  fsbl5
   6          168994         1705857   750.4 MiB   8300  fsbl6
   7         1705858        15759324   6.7 GiB     8300  fsbl7
The operation has completed successfully.
pc挂载可用ls -l /dev/disk/by-partlabel/查看分区

 也可以继续添加内核分区
        例如rootfs 分区:
                -n 3:5154:              -c 4:rootfs \

c) 拷贝FSBL (拷贝2 次) 和SSBL文件到指定的分区.
     例如分区 1 to 3

     一般模式 : <SDCard dev> = /dev/mmcblk0
        # dd if=u-boot-spl.stm32 of=/dev/mmcblk0p1
        # dd if=u-boot-spl.stm32 of=/dev/mmcblk0p2
        # dd if=u-boot.img of=/dev/mmcblk0p3

     安全模式 :
        # dd if=tf-a.stm32 of=/dev/mmcblk0p1
        # dd if=tf-a.stm32 of=/dev/mmcblk0p2
        # dd if=u-boot.stm32 of=/dev/mmcblk0p3
  SDCard启动, 选择BootPinMode = 1 1 1 然后复位.       

 

 六.eMMC启动
===============
用U-Boot拷贝二进制文件到 eMMC.

在接下来的例子中, 你需要先从 SDCARD 启动，镜像文件(u-boot-spl.stm32, u-boot.img)是ext4格式
目前在 SDCARD (mmc 0)  第四分区(bootfs)

 SDCard启动, 选择BootPinMode = 1 1 1 然后复位.

然后更新eMMC在接下来的 U-Boot 命令中 :

a) 在eMMC准备全局引导GPT,
        例子中的2个分区, bootfs 和roots:

        # setenv emmc_part  "name=ssbl, size=2MiB; name=bootfs, type=linux, bootable, size=64MiB; name=rootfs, type=linux, size=512"
        # gpt write mmc 1 ${emmc_part}

b)复制SPL 到eMMC 的第一个启动分区
        (SPL最大是 256kB, with LBA 512, 0x200)

        # ext4load mmc 0:4 0xC0000000 u-boot-spl.stm32
        # mmc dev 1
        # mmc partconf 1 1 1 1
        # mmc write ${fileaddr} 0 200
        # mmc partconf 1 1 1 0

b) 复制 U-Boot 在eMMC的第一个GPT 分区

        # ext4load mmc 0:4 0xC0000000 u-boot.img
        # mmc dev 1
        # part start mmc 1 1 partstart
        # part size mmc 1 1 partsize
        # mmc write ${fileaddr} ${partstart} ${partsize}

从eMMC启动, 选择 BootPinMode = 0 1 0 然后复位重启.

 

 七. MAC 地址

Mac id 的存取和索引再stm32mp otp上 :
- OTP_57[31:0] = MAC_ADDR[31:0]
- OTP_58[15:0] = MAC_ADDR[47:32]

编程一个MAC 地址在 上面原始的OTP上 , 可以在bank0上用fuse 命令
进入内部的 OTP:

  例如去设置MAC地址"12:34:56:78:9a:bc"

    1） 首先写 OTP
       STM32MP> fuse prog -y 0 57 0x78563412 0x0000bc9a

    2）然后读OTP
       STM32MP> fuse sense 0 57 2
       bank 0上显示:
       Word 0x00000039: 78563412 0000bc9a

    3）接着重新启动 REBOOT :
       ### Setting environment from OTP MAC address = "12:34:56:78:9a:bc"

    4 ）检查环境更新
       STM32MP> print ethaddr
       ethaddr=12:34:56:78:9a:bc

 

八. 协处理器固件
========================

U-Boot可以在内核之前启动协处理器 (协处理器早期启动).

1）用rproc命令手动启动  (更新bootcmd)
配置：
        # env set name_copro "stm32mp15_m4.elf"
        # env set dev_copro 0
        # env set loadaddr_copro 0xC1000000

    在SDCard（mmc 0）第四分区，从bootfs分区下载二进制文件 
        # ext4load mmc 0:4 ${loadaddr_copro} ${name_copro}
        => ${filesize} updated with the size of the loaded file

  用远程proc命令 启动M4固件 
        # rproc init
        # rproc load ${dev_copro} ${loadaddr_copro} ${filesize}
        # rproc load_rsc ${dev_copro} ${loadaddr_copro} ${filesize}
        # rproc start ${dev_copro}

2）用FIT特性和通用的DISTRO bootcmd自动加载//Automatically by using FIT feature and generic DISTRO bootcmd

  看这个目录的例子:

   生成 FIT， 包括kernel + device tree + M4 固件
  用 cfg 用 M4 boot
        $> mkimage -f fit_copro_kernel_dtb.its fit_copro_kernel_dtb.itb

   然后用DISTRO 配置文件:参考extlinux.conf 选择
正确的配置如下：
        => stm32mp157c-ev1-m4
        => stm32mp157c-dk2-m4
                                  

 

 九.通过SDCARD更新uboot启动：
-----------------------

1）基本启动模式
需要更新的文件：* u-boot-spl.stm32-*
  把二进制文件拷贝到指定分区, 在SDCARD/USB闪存分区
  "fsbl1" 是分区 1:
  - SDCARD: /dev/mmcblkXp1 (where X is the instance number)
  - SDCARD via USB reader: /dev/sdX1 (where X is the instance number)
  dd if=<U-Boot SPL file> of=/dev/<device partition> bs=1M conv=fdatasync

需要更新的文件* u-boot*.img
拷贝二进制文件到指定分区, 在SDCARD/USB 的闪存 分区
  "ssbl" 是分区 4:
  - SDCARD: /dev/mmcblkXp3 (x是分区的实例标号)

2）安全可靠启动模式
需要更新的文件：* tf-a-*.stm32
  拷贝二进制到指定的分区,在SDCARD/USB 闪存的分区
  "fsbl1" 是分区1:
  - SDCARD: /dev/mmcblkXp1 (是实际分区名字)
  - SDCARD via USB reader: /dev/sdX1 （x是实际分区的名字)
  dd if=<TF-A binary file> of=/dev/<device partition> bs=1M conv=fdatasync

需要更新的二进制文件：* u-boot*.stm32
  拷贝二进制到指定的分区,在SDCARD/USB 闪存的分区
  "ssbl" 是分区 4:
  - SDCARD: /dev/mmcblkXp3 (是实际分区名字)
  - SDCARD via USB reader: /dev/sdX3 (是实际分区名字)
  dd if=<U-Boot stm32 binary file> of=/dev/<device partition> bs=1M conv=fdatasync
