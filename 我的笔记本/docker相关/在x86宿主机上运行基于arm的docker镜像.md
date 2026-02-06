# 在x86宿主机上运行基于arm的docker镜像

经过一段时间的研究，发现可以在基于x86的宿主机上运行基于ARM平台的docker镜像。现将其方案说明如下：

1、正常安装docker。安装完成后将用户加入docker组：

```sh
sudo adduser jason docker
```



2、安装qemu两个组件：

```sh
sudo apt install qemu-system-arm
sudo apt install qemu-user-static
```

3、下载基于ARM的docker镜像，如果armhf的，则一般为：

```sh
arm32v7/ubuntu:18.04
```

如果是基于aarch64的，则镜像名称一般为：

```sh
arm64v8/ubuntu:18.04
```

其它操作就和普通docker操作一样了。

4、armhf国内镜像源：

ubuntu20.04:

```sh
    deb http://mirrors.aliyun.com/ubuntu-ports/ focal main restricted universe multiverse
    deb http://mirrors.aliyun.com/ubuntu-ports/ focal-updates main restricted universe multiverse
    deb http://mirrors.aliyun.com/ubuntu-ports/ focal-backports main restricted universe multiverse
    deb http://mirrors.aliyun.com/ubuntu-ports/ focal-security main restricted universe multiverse
```

ubuntu16.04:

```sh
deb http://mirrors.aliyun.com/ubuntu-ports/ xenial main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ xenial-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ xenial-backports main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ xenial-security main restricted universe multiverse
```

ubuntu18.04:

```sh
deb http://mirrors.aliyun.com/ubuntu-ports/ bionic main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ bionic-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ bionic-backports main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu-ports/ bionic-security main restricted universe multiverse
```

