# 从零开始制作一个TF启动卡

## 1、先制作一个4GB大小的虚拟映像磁盘：

```sh
sudo dd if=/dev/zero of=tf_card.img bs=8M count=512
```

---

## 2、用`sgdisk`将DCA-173的eMMC的分区恢复到虚拟磁盘中：

```sh
sudo sgdisk --load-backup=backup_gpt.bin tf_card.img
```

如果操作成功，则可以查看一下虚拟磁盘的分区信息：

```sh
$ sudo sgdisk -p tf_card.img 
[sudo] jason 的密码：
Disk emmc_dca173_v10_20251222.img: 8388608 sectors, 4.0 GiB
Sector size (logical): 512 bytes
Disk identifier (GUID): AB6F3888-569A-4926-9668-80941DCB40BC
Partition table holds up to 8 entries
Main partition table begins at sector 2 and ends at sector 3
First usable sector is 73728, last usable sector is 8388574
Partitions will be aligned on 2-sector boundaries
Total free space is 655360 sectors (320.0 MiB)

Number  Start (sector)    End (sector)  Size       Code  Name
   1           73728          108165   16.8 MiB    0700  boot-resource
   2          108166          110213   1024.0 KiB  0700  env
   3          110214          112261   1024.0 KiB  0700  env-redund
   4          112262          147461   17.2 MiB    0700  boot
   5          147462         2244613   1024.0 MiB  8300  rootfs
   6         2244614         2246661   1024.0 KiB  0700  dtb
   7         2246662         2279429   16.0 MiB    0700  private
   8         2279430         7733214   2.6 GiB     8300  UDISK

```

---

## 3、此时可以将虚拟磁盘挂载到Linux系统下，便于后续的操作：

```sh
$ sudo losetup --show -fP emmc_dca173_v10_20251222.img 
/dev/loop0
```

可以看到，虚拟磁盘被挂载到了Linux系统的`/dev/loop0`的还回设备上。接下来可以直接对这个还回设备进行操作了。

可以查看一下/dev/loop0的分区信息：

```sh
$ sudo sgdisk -p /dev/loop0
Disk /dev/loop0: 8388608 sectors, 4.0 GiB
Sector size (logical/physical): 512/512 bytes
Disk identifier (GUID): AB6F3888-569A-4926-9668-80941DCB40BC
Partition table holds up to 8 entries
Main partition table begins at sector 2 and ends at sector 3
First usable sector is 73728, last usable sector is 8388574
Partitions will be aligned on 2-sector boundaries
Total free space is 655360 sectors (320.0 MiB)

Number  Start (sector)    End (sector)  Size       Code  Name
   1           73728          108165   16.8 MiB    0700  boot-resource
   2          108166          110213   1024.0 KiB  0700  env
   3          110214          112261   1024.0 KiB  0700  env-redund
   4          112262          147461   17.2 MiB    0700  boot
   5          147462         2244613   1024.0 MiB  8300  rootfs
   6         2244614         2246661   1024.0 KiB  0700  dtb
   7         2246662         2279429   16.0 MiB    0700  private
   8         2279430         7733214   2.6 GiB     8300  UDISK
```



---

## 4、刷写SPL：

```sh
sudo dd if=boot0_sdcard.fex of=/dev/loop0 seek=16 conv=notrunc bs=512
```

---

## 5、刷写u-boot：

```sh
sudo dd if=boot_package.fex of=/dev/loop0 seek=32800 conv=notrunc bs=512
```

---

## 6、刷写ENV：

```sh
sudo dd if=uboot_env.img of=/dev/loop0p2
sudo dd if=uboot_env-redund.img of=/dev/loop0p3
```

---

## 7、刷写boot-resource：

```sh
sudo dd if=boot-resource.fex of=/dev/loop0p1 bs=1M
```

---

## 8、刷写Linux内核映像：

```sh
sudo dd if=boot.img of=/dev/loop0p4 bs=1M
```

---

## 9、刷写dtb设备树映像：

```sh
sudo dd if=dca173_v1.0.dtbo of=/dev/loop0p6
```

---

## 10、格式化rootfs文件系统分区，和UDISK分区：

```sh
sudo mkfs.ext4 /dev/loop0p5
sudo mkfs.ext4 /dev/loop0p8
```

---

## 11、挂载rootfs分区，并将rootfs压缩包文件解压到rootfs分区下：

```sh
sudo mount /dev/loop0p5 /mnt
sudo tar xzvf rootfs_dca173_v10_armhf_20251221.tar.gz -C /mnt
```

> [!note]
>
> **解压完成后，别忘记还要卸载rootfs分区：**
>
> ```sh
> sudo umount /mnt
> ```

---

## 12、这一步骤可选：

如果需要的话，挂载UDSIK分区，并将压缩包：`sanway_dca173_v1.0_20251221.tar.gz`解压到该分区下：

```sh
sudo mount /dev/loop0p8 /mnt
sudo tar xzvf sanway_dca173_v1.0_20251221.tar.gz -C /mnt
```

同样的，解压缩完成后也记得要卸载该分区：

```sh
sudo umount /mnt
```

---

至此，操作完成，此时可以卸载虚拟磁盘：

```sh
sudo losetup -d /dev/loop0
```

