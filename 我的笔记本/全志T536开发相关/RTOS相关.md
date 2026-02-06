# RTOS相关

1、编译：

进入到目录rtos下，运行：

```sh
$ source envsetup.sh 
Setup env done!
Run lunch_rtos to select project
```

根据提示，运行：lunch_rtos选择项目名称：

```sh
$ lunch_rtos
last=t536_e907_demo

You're building on Linux

Lunch menu... pick a combo:
     1. mr527_e906_evb
     2. mr527_e906_pro1
     3. mr536_e907_evb1
     4. t536_demo
     5. t536_e907_demo

Which would you like? [Default t536_e907_demo]: 
select=...
t536_e907/demo
'/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos/projects/t536_e907/demo/defconfig' -> '/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos/.config'
============================================
RTOS_BUILD_TOP=/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos
RTOS_TARGET_ARCH=riscv
RTOS_TARGET_CHIP=sun55iw6p1
RTOS_TARGET_DEVICE=t536_e907
RTOS_PROJECT_NAME=t536_e907_demo
============================================
Run mrtos_menuconfig to config rtos
Run m or mrtos to build rtos
```

接着运行：mrtos_menuconfig来配置rtos，会出现错误提示：

```sh
$ mrtos_menuconfig 
cd /media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos/ && TERM=xterm-color CONFIG_=CONFIG_ PROJECT_DIR=/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos /media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos/scripts/kconfig-linux64/kconfig-mconf Kconfig
Error opening terminal: xterm-color.
make: *** [/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/rtos/lichee/rtos/.dbuild/dbuild.mk:253：menuconfig] 错误 1
```

**解决方法：**

```sh
cd /usr/share/terminfo
sudo mkdir x
sudo ln -s /lib/terminfo/x/xterm-color xterm-color
```

配置完成后，编译rtos系统：

```sh
mrtos
```

最后再和Linux系统一样，进行：

```sh
./build.sh
./build.sh pack
```

---

**注意！也可以直接在系统根目录下运行：**

```sh
./build.sh rtos
./build.sh rtos menuconfig
```

来进行编译和配置。