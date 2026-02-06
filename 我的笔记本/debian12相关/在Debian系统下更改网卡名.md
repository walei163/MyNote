# 在DEBIAN Linux系统下更改网卡名称

一般在DEBIAN系统下，网卡名称是一个很长的名称，如果要改为诸如`eth0`这样的名称，可以按照以下方式来操作（已经在我的`ARM64`笔记本的电脑上测试成功）：

## 1. 用`udevadm`命令查询网卡的信息：

```bash
udevadm info /sys/class/net/IF_NAME
```

一般会输出以下内容：

```bash
$ sudo udevadm info /sys/class/net/eth0
[sudo] jason 的密码：
P: /devices/pci0000:00/0000:00:03.0/net/eth0
M: eth0
R: 0
U: net
I: 2
E: DEVPATH=/devices/pci0000:00/0000:00:03.0/net/eth0
E: SUBSYSTEM=net
E: INTERFACE=eth0
E: IFINDEX=2
E: USEC_INITIALIZED=5844908
E: ID_NET_NAMING_SCHEME=v252
E: ID_NET_NAME_MAC=enx080027744199
E: ID_OUI_FROM_DATABASE=PCS Systemtechnik GmbH
E: ID_NET_NAME_PATH=enp0s3
E: ID_BUS=pci
E: ID_VENDOR_ID=0x8086
E: ID_MODEL_ID=0x100e
E: ID_PCI_CLASS_FROM_DATABASE=Network controller
E: ID_PCI_SUBCLASS_FROM_DATABASE=Ethernet controller
E: ID_VENDOR_FROM_DATABASE=Intel Corporation
E: ID_MODEL_FROM_DATABASE=82540EM Gigabit Ethernet Controller (PRO/1000 MT Desktop Adapter)
E: ID_MM_CANDIDATE=1
E: ID_PATH=pci-0000:00:03.0
E: ID_PATH_TAG=pci-0000_00_03_0
E: ID_NET_DRIVER=e1000
E: ID_NET_LINK_FILE=/usr/lib/systemd/network/99-default.link
E: ID_NET_NAME=eth0
E: SYSTEMD_ALIAS=/sys/subsystem/net/devices/eth0
E: TAGS=:systemd:
E: CURRENT_TAGS=:systemd:
```

找到`ID_NET_LINK_FILE=` 对应的文件。一般都是`/usr/lib/systemd/network/99-default.link`。

## 2. 修改`ID_NET_LINK_FILE`对应的`link`后缀的文件：

其内容一般为：

```bash
[Match]
OriginalName=*

[Link]
NamePolicy=keep kernel database onboard slot path
AlternativeNamesPolicy=database onboard slot path
MACAddressPolicy=persistent
```

修改其中的：`NamePolicy=mac`，表示根据`MAC`地址规则来命名网卡名称。修改后的内容：

```bash
[Match]
OriginalName=*

[Link]
#NamePolicy=keep kernel database onboard slot path
#AlternativeNamesPolicy=database onboard slot path
# 修改为通过mac识别
NamePolicy=mac
MACAddressPolicy=persistent
```

## 3. 在`/etc/systemd/network`目录下创建具体规则文件

在`/etc/systemd/network`目录下创建具体规则文件，例如文件名称为：`20-persistent-net.link`：

内容如下：

```bash
[Match]
MACAddress=00:50:56:2d:24:98

[Link]
Name=eth0
```

如果有多个网卡，则可以创建多个对应的`link`文件，只要确保`MAC`地址是唯一的就行。例如第二个网卡，就可以创建`30-persistent-net.link`。

完成后reboot重启系统，网卡名称就改变为`ethx`了。

> [!note]
>
> **<font face="微软雅黑" color=cyan>这种方法修改网卡名不能在[grub](https://so.csdn.net/so/search?q=grub&spm=1001.2101.3001.7020)文件中添加 net.ifnames=0 biosdevname=0</font>**

