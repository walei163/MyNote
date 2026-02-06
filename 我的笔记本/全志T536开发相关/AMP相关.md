# AMP相关

T536大小核之间通讯实现：

## 一、Linux端配置：

### 1、编译好RTOS的二进制固件：

```sh
./build.sh rtos
```

如果需要配置RTOS，则：

```sh
./build.sh rtos menuconfig
```

如果编译无误，则编译后的结果在：

```sh
/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/bin/amp_rv0.bin
```

注意固件文件名：amp_rv0.bin需要和Linux下remoteproc中设置的一致，可以通过以下方式来查看remoteproc设置的固件名称：

```sh
# cat /sys/class/remoteproc/remoteproc0/firmware 
amp_rv0.bin
```

其默认设置在DTB文件：`sun55iw6p1.dtsi`中：

```c
e907_rproc: e907_rproc@1a00000 {
	reg = <0x0 0x1a00000 0x0 0x1000>,
		<0x0 0x01a02000 0x0 0x400>;
	compatible = "allwinner,e907-rproc";
	/* core clk depend on vdd-sys, if vol equal with 0.94v, set core clk as 600M,
		set parent clk as CLK_PLL_PERI0_600M,otherwise if vol equla with 0.92v,
		core clk set as 480m, set parent clk as CLK_PLL_PERI0_480M */
	core-clock-frequency = <480000000>;
	axi-clock-frequency = <240000000>;
	clocks = <&ccu CLK_PLL_PERI0_480M>, <&ccu CLK_RV_CORE>, <&ccu CLK_RV_TS>, <&ccu CLK_RV_CFG>, <&ccu CLK_E907_AXI>;
	clock-names = "input", "core-gate", "rv-ts-gate", "rv-cfg-gate", "axi-div";
	resets = <&ccu RST_BUS_RV_CFG>, <&ccu RST_BUS_RV_CORE>, <&ccu RST_BUS_RV_SY>;
	reset-names = "rv-cfg-rst", "core-rst", "rv-sys-rst";
	firmware-name = "amp_rv0.bin";
	reg-names = "rv-cfg", "rproc_wdt_reg";
	interrupts = <GIC_SPI 185 IRQ_TYPE_LEVEL_HIGH>,
				<GIC_SPI 190 IRQ_TYPE_LEVEL_HIGH>;
	interrupt-names = "rproc_wdt_irq", "rproc_ecc_irq";
	status = "disabled";
};
```

### 2、将固件上传到文件系统的`/lib/firmware`目录下：

```sh
# ll /lib/firmware/
total 1M     
-rw-------    1 root     root      252.5K Jan  1 01:35 amp_rv0.bin
```

### 3、启动remoteproc：

```sh
# echo start > /sys/class/remoteproc/remoteproc0/state
[12151.498728] remoteproc remoteproc0: powering up e907_rproc
[12151.505234] remoteproc remoteproc0: using internal firmware, skip checking here
[12151.513449] remoteproc remoteproc0: Booting fw image amp_rv0.bin, size 258568
[12151.521667] remoteproc remoteproc0: failed to load fw from partition riscv0
[12151.529617] remoteproc remoteproc0: failed to load fw from partition riscv0-r
[12151.537668] remoteproc remoteproc0: load fw from filesystem, filename: amp_rv0.bin
[12151.546208] sunxi-rproc 1a00000.e907_rproc: handle vendor resource, type: 129
[12151.554274] sunxi-rproc 1a00000.e907_rproc: can't add trace mem 'aw_trace_log', da: 0x60042180, len: 32768, please enable aw trace mem cfg!
[12151.568382] remoteproc remoteproc0: firmware version: UTS - Tue, 06 May 2025 12:06:23 +0800
[12151.568382] Compile Time - 12:06:23
[12151.581792] sunxi-rproc 1a00000.e907_rproc: boot address: 0x60000000
[12151.588978]  remoteproc0#vdev0buffer: assigned reserved memory node vdev0buffer@42400000
[12151.598442] virtio_rpmsg_bus virtio0: rpmsg host is online
[12151.604808] reg-virt-consumer reg-virt-consumer.3.auto: Failed to obtain supply 'aldo1': -517
[12151.614385]  remoteproc0#vdev0buffer: registered virtio0 (type 7)
[12151.621233] remoteproc remoteproc0: remote processor e907_rproc is now up
[12151.629169] reg-virt-consumer reg-virt-consumer.4.auto: Failed to obtain supply 'dldo1': -517
# [12151.639071] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12151.648709] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12151.657602] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12151.667066] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12151.676036] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12151.685529] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12151.694309] reg-virt-consumer reg-virt-consumer.0.auto: Failed to obtain supply 'dcdc1': -517
[12151.704076] reg-virt-consumer reg-virt-consumer.1.auto: Failed to obtain supply 'dcdc2': -517
[12151.713820] reg-virt-consumer reg-virt-consumer.2.auto: Failed to obtain supply 'dcdc3': -517
[12151.919133] virtio_rpmsg_bus virtio0: creating channel sunxi,rpmsg_ctrl addr 0x400
[12151.928146] reg-virt-consumer reg-virt-consumer.3.auto: Failed to obtain supply 'aldo1': -517
[12151.937933] reg-virt-consumer reg-virt-consumer.4.auto: Failed to obtain supply 'dldo1': -517
[12151.947520] virtio_rpmsg_bus virtio0: creating channel rpbuf-service addr 0x401
[12151.956094] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12151.965578] virtio_rpmsg_bus virtio0: creating channel sunxi,rpmsg_heartbeat addr 0x402
[12151.974634] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12151.983517] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12151.992977] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12152.001866] sunxi:sound-mach:[WARN]: 372 asoc_simple_parse_ucfmt(): set data late to default
[12152.011337] sunxi:sound-mach:[ERR]: 488 simple_parse_of(): simple_dai_link_of failed
[12152.020106] reg-virt-consumer reg-virt-consumer.0.auto: Failed to obtain supply 'dcdc1': -517
[12152.029824] reg-virt-consumer reg-virt-consumer.1.auto: Failed to obtain supply 'dcdc2': -517
[12152.039563] reg-virt-consumer reg-virt-consumer.2.auto: Failed to obtain supply 'dcdc3': -517
```

