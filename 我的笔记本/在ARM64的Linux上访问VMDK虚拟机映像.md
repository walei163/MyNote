# 在ARM64的Linux系统上访问VMDK虚拟机映像的方法

文章目录：

[TOC]



## 一、第一步，将VMDK或者VDI格式的映像文件转换为原始映像：

### 1、可以采用`qemu_img`命令将VMDK文件转换为原始映像文件：

```bash
qemu-img convert -f vmdk -O raw debian12.vmdk debian12.img
```

参数说明：

- -f：输入的文件格式类型，可以是`vmdk、vdi、`等虚拟机映像文件。
- -O：输出文件的格式，如果是原始映像，一般为：`raw`，或者`qcow2`等格式。

上面的命令会将vmdk格式虚拟机映像转换为raw格式的img文件。

### 2、qemu-img还可以新建1个img原始镜像文件来使用：

```bash
[root@desktop example]# ls
[root@desktop example]# qemu-img create t1.img 1g    # 创建一个1G的磁盘文件
Formatting 't1.img', fmt=raw size=1073741824
[root@desktop example]# ls
t1.img
[root@desktop example]# qemu-img info t1.img         # 查看指定磁盘文件的信息
image: t1.img
file format: raw
virtual size: 1.0G (1073741824 bytes)
disk size: 0
[root@desktop example]#
```

对于新建的镜像文件，后续就可以通过`losetup`来映射到`/dev/loopx`设备下，并通过`gparted`软件对其进行分区、格式化等操作。

### 3、qemu-img还可以对img文件进行resize操作：

**1）扩大尺寸：**

```bash
qemu-img resize t1.img +5G
```

**2）缩减尺寸：**

```bash
qemu-img resize --shrink -f raw t1.img -5G
```



## 二、挂载并修改img映像分区文件：

### 1、Linux命令`losetup`

`losetup`命令用于设置循环设备。

循环设备可把文件虚拟成区块设备，籍以模拟整个文件系统，让用户得以将其视为硬盘驱动器，光驱或软驱等设备，并挂入当作目录来使用。

```bash
$ losetup -help

Usage:
 losetup [options] [<loopdev>]
 losetup [options] -f | <loopdev> <file>

Set up and control loop devices.

Options:
 -a, --all                     list all used devices
 -d, --detach <loopdev>...     detach one or more devices
 -f, --find                    find first unused device
 -P, --partscan                create a partitioned loop device
...

```

关注的参数有

- -d： 移除已经挂载的loop设备
- -f： 查找下一个可用的loop设备
- -p：加载带分区loop设备

当挂载含分区img文件时，需要添加-p参数。

下面演示如何losetup、mount来挂载img文件。

### 2、挂载img系统镜像

#### 2.1. 树莓派备份系统镜像

以树莓派为例备份一个系统镜像img文件，使用 `fdisk -l filename.img` 查看信息如下：

```bash
$ fdisk -l raspios_copy_cm4.img
Disk raspios_copy_cm4.img: 7.3 GiB, 7818182656 bytes, 15269888 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0xadf6c25e

Device                Boot  Start      End  Sectors  Size Id Type
raspios_copy_cm4.img1        8192   532479   524288  256M  c W95 FAT32 (LBA)
raspios_copy_cm4.img2      532480 15269887 14737408    7G 83 Linux

```

整个镜像占用7.3G，未进行系统压缩（系统压缩参 [树莓派系统最小img镜像的制作-备份与还原](https://wanggao1990.blog.csdn.net/article/details/121420289)），含有两个分区，一个是固定的 256M 的boot启动分区，另一个是约7G的rootfs文件系统。

#### 2.2、挂载分区

**（1）查看下一个可用的loop设备**

使用命令`losetup -f` 查询到的下一个可用loop设备文件为 /dev/loop19。

```bash
$ losetup -f
/dev/loop19
```

**（2）使用 losetup -p 参数挂载img文件**

```bash
$ sudo losetup -P /dev/loop19 raspios_copy_cm4.img

$ sudo lsblk
NAME       MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
loop0        7:0    0  32.3M  1 loop /snap/snapd/12883
... // 省略
loop18       7:18   0   2.5M  1 loop /snap/gnome-system-monitor/174
loop19       7:19   0   7.3G  0 loop
├─loop19p1 259:0    0   256M  0 loop
└─loop19p2 259:1    0     7G  0 loop
sda          8:0    0 931.5G  0 disk
├─sda1       8:1    0   512M  0 part /boot/efi
├─sda2       8:2    0 930.1G  0 part /
└─sda3       8:3    0   977M  0 part
```

可以看到设备 loop19下有两个分区加载，大小和树莓派系统镜像一致，loop19p1是启动分区boot，loop19p2是文件系统分区rootfs。

当需要卸载img镜像文件时，使用命令 `losetup -d` ，例如 losetup -d /dev/loop19。

**（3）挂载文件系统**

创建两个文件夹 rootfs 和 boot，并分别使用`mount`命令挂载loop19p1和loop19p2。

```bash
$ mkdir boot
$ mkdir rootfs

$ sudo mount /dev/loop19p1 ./boot/
$ sudo mount /dev/loop19p2 ./rootfs/
```

**（4）查看和修改分区文件**

分别进入 rootfs 和 boot，就是在树莓派启动后看到的文件内容。

```bash
$ ls ./boot/
 COPYING.linux                bcm2709-rpi-2-b.dtb        cmdline.txt    fixup_db.dat   start4.elf
 LICENCE.broadcom             bcm2710-rpi-2-b.dtb        config.txt     fixup_x.dat    start4cd.elf
'System Volume Information'   bcm2710-rpi-3-b-plus.dtb   dt-blob.bin    issue.txt      start4db.elf
 bcm2708-rpi-b-plus.dtb       bcm2710-rpi-3-b.dtb        fixup.dat      kernel.img     start4x.elf
 bcm2708-rpi-b-rev1.dtb       bcm2710-rpi-cm3.dtb        fixup4.dat     kernel7.img    start_cd.elf
 bcm2708-rpi-b.dtb            bcm2711-rpi-4-b.dtb        fixup4cd.dat   kernel7l.img   start_db.elf
 bcm2708-rpi-cm.dtb           bcm2711-rpi-400.dtb        fixup4db.dat   kernel8.img    start_x.elf
 bcm2708-rpi-zero-w.dtb       bcm2711-rpi-cm4.dtb        fixup4x.dat    overlays
 bcm2708-rpi-zero.dtb         bootcode.bin               fixup_cd.dat   start.elf

$ ls ./rootfs/
bin   dev  home  lost+found  mnt  proc  run   srv  tmp      usr
boot  etc  lib   media       opt  root  sbin  sys  upgrade  var
```

之后，我们可以像进入了一个启动的树莓派系统一样进行文件的增删改了，但是要注意路径的操作。

#### 2.3、卸载退出

```bash
$ sudo umount /dev/loop19p
$ sudo umount /dev/loop19p2
$ sudo losetup -d /dev/loop19
```

> [!note]
>
> <font face="微软雅黑" color=cyan>**采用`losetup`是当虚拟img文件中包含多个分区时使用 。**</font>
>
> 若img文件只有一个分区，则可以直接采用：
>
> ```bash
> mount -o loop xxx.img /mnt/temp
> ```
>
> 

