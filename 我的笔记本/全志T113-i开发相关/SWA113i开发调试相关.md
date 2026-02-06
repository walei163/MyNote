# SWA113i开发调试相关

## 2025-12-11:

今天算是搞清楚了T113i加载DTB的方式，代码在：[/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/brandy/brandy-2.0/u-boot-2018/board/sunxi/sunxi_replace_fdt.c](/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/brandy/brandy-2.0/u-boot-2018/board/sunxi/sunxi_replace_fdt.c)：

```c
int sunxi_get_dtb(ulong *dtb_data, ulong *dtb_len)
{
	int work_mode = get_boot_work_mode();
	if (work_mode == WORK_MODE_BOOT) {
		u32 part_start;
		char env_boot_normal[8][16];
		sunxi_parsed_specific_string(env_get("boot_normal"), env_boot_normal, ' ', 0);
		u32 boot_head_addr = (u32)simple_strtoul(env_boot_normal[2], NULL, 16);

		part_start = sunxi_partition_get_offset_byname("dtb");
		if (part_start != 0) {
			*dtb_data = CONFIG_SUNXI_FDT_ADDR;
			sunxi_flash_read(part_start, ALIGN(sizeof(struct fdt_header), 512)/512, (void *)(ulong)(*dtb_data));
			*dtb_len = fdt_totalsize((char *)(*dtb_data));
			sunxi_flash_read(part_start, ALIGN(*dtb_len, 512)/512, (char *)(*dtb_data));
		}
		if (fdt_check_header((void *)*dtb_data) < 0) {
			char *boot_name = env_get("boot_partition");
			if (boot_name == NULL) {
				part_start = sunxi_partition_get_offset_byname("boot");
			} else {
				part_start = sunxi_partition_get_offset_byname(boot_name);
			}
			sunxi_flash_read(part_start, ALIGN(sizeof(struct andr_img_hdr), 512)/512, (void *)(ulong)boot_head_addr);
#ifdef CONFIG_ANDROID_BOOT_IMAGE
			android_image_get_dtb((const struct andr_img_hdr *)(ulong)boot_head_addr, dtb_data, dtb_len);
#endif
		}
	}
	return 0;
}
```

要点如下：

> [!important]
>
> **1、从emmc分区寻找名为“dtb”的分区，如果找到，则读取该分区的dtb信息；**
>
> **2、如果没有该分区，或者该分区内没有dtb信息，则进入分支判断：if (fdt_check_header((void *)*dtb_data) < 0) ，并且执行：**
>
> ```c
> #ifdef CONFIG_ANDROID_BOOT_IMAGE
> 			android_image_get_dtb((const struct andr_img_hdr *)(ulong)boot_head_addr, dtb_data, dtb_len);
> #endif
> ```
>
> 也就是从boot.img中获取到dtb信息，然后加载。
>
> 根据这个代码的分析，我们可以实现内核映像和dtb的分离加载：



**1、官方默认的分区为：**

```sh
# sgdisk -p /dev/mmcblk1
Disk /dev/mmcblk1: 15126528 sectors, 7.2 GiB
Sector size (logical/physical): 512/512 bytes
Disk identifier (GUID): AB6F3888-569A-4926-9668-80941DCB40BC
Partition table holds up to 8 entries
Main partition table begins at sector 73726 and ends at sector 73727
First usable sector is 73728, last usable sector is 15126524
Partitions will be aligned on 2-sector boundaries
Total free space is 31 sectors (15.5 KiB)

Number  Start (sector)    End (sector)  Size       Code  Name
   1           73728          108165   16.8 MiB    0700  boot-resource
   2          108166          110213   1024.0 KiB  0700  env
   3          110214          112261   1024.0 KiB  0700  env-redund
   4          112262          147461   17.2 MiB    0700  boot
   5          147462         2244613   1024.0 MiB  0700  rootfs
   6         2244614         2246661   1024.0 KiB  0700  dsp0
   7         2246662         2279429   16.0 MiB    0700  private
   8         2279430        15126493   6.1 GiB     0700  UDISK
# 
```



**2、设置一个分区，将其名称改为：“dtb”：**

```sh
sgdisk -c=6:"dtb" /dev/mmcblk0
```

> [!note]
>
> 即将mmcblk0的第6个分区，原来叫做：“dsp0”的分区改为：“dtb”分区。



**3、新的分区信息如下：**

