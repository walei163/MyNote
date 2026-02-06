# HSR和PRP相关源码和设置说明

[TOC]



## 一、源码和内核配置：

源代码位于内核目录：`net/hsr-prp/`下面，默认已经编译进内核中。

如果要配置的话，可以按照以下方式：

内核配置界面：

```bash
[*] Networking support  ---> 
	Networking options  --->
		 <*> IEC 62439 HSR/PRP Support
```

配置完成后，重新编译内核，即实现了`HSR/PRP`功能。

## 二、设置与使用方法：

### 1、HSR设置使用：

一般采用`iproute2`来配置（设系统的2路以太网设备名称分别为：`eth0/eth1`）：

- 首先，需要将2路以太网设备的MAC地址设为同一个MAC地址：

```sh
ifconfig eth0 0.0.0.0 down
ifconfig eth1 0.0.0.0 down

ifconfig eth0 hw ether aa:9d:41:f2:69:7f
ifconfig eth1 hw ether aa:9d:41:f2:69:7f

ifconfig eth0 up
ifconfig eth1 up
```

- 其次，采用`iproute2`命令将2路以太网加入到一个新的虚拟设备：`hsr0`中，并且其类型为`hsr`：

```sh
ip link add name hsr0 type hsr slave1 eth0 slave2 eth1 supervision 45 version 1
```

> [!note]
>
> **此处需要注意，如果`iproute2`的版本过于陈旧，则有可能设置不成功，则需要升级新版本的`iproute2`才能支持。**

- 最后，将`hsr0`作为一个新的网卡设备，给其设置一个IP地址：

```sh
ifconfig hsr0 192.168.1.230
```

这样设置完成后，`HSR`环网就设置成功了。

> [!tip]
>
> 可以用3台装置搭建一个测试环境，将3台均按照以上方式设置，然后每台装置的网线按照手拉手方式连接起来。即可以进行测试。

### 2、PRP设置使用：

`PRP`模式设置与`HSR`模式基本相同：

- 首先，需要将2路以太网设备的MAC地址设为同一个MAC地址：

```sh
ifconfig eth0 0.0.0.0 down
ifconfig eth1 0.0.0.0 down

ifconfig eth0 hw ether aa:9d:41:f2:69:7f
ifconfig eth1 hw ether aa:9d:41:f2:69:7f

ifconfig eth0 up
ifconfig eth1 up
```

- 其次，采用`iproute2`命令将2路以太网加入到一个新的虚拟设备：`prp0`中，并且其类型为`hsr`：

```sh
ip link add name prp0 type hsr slave1 eth0 slave2 eth1 supervision 45 proto 1
```

> [!note]
>
> **按照上面的命令有可能设置失败，则可能需要升级`iproute2`命令的版本**。
>
> <font face="微软雅黑" color=blue>**1、经过研究内核代码发现，上面的设置只在5.10以上的内核上可能实现，因为内核代码是通过读取proto的值来判断是否打开prp协议的。**</font>
>
> 参见内核代码：/net/hsr/hsr_netlink.c中的相关设计。
>
> <font face="微软雅黑" color=blue>**2、在设置prp协议时，可以不要以下字段：supervision 45，这个是用于设置：HSR supervision frames，HSR supervision frames（HSR监测帧）是交换机中用于实现HSR冗余协议的关键技术，主要用于检测网络中节点的存活状态并维护节点表。设置这个会打开网络多播，让网卡接收报文进入混杂模式。**</font>
>
> **或者也可以按照以下命令来设置：**
>
> 以下设置是在4.19的内核版本上可以实现。
>
> ```sh
> ip link add name prp0 type hsr slave1 eth0 slave2 eth1 supervision 45 version 2
> echo 1 > /proc/prp0/prp-tr
> ```

- 最后，将`prp0`作为一个新的网卡设备，给其设置一个IP地址：

```sh
ifconfig prp0 192.168.1.230
```

这样设置完成后，`PRP`环网就设置成功了。

---

> [!important]
>
> <font face="微软雅黑" color=green>**相关参考资料**</font>
>
> 关于ip-link设置的详细说明文档：[/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/ip-link.8.pdf](/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/ip-link.8.pdf)
>
> iproute2的源码包：[/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/iproute2-5.9.0.tar.gz](/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/iproute2-5.9.0.tar.gz)
>
> 交叉编译说明：[/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/iproute2交叉编译.pdf](/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/iproute2交叉编译.pdf)
>
> HSR执行脚本：[/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/rc.hsr](/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/rc.hsr)
>
> PRP执行脚本：[/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/rc.prp](/mnt/hgfs/E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/HSR和PRP协议相关/hsr和prp设置应用相关_20230529/rc.prp)