# PEE v1.0开发说明

# 2025-09-20：

## 1、安装oebuild：

文档中第一步需要先安装oebuild，需要按照如下方式安装：

### 1）安装pipx：

```sh
sudo apt install pipx
```

> **注意：**
>
> **在安装pipx之前，要先安装以下几个包：**
>
> ```
> sudo apt install python3 python3-pip docker.io
> ```
>
> **然后将用户加入到docker组：**
>
> ```
> sudo add jason docker
> ```
>
> **重启系统，再继续安装。**



### 2）用pipx安装oebuild：

```sh
$ pipx install oebuild==v0.0.27
```

如果安装成功，出现如下提示：

```sh
installed package oebuild 0.0.27, installed using Python 3.11.2
  These apps are now globally available
    - oebuild
⚠️  Note: '/home/jason/.local/bin' is not on your PATH environment variable. These apps will not be globally accessible until your PATH is updated. Run `pipx ensurepath` to automatically add it, or manually modify your PATH in your
    shell's config file (i.e. ~/.bashrc).
done! ✨ 🌟 ✨
```

> **注意：有时由于网络的问题，需要多安装几次才能成功。**

### 3）配置oebuild的环境变量，编辑home下的.bashrc文件：

```sh
PATH="$HOME/.local/bin/:$PATH"
```

### 4）配置完成后，进入到目录：/media/jason/btrfs_disk_M/Develop/powereuler-embedded下，运行：

```sh
oebuild update
```

提示如下：

```sh
$ oebuild update
2025-09-20 15:16:35,347 - INFO - Fetching into /media/jason/btrfs_disk_M/Develop/powereuler-embedded/src/yocto-meta-openeuler ...

remote: Counting objects: 100% (135496.0/135496.0), done

remote: Compressing objects: 100% (4998.0/4998.0), done

Receiving objects: 100% (272274.0/272274.0), done8 MiB | 260.00 KiB/s

2025-09-20 15:23:23,129 - INFO - pull swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/openeuler-container:22.03-lts-sp2 ...
2025-09-20 15:27:35,376 - INFO - finishd pull swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/openeuler-container:22.03-lts-sp2 ...                                                                                                       0847bcb5dfdb: Pull complete                                                                                                                                                                                                                   3187cbf4c087: Pull complete                                                                                                                                                                                                                   3f14de7279c2: Pull complete                                                                                                                                                                                                                   416428d7bb5a: Pull complete                                                                                                                                                                                                                   43afc64e01b2: Pull complete                                                                                                                                                                                                                   8efa9de04a95: Pull complete                                                                                                                                                                                                                   bda49c9296b9: Pull complete                                                                                                                                                                                                                   bf37c47dc58e: Pull complete                                                                                                                                                                                                                   e1828d5de5fe: Pull complete                                                                                                                                                                                                                   Digest: sha256:8714c3a27cfec6b6dab7dc3fd5362352154e7c348b0d748e233c4bfad6cfb9c2                                                                                                                                                               Status: Downloaded newer image for swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/openeuler-container:22.03-lts-sp2                                                                                                                      2025-09-20 15:27:35,399 - INFO - Fetching into /media/jason/btrfs_disk_M/Develop/powereuler-embedded/src/yocto-poky ...

remote: Counting objects: 100% (356.0/356.0), done

remote: Compressing objects: 100% (291.0/291.0), done

Receiving objects: 100% (356.0/356.0), done0 KiB | 253.00 KiB/s

2025-09-20 15:27:42,487 - INFO - Fetching into /media/jason/btrfs_disk_M/Develop/powereuler-embedded/src/yocto-meta-openembedded ...

```

> **注意：**
>
> **这一步操作实际上是去网站：swr.cn-north-4.myhuaweicloud.com/openeuler-embedded/openeuler-container下载yocto容器映像，以使得可以使用bitbake来进行系统编译等操作。如果之前已经下载过容器映像，则这一步可以省略。**



### 5）进入bitbake容器环境：

```sh
$ cd build/powereuler-dgri/
$ oebuild bitbake 

2025-09-20 15:33:15,415 - INFO - bitbake starting ...


Welcome to 6.1.0-38-amd64

System information as of time: 	Sat Sep 20 07:33:18 UTC 2025

System load: 	0.32
Processes: 	6
Memory used: 	10.8%
Swap used: 	.2%
Usage On: 	7%
Users online: 	0
To run a command as administrator(user "root"),use "sudo <command>".

Welcome to the openEuler Embedded build environment,
where you can run bitbake openeuler-image to build
standard images
```

### 6）编译rootfs文件系统：

```sh
[openeuler@debian12 powereuler-dgri]$ bitbake openeuler-image
```

如果编译无误，整个编译提示如下：