```sh
# sgdisk -p /dev/mmcblk0
Disk /dev/mmcblk0: 7733248 sectors, 3.7 GiB
Sector size (logical/physical): 512/512 bytes
Disk identifier (GUID): AB6F3888-569A-4926-9668-80941DCB40BC
Partition table holds up to 8 entries
Main partition table begins at sector 2 and ends at sector 3
First usable sector is 73728, last usable sector is 7733214
Partitions will be aligned on 2-sector boundaries
Total free space is 0 sectors (0 bytes)

Number  Start (sector)    End (sector)  Size       Code  Name
   1           73728          108165   16.8 MiB    0700  boot-resource
   2          108166          110213   1024.0 KiB  0700  env
   3          110214          112261   1024.0 KiB  0700  env-redund
   4          112262          147461   17.2 MiB    0700  boot
   5          147462         2244613   1024.0 MiB  0700  rootfs
   6         2244614         2246661   1024.0 KiB  0700  dtb
   7         2246662         2279429   16.0 MiB    0700  private
   8         2279430         7733214   2.6 GiB     0700  UDISK
```



**4、将设备树dtb文件烧写进这个分区中：**

```sh
dd if=dca173_v1.0.dtbo of=/dev/mmcblk0p6
```



**5、设置u-boot环境变量如下：**

```sh
sanway> setenv kernel_load_addr 0x40008000
sanway> setenv boot_cmd_sanway 'run setargs_mmc; sunxi_flash read ${kernel_load_addr} boot; bootz ${kernel_load_addr}'
sanway> setenv bootcmd 'run boot_cmd_sanway'
sanway> saveenv
```

> [!note]
>
> **2025-12-14：最新研究发现，采用bootz启动后，在对eMMC进行大块写操作时，内核会出现崩溃信息。必须选择采用bootm启动方式才行。因此，u-boot下的启动还要该回到原来的bootcmd：**
>
> ```sh
> setenv bootcmd 'run setargs_mmc boot_normal'
> ```
>
> **DTB仍旧可以采用上面的描述，将其烧写到dtb（/dev/mmcblock0p6）分区中，此时用bootm加载内核，但是设备树却是从dtb分区加载的。这样也可以起到内核和设备树分离的作用。**

完成，可以重启系统验证一下。

---

## 2025-12-08：

在T113i下也可以实现加载dtbo模式，需要设置的uboot环境变量如下：

```sh
setenv fdtaddr 43000000
setenv fdt_load_addr 44000000
setenv dtbo_file '/boot/dtb/dts-overlay-1.dtbo'
setenv mmc_dtb_load 'mmc dev 2; mmc part; ext4load mmc 2:5 ${fdt_load_addr} ${dtbo_file}; fdt apply ${fdt_load_addr}' 
setenv bootcmd 'run mmc_dtb_load; run setargs_mmc boot_normal'
```

加载流程与T536模式一样，uboot启动时：

1. 将dtbo文件/boot/dtb/dts-overlay-1.dtbo读取到内存的0x44000000处，然后和基础的dtb通过“fdt apply”命令合并；
2. 接着执行默认的bootcmd流程。
3. 可以用我之前做的update_linux.sh脚本来更新dtbo文件。
4. 默认的buildroot下的env.cfg文件已经更新：[/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/device/config/chips/t113_i/configs/evb1_auto/buildroot/env.cfg](/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/device/config/chips/t113_i/configs/evb1_auto/buildroot/env.cfg)

```c
#uboot system env config
bootdelay=3
bootstopkey=q
fdtaddr=43000000
fdt_load_addr=44000000
dtbo_file=/boot/dtb/dts-overlay-1.dtbo
mmc_dtb_load=mmc dev 2; mmc part; ext4load mmc 2:5 ${fdt_load_addr} ${dtbo_file}; fdt apply ${fdt_load_addr}

#default bootcmd, will change at runtime according to key press
#default nand boot
bootcmd=run mmc_dtb_load; run setargs_mmc boot_normal
```



---

## 2025-11-29：

> [!note]
>
> 2025-12-04：
>
> <font color=red>**该问题已经找到，需要在编译内核时，将`Device Tree overlays` 编译进内核，才能支持dtbo模式。**</font>
>
> 具体内核配置：
>
> ```sh
> Device Drivers  --->
> 	-*- Device Tree and Open Firmware support  --->
> 		--- Device Tree and Open Firmware support
> 		[ ]   Device Tree runtime unit tests 
> 		[*]   Device Tree overlays 
> ```
>
> 重新编译内核之后，在u-boot下就可以按照T536的模式，来加载dtbo运行了。

---

