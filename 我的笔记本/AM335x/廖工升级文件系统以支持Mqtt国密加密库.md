# 廖工升级文件系统以支持Mqtt下国密加密库

为了支持`libssl.so.1.1`，我在docker下查了一下，需要ubuntu18.04的arm系统。但是ubuntu18.04的文件系统采用的是gcc7.5，所以需要升级我们的文件系统的GLIBC的库，以支持`libssl.so.1.1`。

测试平台：**SWM61850_MINI+_V4.0**

1、为了确保在原来的文件系统上升级GLIBC不会出错，我先在一个TF卡上做试验，将文件系统做在了TF卡上，然后修改U-boot下环境变量，增加以下两个环境变量：

```sh
set nandtfargs "setenv bootargs console=ttyO0,115200n8 root=/dev/mmcblk0p2 rw rootwait";

set nandtfboot "echo Booting from TF CARD ...; run nandtfargs; nand read ${fdtaddr} u-boot-spl-os; nand read ${loadaddr} kernel; bootz ${loadaddr} - ${fdtaddr}"
```

在U-boot环境下，运行：

```sh
run nandtfboot
```

让系统启动后加载TF卡上的文件系统，这样就不从nand Flash上启动了。等待TF的文件系统测试完成后，就可以直接在TF卡的文件系统环境下，去刷写nand Flash对应文件系统的mtd分区（本例中为`/dev/mtd6`），然后重启装置即可实现文件系统的升级更新。

2、在TF卡文件系统下刷写nand Flash的mtd分区：

```sh
flash_erase /dev/mtd6 0 0
nandwrite -p /dev/mtd6 ubi_gcc7.5_20241014.img
```

制作成ubifs映像的文件和原始rootfs文件目录在：

[gcc7.5文件系统](/home/jason/ubuntu13.04/arm/am335x/swm61850_mini+_190929/v4.0_20211108/rootfs_gcc_7.5/)