```sh
Loading cache: 100% |                                                                                                                                                                                                         | ETA:  --:--:--
Loaded 0 entries from dependency cache.
Parsing recipes: 100% |########################################################################################################################################################################################################| Time: 0:00:31
Parsing of 2392 .bb files complete (0 cached, 2392 parsed). 3692 targets, 360 skipped, 0 masked, 0 errors.
Removing 120 recipes from the dgri sysroot: 100% |#############################################################################################################################################################################| Time: 0:00:42
NOTE: Resolving any missing task queue dependencies

Build Configuration:
BB_VERSION           = "1.50.0"
BUILD_SYS            = "x86_64-linux"
NATIVELSBSTRING      = "openeuler-22.03"
TARGET_SYS           = "aarch64-powereuler-linux"
MACHINE              = "dgri"
DISTRO               = "powereuler"
DISTRO_VERSION       = "1.0"
TUNE_FEATURES        = "aarch64"
TARGET_FPU           = ""
EXTERNAL_TOOLCHAIN   = "/usr1/openeuler/gcc/openeuler_gcc_arm64le"
EXTERNAL_TARGET_SYS  = "aarch64-openeuler-linux-gnu"
GCC_VERSION          = "10.3.1"
OPENEULER_TOOLCHAIN_VERSION = "10.3.1"
meta                 = "HEAD:230e29ee70a751dea2bbba1d69b6fa803db51353"
meta-oe              
meta-python          
meta-networking      
meta-filesystems     = "HEAD:54204353f2953d0cd36a8d5ed35ce9a560852ff7"
meta-openeuler       
meta-openeuler-bsp   
meta-dgri            = "openEuler-22.03-LTS-SP2:654809887988aa6bbacb33ff8e308d70e9ea4b39"
workspace            = "<unknown>:<unknown>"

Initialising tasks: 100% |#####################################################################################################################################################################################################| Time: 0:00:01
Sstate summary: Wanted 805 Local 802 Network 0 Missed 3 Current 117 (99% match, 99% complete)
Removing 2 stale sstate objects for arch dgri: 100% |##########################################################################################################################################################################| Time: 0:00:00
Removing 5 stale sstate objects for arch allarch: 100% |#######################################################################################################################################################################| Time: 0:00:00
NOTE: Executing Tasks
NOTE: Tasks Summary: Attempted 2037 tasks of which 1908 didn't need to be rerun and all succeeded.
```

### 7）编译后的结果：

最后编译出的rootfs文件在目录：[/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/output/](/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/output/)

下，以年月日时分秒建立的目录，例如：[20250920073449](/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/output/20250920073449/)

---

# 1、按照《[PEE开发环境搭建-v3.pdf](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/电力鸿蒙/PEE v1.0/PEE开发环境搭建-v3.pdf)》文档说明，最后编译生成SDK：

```sh
$ bitbake openeuler-image -c do_populate_sdk 生成 sdk
```

之后，会在：/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/output/20241211070100/下产生一个文件：

```sh
powereuler-glibc-x86_64-openeuler-image-aarch64-dgri-toolchain-1.0.sh
```

该文件是一个SDK安装脚本，运行该脚本，出现如下提示内容：

```sh
jason@xubuntu:/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/output/20241211070100
$ ./powereuler-glibc-x86_64-openeuler-image-aarch64-dgri-toolchain-1.0.sh 
PowerHarmony+ PEE SDK installer version 1.0
===========================================
Enter target directory for SDK (default: /opt/powereuler/oecore-x86_64): 
You are about to install the SDK to "/opt/powereuler/oecore-x86_64". Proceed [Y/n]? y
[sudo] password for jason: 
Extracting SDK.....................................done
Setting it up...SDK has been successfully set up and is ready to be used.
Each time you wish to use the SDK in a new shell session, you need to source the environment setup script e.g.
 $ . /opt/powereuler/oecore-x86_64/environment-setup-aarch64-powereuler-linux

```

2、除了`dgri-rk3568`这个platform之外，我们还可以设置其他平台进行编译，可以通过以下命令来查看`yocto-meta-openeuler`支持的platform：

```sh
jason@xubuntu:/media/jason/btrfs_disk_M/Develop/powereuler-embedded
$ oebuild generate -l platform
2024-12-16 11:09:09,837 - INFO - =============================================
the platform list is:
arm-std 
dgri-t3 
x86-64-std 
ok3568 
ryd-3568 
aarch64-std 
visionfive2 
hi3093 
raspberrypi4-64 
dgri-rk3568 
riscv64-std 
```

例如我们选择采用`ryd-3568`来作为新的编译平台，可以输入以下命令：

```sh
$ oebuild generate -p ryd-3568 -f systemd -d ryd3568
2024-12-16 11:13:26,444 - INFO - 
generate compile.yaml successful

please run follow command:
=============================================

cd /media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/ryd3568
oebuild bitbake

=============================================
```

将会创建目录：`powereuler-embedded/build/ryd3568`，进入该目录，运行：

```sh
oebuild bitbake
```

将进入bitbake容器环境，然后可以执行编译：

```sh
[openeuler@xubuntu ryd3568]$ bitbake openeuler-image
```

> [!note]
>
> **编译成功后，如果想要重新进行编译，则需要执行：**
>
> ```sh
> [openeuler@xubuntu ryd3568]$bitbake -c clean  openeuler-image
> ```
>
> 然后再一次运行：
>
> ```sh
> [openeuler@xubuntu ryd3568]$bitbake openeuler-image
> ```
>
> 如果只想要编译Linux内核，则可以执行：
>
> ```sh
> [openeuler@xubuntu ryd3568]$bitbake linux-openeuler
> ```
>
> 