- 为了兼容以前的设计，决定还是将内核映像烧录到mmc的`boot`分区，只是该分区烧进去的不是`boot.img`，而是内核的`zImage`映像。

- 将dtb放置到文件系统的/boot分区下，文件名：`sunxi.dtb`。在u-boot启动时，会将该文件加载至内存对应的地址处。
- 最后设置u-boot环境变量：
  - 设置`bootargs`变量。
  - 引导采用`bootz`命令。

完整的u-boot环境变量设置如下：

```sh
setenv dtb_file_name sunxi_dtb
setenv mmc_dev_no 2
setenv mmc_dev_part 5
setenv kernel_load_addr 0x40008000
setenv fdt_load_addr 0x43000000
setenv mmc_dtb_load 'mmc dev ${mmc_dev_no}; mmc part; ext4load mmc ${mmc_dev_no}:${mmc_dev_part} ${fdt_load_addr} /boot/${dtb_file_name}'
#setenv mmc_kernel_image_load 'ext4load mmc 2:5 ${kernel_load_addr} /boot/zImage'

setenv mmc_kernel_image_load 'sunxi_flash read ${kernel_load_addr} boot'

setenv boot_cmd_sanway 'run setargs_mmc; run mmc_dtb_load; run mmc_kernel_image_load; bootz ${kernel_load_addr} - ${fdt_load_addr}'
```

按照上面的设计，就实现了与官方不同的，将内核映像与dtb设备文件树分离加载。

> [!tip]
>
> **<font color=red>这样做的目的，是为了后续维护时，多数情况下，尽量不需要重新编译内核，而只需要修改并重新编译dtb即可。</font>**

---

## 2025-11-27：

### 将内核启动映像zImage和设备树分离的方法：

#### 1、可以用binwalk命令来查看boot.img这个文件的信息：

```sh
$ binwalk boot.img

DECIMAL       HEXADECIMAL     DESCRIPTION
--------------------------------------------------------------------------------
0             0x0             Android bootimg, kernel size: 5653400 bytes, kernel addr: 0x40008000, ramdisk size: 0 bytes, ramdisk addr: 0x41000000, product name: "sun8i_arm"
2048          0x800           Linux kernel ARM boot executable zImage (little-endian)
18056         0x4688          gzip compressed data, maximum compression, from Unix, last modified: 1970-01-01 00:00:00 (null date)
5656576       0x565000        Flattened device tree, size: 59604 bytes, version: 17
```

可以知道，设备树被拼接到内核的zImage映像的后面以单独文件存在。下面是copilot的解释：

```sh
针对 Allwinner / sunxi 的备注

Allwinner 平台往往把 DTB 拼接在 kernel/zImage 后面或以单独文件存在（例如 script.bin 或 sunxi 特有布局）。
如果上述方法找不到魔数，可能 DTB 被压缩或采用了不同容器（例如 fitImage、U-Boot FIT）。
此时：试用 binwalk -e（会尝试解压）
用 dumpimage -l / dumpimage -T（u-boot-tools 的 dumpimage）查看是否是 FIT：dumpimage -l boot.img 或 dumpimage -T boot.img
若是 FIT：dumpimage -i boot.img -p 0 -o extracted.dtb（视具体 FIT 布局而定）
```

然后可以用以下命令将其提取出来：

```sh
$ dd if=boot.img of=extracted.dtb bs=1 skip=5656576 count=59604 status=progress
输入了 59604+0 块记录
输出了 59604+0 块记录
59604 字节 (60 kB, 58 KiB) 已复制，0.276914 s，215 kB/s
```

#### 2、分离映像和设备树的设计：

根据对boot.img的分析，我们就可以采用在u-boot下分别启动zImage和dtb，这样就将设备树从内核中分离出来了。为此，我设计了u-boot下的环境变量：

```sh
setenv kernel_load_addr 0x40008000
setenv fdt_load_addr 0x43000000
setenv mmc_dtb_load 'mmc dev 2; mmc part; ext4load mmc 2:5 ${fdt_load_addr} /boot/sunxi.dtb'
setenv mmc_kernel_image_load 'ext4load mmc 2:5 ${kernel_load_addr} /boot/zImage'

setenv boot_cmd_sanway 'run setargs_mmc; run mmc_dtb_load; run mmc_kernel_image_load; bootz ${kernel_load_addr} - ${fdt_load_addr}'
```



备份原来官方的启动bootcmd：

```sh
setenv bootcmd 'run setargs_mmc boot_normal'
```

最后修改新的bootcmd：