如果启动成功，可以通过以下方式来查阅启动后的运行状态：

```sh
# cat /sys/class/remoteproc/remoteproc0/state
running
```

此时将会在`/dev`目录下创建一个字符型设备：

```sh
# ll /dev/rp*
crw-------    1 root     root      240,   0 Jan  1 05:12 /dev/rpmsg_ctrl-e907_rproc@1a00000
```

同时在系统的`/sys/class/rpmsg`目录下，也可以看到该设备：

```sh
# ll /sys/class/rpmsg/rpmsg_ctrl-e907_rproc\@1a00000/
total 0      
--w-------    1 root     root        4.0K Jan  1 05:13 clear
--w-------    1 root     root        4.0K Jan  1 05:13 close
-r--r--r--    1 root     root        4.0K Jan  1 05:13 dev
lrwxrwxrwx    1 root     root           0 Jan  1 05:13 device -> ../../../virtio0.sunxi,rpmsg_ctrl.-1.1024
--w-------    1 root     root        4.0K Jan  1 05:20 open
drwxr-xr-x    2 root     root           0 Jan  1 05:13 power
--w-------    1 root     root        4.0K Jan  1 05:13 reset
lrwxrwxrwx    1 root     root           0 Jan  1 05:13 subsystem -> ../../../../../../../../../../../class/rpmsg
-rw-r--r--    1 root     root        4.0K Jan  1 05:13 uevent
```

### 4、操作RPMSG设备（未测试）：

#### 4.1. 控制设备接口：`rpmsg_ctrl-e907_rproc@1a00000`的说明：

`rpmsg_ctrl-e907_rproc@1a00000`是大小核之间基于rpmsg通讯的一个控制接口，其具体代码实现见：

```sh
jason@xubuntu:/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/drivers/rpmsg
$ ll
总计 116K
-rw-r--r-- 1 jason 1005  34K 12月 26 21:30 aw_virtio_rpmsg_bus.c
-rw-r--r-- 1 jason 1005 1.6K 12月 26 21:30 Kconfig
-rw-r--r-- 1 jason 1005  447 12月 26 21:30 Makefile
-rw-r--r-- 1 jason 1005  15K 12月 26 21:30 rpmsg_client.c
-rw-r--r-- 1 jason 1005 3.6K 12月 26 21:30 rpmsg_heartbeat.c
-rw-r--r-- 1 jason 1005  25K 12月 26 21:30 rpmsg_master.c
-rw-r--r-- 1 jason 1005 2.9K 12月 26 21:30 rpmsg_master.h
-rw-r--r-- 1 jason 1005  13K 12月 26 21:30 rpmsg_notify.c
-rw-r--r-- 1 jason 1005 3.8K 12月 26 21:30 rpmsg_perf.c
```

