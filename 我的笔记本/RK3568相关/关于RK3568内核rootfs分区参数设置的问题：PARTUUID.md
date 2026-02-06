# 关于RK3568内核rootfs分区参数设置的问题：PARTUUID

在`RK3568`内核DTB中，rootfs是采用`PARTUUID`来指定分区的，如下所示：

```bash
bootargs = "earlycon=uart8250,mmio32,0xfe660000 console=ttyFIQ0 cgroup_enable=memory cgroup_memory=1 swapaccount=1 root=PARTUUID=614e0000-0000 rw rootwait";
```

如果要修改rootfs分区，或者更换指定的rootfs分区，则需要将新的rootfs分区，设置为DTB中指定的`PARTUUID`。

可以按照以下方式来实现：

1. `uuidgen`产生新的`UUID`：

```bash
$ uuidgen
85f72a0c-cf62-4a02-88e3-34bb75803f22
```

如果系统提示没有找到uuidgen，则可以安装一下：

```bash
$ sudo apt install uuid-runtime 
```

2. 使用`sgdisk`将新的`UUID`写入到对应的分区中：

```bash
sgdisk /dev/mmcblk0p8 -u 85f72a0c-cf62-4a02-88e3-34bb75803f22
```

> [!important]
>
> **1、`sgdisk`操作的分区，必须要处于卸载状态，如果已经加载，则不能直接对其进行操作。** 
>
> **2、rootfs官方出厂时默认的PARTUUID：`guid:   614e0000-0000-4b53-8000-1d28000054a9`** 
>
> **3、分区必须是`GPT`分区，才能按照以上描述进行操作。**

