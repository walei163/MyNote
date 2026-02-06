# T536上u-boot加载dtbo的方法

## 2025-12-04：

> [!important]
>
> 今天发现，除了以下描述的方法之外，如果想要在u-boot下正确加载增量dtbo，还必须在Linux内核中打开相关配置。如下图：
>
> ```sh
> Device Drivers  ---> 
> 	-*- Device Tree and Open Firmware support  --->
> 		--- Device Tree and Open Firmware support
> 		[ ]   Device Tree runtime unit tests
> 		[*]   Device Tree overlays
> ```
>
> <font color=red>**经过这样配置后，重新编译内核，产生的`boot.img`才能支持在u-boot下通过`fdt apply`命令加载`dtbo`增量设备树。**</font>



---

## 一、u-boot增量dtbo加载

1、在U-boot下设定环境变量：fdt_load_addr

```sh
setenv fdt_load_addr 43000000
savenv
```

2、将增量dtbo文件加载到内存（默认这些dtbo文件在/boot目录下）。例如：

```sh
load mmc 0:5 ${fdt_load_addr} /boot/dts/dts-overlay-1.dtbo
load mmc 0:5 ${fdt_load_addr} /boot/dts/wld2660-overlay-1.dtb
```

3、将增量dtbo合并到基础dtb中：

```sh
fdt apply ${fdt_load_addr}
```

4、启动系统

```sh
boot
```

---

## 二、在电脑主机上合并dtbo，u-boot加载整个dtb

还有一种方法，可以直接在电脑端将增量dtbo和基础dtb直接合并。例如：

```sh
fdtoverlay -i sunxi.dtb -o merged.dtb wld2660-overlay-1.dtb
```

其中：

- sunxi.dtb是基础dtb文件
- wld2660-overlay-1.dtb是增量dtbo
- merged.dtb是合并的最终dtb文件。

然后可以在U-boot中直接加载并运行：

```sh
load mmc 0:5 ${fdtaddr} /boot/merged.dtb
boot
```

**注意这里加载的地址，是`fdtaddr`，也就是u-boot启动时传给内核的dtb地址。**

---

## 三、设置u-boot环境变量，系统启动时自动加载dtbo

为了方便系统系统时直接加载增量dtbo，我们可以在Linux系统通过fw_setenv来设置u-boot环境变量。设置好的环境变量如下：

```sh
dgri:~# fw_printenv 
boot_fastboot=fastboot
boot_kernel=mmc read ${sunxi_load_addr} ${kernel_start_blk} ${kernel_blk_count};bootm ${sunxi_load_addr} ${sunxi_ramd_addr} ${sunxi_fdt_addr}
boot_normal=sunxi_flash read 0x4007f800 ${boot_partition};bootm 0x4007f800 ${sunxi_ramd_addr} ${sunxi_fdt_addr}
boot_partition=boot
boot_recovery=sunxi_flash read 0x4007f800 recovery;bootm 0x4007f800
boot_riscv=bootrv 46000000 200000 0 riscv0 riscv0-r
bootcmd=run dtbo_test; run sunxi_pre_cmd boot_normal
bootdelay=3
bootstopkey=q
coherent_pool=16K
console=ttyAS0,115200
dtbo_file=/boot/dtb/dts-overlay-1.dtbo
dtbo_load=load mmc ${dtbo_mmc} ${fdt_load_addr} ${dtbo_file}; fdt apply ${fdt_load_addr}
dtbo_mmc=0:5
dtbo_test=run dtbo_load
earlyprintk=sunxi-uart,0x02600000
eth1addr=9a:54:a8:be:69:4e
ethaddr=1a:78:7d:1f:61:84
fdt_high=0xffffffff
fdt_load_addr=43000000
fdtaddr=42000000
fdtcontroladdr=b7d881d0
fel_booted=1
fileaddr=b8099000
filesize=1c2038
force_normal_boot=1
init=/init
initcall_debug=0
kernel_blk_count=00030000
kernel_start_blk=0002a000
loglevel=8
mmc_root=/dev/mmcblk0p5
mtd_name=sys
nand_root=ubi0_4
nor_root=/dev/mtdblock1
partitions=boot-resource@mmcblk0p1:env@mmcblk0p2:boot@mmcblk0p3:private@mmcblk0p4:rootfs@mmcblk0p5:riscv0@mmcblk0p6:UDISK@mmcblk0p7
preboot=sunxi_get_kern_env;sunxi_preboot
rdinit=/rdinit
riscv_partition=riscv0
root_partition=rootfs
rootfstype=ubifs, rw
setargs_mmc=setenv  bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${mmc_root} rootwait init=${init} rdinit=${rdinit} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} coherent_pool=${coherent_pool} ion_carveout_list=${reserve_list} gpt=1
setargs_nand=setenv bootargs earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel}  ubi.mtd=${mtd_name} root=${nand_root} rootfstype=${rootfstype} rootwait init=${init} rdinit=${rdinit} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} selinux=${selinux} specialstr=${specialstr} coherent_pool=${coherent_pool} ion_carveout_list=${reserve_list}
setargs_nand_ubi=setenv bootargs ubi.mtd=${mtd_name} earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nand_root} rootfstype=${rootfstype} init=${init} rdinit=${rdinit} partitions=${partitions} cma=${cma} snum=${snum} mac_addr=${mac} wifi_mac=${wifi_mac} bt_mac=${bt_mac} specialstr=${specialstr} gpt=1
setargs_nor=setenv bootargs  earlyprintk=${earlyprintk} initcall_debug=${initcall_debug} console=${console} loglevel=${loglevel} root=${nor_root} rootwait init=${init} rdinit=${rdinit} partitions=${partitions} cma=${cma} snum=${snum} coherent_pool=${coherent_pool} ion_carveout_list=${reserve_list}
snum=7c000c696404081210e
sunxi_fdt_addr=42000000
sunxi_load_addr=0x4007f800
sunxi_pre_cmd=${sunxicmd};sunxi_update
sunxi_ramd_addr=-
sunxicmd=run setargs_mmc
verify=N
```

