# QEMU模拟ARM64虚拟机（Windows下安装）

1. ## 准备文件qemu,并安装

```sh
https://qemu.weilnetz.de/w64/
https://qemu.weilnetz.de/w64/qemu-w64-setup-20240423.exe
#下载 QEMU UEFI 固件文件
```

## 2.创建虚拟网卡TAP-windows

```sh
https://build.openvpn.net/downloads/releases/tap-windows-9.24.7-I601-Win10.exe
安装后，多出来一个网卡
重命名新网卡tap-1212
```

QEMU 两种上网方式(不同的网络后端)：

- **user mode network :**

这种方式实现虚拟机上网很简单，类似vmware里的`nat`，qemu启动时加入-user-net参数，虚拟机里使用dhcp方式，即可与互联网通信，但是这种方式虚拟机与主机的通信不方便。这种方式实现虚拟机上网很简单，类似vmware里的`nat`，qemu启动时加入-user-net参数，虚拟机里使用`dhcp`方式，即可与互联网通信
比如：

```sh
-net nic -net user,hostfwd=tcp::2222-:22,hostfwd=tcp::15236-:5236
```

- **tap/tun network :**

这种方式要比user mode复杂一些，但是设置好后 虚拟机<–>互联网 虚拟机<–> 主机 通信都很容易。这种方式设置上类似vmware的`host-only`,qemu使用`tun/tap`设备在主机上增加一块虚拟网络设备(`tun0`),然后就可以象真实网卡一样配置
比如 ：

```sh
-net nic -net tap,ifname=tap1212 
```

这里是 安装    后改名为  tap1212

下图，配置 可以上外网，这种方式类似vmware host-only方式，只有配置后才能上网

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/_v_images/e4f03287e46f4574943ee4d9e1b8173f~noop.image)





## 3.创建硬盘镜像

```sh
qemu-img create -f qcow2 d:\vm\arm64\kylindisk.qcow2 40G
```

### 1. 安装镜像

```sh
qemu-system-aarch64.exe -m 8192 -cpu cortex-a72 -smp 8,sockets=4,cores=2 -M virt -bios d:\vm\arm64\QEMU_EFI.fd -net nic -net tap,ifname=tap1212  -device VGA -device nec-usb-xhci -device usb-mouse -device usb-kbd -drive if=none,file=d:\vm\arm64\kylindisk.qcow2,id=hd0 -device virtio-blk-device,drive=hd0 -drive if=none,file=d:\d\Kylin-Server-10-SP2-aarch64-Release-Build09-20210524.iso,id=cdrom,media=cdrom -device virtio-scsi-device -device scsi-cd,drive=cdrom  
```

![img](https://p3-sign.toutiaoimg.com/tos-cn-i-6w9my0ksvp/9c71464c165c4bb3884ed2a41ccc0f80~noop.image?_iz=58558&from=article.pc_detail&lk3s=953192f4&x-expires=1721441905&x-signature=hLNSTycW5sGAWETDxNJ3IzSCXnM%3D)

### 2. 启动镜像

```sh
qemu-system-aarch64.exe -m 8192 -cpu cortex-a72 -smp 8,sockets=4,cores=2 -M virt -bios d:\vm\arm64\QEMU_EFI.fd -net nic -net tap,ifname=tap1212  -device VGA -device nec-usb-xhci -device usb-mouse -device usb-kbd -drive if=none,file=d:\vm\arm64\kylindisk.qcow2,id=hd0 -device virtio-blk-device,drive=hd0  -device virtio-scsi-device -device scsi-cd,drive=cdrom  
```