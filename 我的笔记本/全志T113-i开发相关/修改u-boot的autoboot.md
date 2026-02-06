# 修改u-boot的autoboot

## 1、修改原因：

​	默认U-boot在启动时有一个bootdelay延时，如果此时按下enter回车键，则直接进入U-boot的shell环境下。这样可能会导致在某些使用环境下，由于外部的干扰，导致系统启动时不能自动启动Linux，而直接进入到了U-boot的shell下。

所以需要将U-boot的autoboot设置为启用`bootstopkey`模式。

## 2、修改方法：

### 2.1. 配置U-boot：

```sh
./build.sh uboot_menuconfig
```

```sh
Command line interface  --->
	(sanway> ) Shell prompt
		Autoboot options  --->
			[*] Autoboot 
			[*] Stop autobooting via specific input key / string
			(Autoboot in %d seconds\n) Autoboot stop prompt
			(qw)    Stop autobooting via specific input key / string
```

配置完成后需要：

```sh
./build.sh uboot_saveconfig
```

但此时发现提示：

```sh
$ ./build.sh uboot_saveconfig
========ACTION List: handle_defconfig uboot_saveconfig;========
options : 
INFO: Prepare toolchain ...
md5sum: configs/savedefconfig: No such file or directory
scripts/kconfig/conf  --savedefconfig=defconfig Kconfig
board/sunxi/Kconfig:326:warning: defaults for choice values not supported
```

也就是配置不支持生成default默认配置。这时需要手动来改一下，将U-boot当前的配置作为default默认配置生效：

```sh
cd /media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/brandy/brandy-2.0/u-boot-2018
cp -raf .config   configs/sun8iw20p1_auto_t113_i_defconfig
```

**即将U-boot源码目录下的`.config`配置文件复制到configs目录下并覆盖原来的defconfig配置文件：`sun8iw20p1_auto_t113_i_defconfig`**。

经过这样修改后，U-boot的配置才算完成。

### 2.2. 编译系统：

接下来要编译系统：

**第一步，编译系统：**

```sh
./build.sh
```

这一步将会进行整个系统编译，包括U-boot，kernel和buildroot文件系统。

**第二步，pack系统：**

```sh
./build.sh pack
```

如果pack无误，将会在out目录下生成最终的img文件，并且用于生成img的所有文件会在系统的`pack_out`目录下：

```sh
/media/jason/btrfs_disk_M/Develop/allwinner/T113X_V1.1/out/pack_out/
```

### 2.3. 刷写U-boot映像：

**第一步，刷写u-boot映像所在的扇区：**

默认系统并没有给U-boot单独分区，因此需要按照文档[《刷写eMMC或者TF卡》](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/全志T113-i开发相关/刷写eMMC或者TF卡.md)来操作，将`pack_out`目录下的文件：`boot_package.fex`按照文档中描述那样刷到eMMC或者TF卡的对应扇区中即可。

**第二步，还需要设置U-boot环境变量，添加`bootstopkey`这个环境变量：**

```sh
bootdelay=5
bootstopkey=q
```

如果是在Linux系统下，可以用`fw_setenv`来设置：

```sh
fw_setenv bootstopkey q
```

**注意：**

**由于刷写之后，原来的回车键将不再起作用，因此在刷写之前，最好先设置好`bootstopkey`这个环境变量，再进行U-boot映像的刷写。**

这样，系统重启后就可以通过设置的`bootstopkey`来进入U-boot的shell环境下了。