需要设置的环境变量为：

```sh
bootcmd=run dtbo_test; run sunxi_pre_cmd boot_normal
fdt_load_addr=43000000
dtbo_file=/boot/dtb/dts-overlay-1.dtbo
dtbo_load=load mmc ${dtbo_mmc} ${fdt_load_addr} ${dtbo_file}; fdt apply ${fdt_load_addr}
dtbo_mmc=0:5
dtbo_test=run dtbo_load
```

设置方法：

1. 采用fw_setenv：

```sh
fw_setenv fdt_load_addr 43000000
fw_setenv dtbo_file '/boot/dtb/dts-overlay-1.dtbo'
fw_setenv dtbo_mmc '0:5'
fw_setenv dtbo_load 'load mmc ${dtbo_mmc} ${fdt_load_addr} ${dtbo_file}; fdt apply ${fdt_load_addr}'
fw_setenv dtbo_test 'run dtbo_load'
fw_setenv bootcmd 'run dtbo_test; run sunxi_pre_cmd boot_normal'
```

2. 采用dd方式直接刷写env分区：

```sh
dd if=uboot_env.img of=/dev/mmcblk0p2 bs=1M
```

我已经制作了脚本文件：`update_linux.sh`，在该脚本文件中，有可以升级并配置dtb的选项。

具体会将dtbo文件复制到rootfs文件系统的/boot/dtb目录下并建立链接。然后重启系统就可以自动加载dtbo，并且合并到base dtb文件中，再引导内核。

## 四、一个具体的dtbo的代码例子（已经经过验证）

```c
/dts-v1/;
/plugin/;

//&{}要采用全路径的方式指定
//路径的具体信息可参考文件：fdt.dts中的__symbols__的内容
//fdt文件来自于板子上的/sys/firmware/fdt
/ {
	compatible = "sun55iw6,sanway_swa536";
};

&{/led} {
        status = "okay";
        led-trigger1 {
        	label = "heartbeat1";        	
        	linux,default-trigger = "heartbeat";
        	default-state = "off";
        };
};

&{/soc@3000000/pinctrl@3604000} {
        uart1_pins@0 {
        	pins = "PG6", "PG7", "PG8", "PG9";
        	//pins = "PH0", "PH1";
        	function = "uart1";
        	bias-pull-up;
        };
        uart1_pins@1 {
        	pins = "PG6", "PG7", "PG8", "PG9";
        	//pins = "PH0", "PH1";
        	function = "io_disabled";
        };
};

&{/soc@3000000} {
	uart@2601000 {
		status = "disabled";
	};
};
```

## 五、多个dtbo文件生成叠加层的使用加载方法（u-boot加载）

还可以将多个dtbo文件通过`mkdtimg`命令生成叠加层文件：`dtbo.img`，然后在u-boot下选择其中一个来进行加载。方法：

### 1. 用命令：`mkdtimg`配合配置文件：`dtboimg.cfg`生成叠加层文件：