里面核心的代码是：`rpmsg_master.c`和`rpmsg_client.c`。可以通过研读这两个内核驱动代码来了解RPMSG通讯的过程。

头文件：`rpmsg_master.h`中包含了ioctl操作的宏定义：

```c
/**
 * RPMSG_CREATE_EPT_IOCTL:
 *     Create the endpoint specified by info.name,
 *     updates info.id.
 * RPMSG_DESTROY_EPT_IOCTL:
 *     Destroy the endpoint specified by info.id.
 * RPMSG_REST_EPT_GRP_IOCTL:
 *     Destroy all endpoint belonging to info.name
 * RPMSG_DESTROY_ALL_EPT_IOCTL:
 *     Destroy all endpoint
 * RPMSG_CREATE_AF_EPT_IOCTL:
 *     Create the endpoint specified by info.name,
 *     updates info.id.
 *     It will automatically destroy the endpoint when closing file descriptor.
 */
#define RPMSG_CREATE_EPT_IOCTL	_IOW(0xb5, 0x1, struct rpmsg_endpoint_info)
#define RPMSG_DESTROY_EPT_IOCTL	_IO(0xb5, 0x2)
#define RPMSG_REST_EPT_GRP_IOCTL	_IO(0xb5, 0x3)
#define RPMSG_DESTROY_ALL_EPT_IOCTL	_IO(0xb5, 0x4)
#define RPMSG_CREATE_AF_EPT_IOCTL	_IOW(0xb5, 0x5, struct rpmsg_endpoint_info)

#define RPMSG_EPTDEV_DELIVER_PERF_DATA_IOCTL _IOW(0xb6, 0x1, int)
```

因此，通过打开设备：`rpmsg_ctrl-e907_rproc@1a00000`并对其进行操作，可以实现与远程e907小核进行通讯。

#### 4.2. 测试程序代码：

一个应用层的测试程序例子如下：

```c
#include <linux/rpmsg.h>
# 创建端点
int fd;
struct rpmsg_ept_info info;
char ept_dev_name[32];
strcpy(info.name, "test");
info.id = 0xfffff; # id由itctl进行更新
fd = open(ctrl_dev, O_RDWR);
ret = ioctl(fd, RPMSG_CREATE_EPT_IOCTL, &info);
# 当ioctl返回值==0时，端点已经创建成功，设备节点会出现在/dev/rpmsg%d(=info.id)下
close(fd);
#读写设备节点
snprintf(ept_dev_name, 32, "/dev/rpmsg%d", info.id);
fd = open(ept_dev_name, O_RDWR);
write,read,poll...
close(fd);
# 关闭节点
fd = open(ctrl_dev, O_RDWR);
ret = ioctl(fd, RPMSG_DESTROY_EPT_IOCTL, &info);
close(fd);
```

该代码的链接网址：https://blog.csdn.net/thisway_diy/article/details/128446126

另rpmsg使用参考：https://wx.comake.online/doc/doc/SigmaStarDocs-SSD238X-Android-20240712/platform/MCU/riscv_rpmsg.html

以及：https://mbd.baidu.com/newspage/data/landingsuper?id=1827182860853756766&wfr=spider&for=pc&third=baijiahao&baijiahao_id=1827182860853756766&c_source=duedge&p_tk=2455yaLHuwrkCpmvcf5WzA8ZEAScdDsZLvTQG8mgTVXEwgONKXRj%2FBC7hsxjF1rLT7za6icdTu1gHRrNNj4EVw%2B6RyCbUWGYuNSOr2EEQy1FaXpId3J%2F0h6B2s4EFebMym9Qci8uYfZVGAvxJdkHFTjedVq47z19wy4GCg6SJbJDMAg%3D&p_timestamp=1746154038&p_sign=b2076aeb50f73929d1bed04e1555b86e&p_signature=4c21b5263ac86a8b072231673e1dcd26&__pc2ps_ab=2455yaLHuwrkCpmvcf5WzA8ZEAScdDsZLvTQG8mgTVXEwgONKXRj%2FBC7hsxjF1rLT7za6icdTu1gHRrNNj4EVw%2B6RyCbUWGYuNSOr2EEQy1FaXpId3J%2F0h6B2s4EFebMym9Qci8uYfZVGAvxJdkHFTjedVq47z19wy4GCg6SJbJDMAg%3D|1746154038|4c21b5263ac86a8b072231673e1dcd26|b2076aeb50f73929d1bed04e1555b86e