# u-boot环境变量修改

可以用fw_printenv和fw_setenv来修改和查看U-boot下的环境变量。关键是一定要设置好/etc/fw_env.config这个文件的配置：

```sh
# Allwinner Tina volume
/dev/mmcblk0    			0x34d0c00               0x20000
/dev/by-name/env-redund         0x00000         0x20000
```

注意上面的地址：0x34d0c00是通过以下方式获取的：

```sh
# sgdisk -p /dev/mmcblk0
Disk /dev/mmcblk0: 15126528 sectors, 7.2 GiB
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
   8         2279430        15126493   6.1 GiB     0700  Basic data partition
```

`env`所在的起始扇区：108166 x 512计算出来的。

其实可以直接按照下面的方式来写fw_env.config配置文件：

```sh
# cat /etc/fw_env.config 
# Allwinner Tina volume
/dev/mmcblk1p2          0x00000         0x20000
/dev/mmcblk1p3          0x00000         0x20000
```

**注意前面的设备名称要改为分区名称，且一定要带上绝对路径！**

这样设置好配置文件，就可以查看和修改u-boot环境变量的值了：

```sh
# ./fw_printenv
boot_dsp0=sunxi_flash read 43000000 ${dsp0_partition};bootr 43000000 0 0
boot_fastboot=fastboot
boot_normal=sunxi_flash read 43000000 boot;bootm 43000000
boot_recovery=sunxi_flash read 43000000 recovery;bootm 43000000
bootcmd=run setargs_mmc boot_normal
bootdelay=3
cma=16M
console=ttyS0,115200
dsp0_partition=dsp0
earlycon=uart8250,mmio32,0x02500000
fdtcontroladdr=7c87ce68
force_normal_boot=1
init=/init
initcall_debug=0
keybox_list=widevine,ec_key,ec_cert1,ec_cert2,ec_cert3,rsa_key,rsa_cert1,rsa_cert2,rsa_cert3
loglevel=8
mmc_root=/dev/mmcblk0p5
mtd_name=sys
mtdids=
mtdparts=
nand_root=ubi0_5
partition=
partitions=boot-resource@mmcblk0p1:env@mmcblk0p2:env-redund@mmcblk0p3:boot@mmcblk0p4:rootfs@mmcblk0p5:dsp0@mmcblk0p6:private@mmcblk0p7:Basic data partition@mmcblk0p8
rootfstype=ubifs,rw
setargs_mmc=setenv  bootargs earlycon=${earlycon} clk_ignore_unused initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${mmc_root}  rootwait init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} specialstr=${specialstr} gpt=1
setargs_nand=setenv bootargs ubi.mtd=${mtd_name} earlycon=${earlycon} clk_ignore_unused initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nand_root} rootfstype=${rootfstype} rootwait init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} specialstr=${specialstr} gpt=1
setargs_nand_ubi=setenv bootargs ubi.mtd=${mtd_name} earlycon=${earlycon} clk_ignore_unused initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nand_root} rootfstype=${rootfstype} rootwait init=${init} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} specialstr=${specialstr} gpt=1
```