```sh
./mkdtimg cfg_create 111.img dtboimg.cfg
```

这样将生成一个名为：`111.img`的叠加层文件映像。

配置文件：`dtboimg.cfg`的内容：

```sh
# global options
  id=/:board_id
  rev=/:board_rev
  custom0=0xabc

#entry
sunxi_board1.dtbo
	id=0x6800
	custom0=0x55551234
	custom1=0x55551234
	custom2=0x55551234
	custom3=0x55551234

sunxi_board2.dtbo
  id=0x6800       # override the value of id in global options

sunxi_board3.dtbo
  id=0x6801       # override the value of id in global options
  custom3=0x123   # override the value of custom0 in global options

board1_led.dtbo
  id=0x6802
  rev=1.0
  custom3=0x111   # override the value of custom0 in global options
```

生成的`111.img`可以通过以下方式来查看其内容：

```sh
$ ./mkdtimg dump 111.img 
dt_table_header:
               magic = d7b7ab1e
          total_size = 1795
         header_size = 32
       dt_entry_size = 32
      dt_entry_count = 4
   dt_entries_offset = 32
           page_size = 2048
         reserved[0] = 00000000
dt_table_entry[0]:
             dt_size = 436
           dt_offset = 160
                  id = 00006800
                 rev = 01000200
           custom[0] = 55551234
           custom[1] = 55551234
           custom[2] = 55551234
           custom[3] = 55551234
           (FDT)size = 436
     (FDT)compatible = board_manufacturer,board_model
dt_table_entry[1]:
             dt_size = 232
           dt_offset = 596
                  id = 00006800
                 rev = 00000000
           custom[0] = 00000abc
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 232
     (FDT)compatible = (unknown)
dt_table_entry[2]:
             dt_size = 232
           dt_offset = 828
                  id = 00006801
                 rev = 00000000
           custom[0] = 00000abc
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000123
           (FDT)size = 232
     (FDT)compatible = (unknown)
dt_table_entry[3]:
             dt_size = 735
           dt_offset = 1060
                  id = 00006802
                 rev = 00000001
           custom[0] = 00000abc
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000111
           (FDT)size = 735
     (FDT)compatible = sun55iw6,sanway_swa536
```

### 2. u-boot下加载叠加层映像，需要用到`adtimg`命令，如果没有则需要重新编译u-boot源码：

**注意：以下操作均在u-boot的shell环境下。**

#### 2.1. 将`111.img`叠加层文件从eMMC加载到指定的内存地址：0x44000000。

```sh
setenv adt_load_addr 44000000
load mmc ${dtbo_mmc}  ${adt_load_addr} /boot/dtb/111.img
```

#### 2.2 指定当前adtimg的内存地址为0x44000000：

```sh
adtimg addr ${adt_load_addr}
```

如果要确认设置是否成功，可以通过以下命令来查看：

```sh
sanway> adtimg dump
dt_table_header:
               magic = d7b7ab1e
          total_size = 1028
         header_size = 32
       dt_entry_size = 32
      dt_entry_count = 3
   dt_entries_offset = 32
           page_size = 2048
             version = 0
dt_table_entry[0]:
             dt_size = 436
           dt_offset = 128
                  id = 00006800
                 rev = 01000200
           custom[0] = 55551234
           custom[1] = 55551234
           custom[2] = 55551234
           custom[3] = 55551234
Error: Root node not found
dt_table_entry[1]:
             dt_size = 232
           dt_offset = 564
                  id = 00006800
                 rev = 00000000
           custom[0] = 00000abc
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 232
     (FDT)compatible = (unknown)
dt_table_entry[2]:
             dt_size = 232
           dt_offset = 796
                  id = 00006801
                 rev = 00000000
           custom[0] = 00000abc
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000123
           (FDT)size = 232
     (FDT)compatible = (unknown)
```

表明加载设置成功。并且显示，一共有3个`dt_table_entry`入口，其值从0~2。

#### 2.3. 选择其中一个作为dtbo overlay叠加到主dtb上：

```sh
adtimg get dt --index=0 fdt_load_addr
```

上面的含义，表示将`dt_table_entry`入口索引值为0的dtbo加载，并且更新`fdt_load_addr`的地址值。如果加载成功，则`fdt_load_addr`的值会发生改变：

```sh
sanway> print fdt_load_addr
fdt_load_addr=44000080
```

接着就可以将这个叠加到主dtb上：

```sh
fdt apply ${fdt_load_addr}
```

这样就将叠加层映像文件：111.img中的dtbo叠加到主dtb上了。



