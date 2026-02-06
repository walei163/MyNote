# MMC编号的设定说明

在SWA113I核心板的设计中，采用TF卡和emmc作为系统的启动设置。在物理硬件的设计上，TF卡对应的接口：`SDC0`，eMMC对应接口：`SDC2`

而默认T113-i的dtb配置中，`SDC2`是被关闭的，因此从TF卡启动时，Linux内核自动为其分配了`mmc0`的编号。

而当在DTB中打开了`SDC2`配置后，TF卡就自动被分配为：`mmc1`，eMMC被分配为：`mmc0`，使用时需要注意这一点。

另外，当mmc编号发生改变时，u-boot下的环境变量：`mmc_root`的值可能也需要跟着修改。默认其值为：

```sh
# fw_printenv mmc_root
mmc_root=/dev/mmcblk0p5
```

该项用于系统启动时从哪个mmc设备上加载rootfs文件系统。如果是从eMMC启动，因为内核编号分配为：`mmc0`，因此不需要修改。

但如果是从TF卡启动，则该值需要修改为：

```sh
# fw_printenv mmc_root
mmc_root=/dev/mmcblk1p5
```

这样才能确保Linux内核启动时加载TF卡上的文件系统，否则系统启动失败。

默认官方的分区为：

```sh
# fdisk -l
Found valid GPT with protective MBR; using GPT

Disk /dev/mmcblk0: 7733248 sectors, 3776M
Logical sector size: 512
Disk identifier (GUID): ab6f3888-569a-4926-9668-80941dcb40bc
Partition table holds up to 8 entries
First usable sector is 73728, last usable sector is 7733214

Number  Start (sector)    End (sector)  Size Name
     1           73728          108165 16.8M boot-resource
     2          108166          110213 1024K env
     3          110214          112261 1024K env-redund
     4          112262          147461 17.1M boot
     5          147462         2244613 1024M rootfs
     6         2244614         2246661 1024K dsp0
     7         2246662         2279429 16.0M private
     8         2279430         7733214 2662M UDISK
```