```sh
setenv bootcmd 'run boot_cmd_sanway'
```

经过这样修改后，就可以从文件系统的/boot目录下直接加载两个文件：

```sh
zImage
sunxi.dtb
```

然后就可以直接启动了。

> [!note]
>
> **<font color=red>经过实际测试，这种情况下，需要将分区/mmcblk0p4（boot）清空，否则会出现内核加载时从该分区加载老的设备树文件。</font>**

---

## 2025-11-26：

### 1、u-boot调试：

#### 1）编译的u-boot中增加了ext4操作命令和gpio操作命令，但是在u-boot下，ext4操作文件系统之前，需要先进行以下操作：

```sh
mmc dev 2
mmc part
ext4load mmc 2:5 0x43000000 /boot/board1_led.dtbo
```

也就是在执行ext4load从文件系统中加载之前，需要有mmc part这一步。

#### 2）DTB设计：

默认官方Linux内核**板级设备树文件**在：[/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/device/config/chips/t113_i/configs/evb1_auto/linux-5.4/board.dts](/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/device/config/chips/t113_i/configs/evb1_auto/linux-5.4/board.dts)

默认官方Linux内核**系统级设备树文件**在：[/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/kernel/linux-5.4/arch/arm/boot/dts/sun8iw20p1.dtsi](/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/kernel/linux-5.4/arch/arm/boot/dts/sun8iw20p1.dtsi)

为了兼容多种板卡的dtb设备树，我将这个文件改成了一个链接，具体指向对应的硬件板卡型号：

```sh
jason@sparky7:/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/device/config/chips/t113_i/configs/evb1_auto/linux-5.4
$ ll
总计 320K
lrwxrwxrwx 1 jason jason       15 11月26日 16:48 board.dts -> dca173_v1.0.dts
-rwxr-xr-x 1 jason autologin  48K 2025年 5月16日 board.dts.bak
-rwxr-xr-x 1 jason jason      49K 11月26日 16:40 board_sun8iw20p1_evb1_auto.dts
-rw-r--r-- 1 jason jason     8.2K 2025年 5月24日 config-5.4
-rw-rw-r-- 1 jason      1005 7.4K 2025年 5月15日 config-5.4.bak
-rw-r--r-- 1 jason autologin 5.2K 2024年10月14日 config-5.4-recovery
-rwxr-xr-x 1 jason autologin 125K 2024年10月28日 config-5.4_ubuntu
-rwxr-xr-x 1 jason jason      49K 11月26日 16:49 dca173_v1.0.dts
-rwxr-xr-x 1 jason autologin 4.1K 2024年10月14日 image_nor.cfg
```

如果需要更改或者新建一个dts设备树文件，则在该目录下可以建立一个新的dts设备树源码，然后新建链接到该文件即可：

```sh
$ ln -sf dca173_v1.0.dts board.dts
```

修改完成后，可以回到T113i的源码目录下，重新编译内核：

```sh
cd /media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/
./sanway_build.py
=========================系统菜单=========================
请选择要执行的操作:
当前目录: /media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1
0. exit - 退出程序
1. all - 全部构建
2. config - 配置build.sh编译环境
3. kernel - 编译内核
4. menuconfig - 内核编译配置
5. saveconfig - 保存内核编译配置
6. bootloader - 编译引导程序bootloader
7. uboot_menuconfig - uboot编译配置
8. uboot_saveconfig - 保存uboot编译配置
9. rootfs - 编译buildroot根文件系统
10. buildroot_menuconfig - buildroot文件系统编译配置
11. buildroot_saveconfig - 保存buildroot文件系统编译配置
12. pack - 将全部编译结果打包pack

==========================================================
请输入你的选择: 
3

最后输出成功信息：
---build dts for sun8iw20p1 evb1_auto-----
'/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/out/t113_i/kernel/build/arch/arm/boot/dts//board.dtb' -> '/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/out/t113_i/kernel/staging/sunxi.dtb'
Warning: init ramdisk file '/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/out/t113_i/evb1_auto/buildroot/ramfs/images/rootfs.cpio.gz' not exist on buildroot platform!
Use init ramdisk file: '/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/kernel/linux-5.4/rootfs_32bit.cpio.gz'.
Copy modules to target ...
old DTB_OFFSET: 15728640(0x00f00000)
new DTB_OFFSET: 19818496(0x012e6800)
bootimg_build
Copy boot.img to output directory ...

sun8iw20p1 compile all(Kernel+modules+boot.img) successful

```

