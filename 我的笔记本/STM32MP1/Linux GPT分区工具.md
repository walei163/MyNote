linux GPT分区工具
原创 Vernon2csdn 最后发布于2018-07-30 14:52:10 阅读数 954 收藏
展开

事出有因，为什么会写这篇博客？由于需要制作SD/TF启动卡，ARM linux指定采用GPT分区，所以在ubuntu需要一款能够进行GPT分区工具。

目前能够进行GPT分区的工具有gdisk、parted。

但是我需要对GPT某一个分区进行指定PARTUUID/PARTLABEL，所以此文章采用gdisk进行GPT分区。

 

gdisk工具使用教程：

1）进入gdisk交互界面

# sudo gdisk /dev/sdc

2）创建一个新的 空 GUID 分区表（GPT）

    Command (? for help): o
    This option deletes all partitions and creates a new protective MBR.
    Proceed? (Y/N): Y

2）修改扇区为64-sector对齐

    Command (? for help): x
     
    Expert command (? for help): d
    Partitions will begin on 2048-sector boundaries.
     
    Expert command (? for help): l
    Enter the sector alignment value (1-65536, default = 2048): 64
     
    Expert command (? for help): d
    Partitions will begin on 64-sector boundaries.
     
    Expert command (? for help): m

3）新创建一个分区

    Command (? for help): n
    Partition number (1-128, default 1): 
    First sector (34-15407070, default = 64) or {+-}size{KMGTP}: 
    Last sector (64-15407070, default = 15407070) or {+-}size{KMGTP}: 8192
    Current type is 'Linux filesystem'
    Hex code or GUID (L to show codes, Enter = 8300): 
    Changed type of partition to 'Linux filesystem'
     
    Command (? for help): p
    Disk /dev/sdc: 15407104 sectors, 7.3 GiB
    Logical sector size: 512 bytes
    Disk identifier (GUID): 3C2DB04C-244C-4472-B248-A2C6927CE4B4
    Partition table holds up to 128 entries
    First usable sector is 34, last usable sector is 15407070
    Partitions will be aligned on 64-sector boundaries
    Total free space is 15398908 sectors (7.3 GiB)
     
    Number  Start (sector)    End (sector)  Size       Code  Name
       1              64            8192   4.0 MiB     8300  Linux filesystem
     

4）修改某一个分区的名字（即PARTLABEL）

    Command (? for help): c
    Partition number (1-10): 1
    Enter name: loader

5）修改某一个分区的PARTUUID

    Command (? for help): x
     
    Expert command (? for help): c
    Partition number (1-10): 9
    Enter the partition's new unique GUID ('R' to randomize): 614e0000-0000-4b53-8000-1d28000054a9
    New GUID is 614E0000-0000-4B53-8000-1D28000054A9
    Expert command (? for help): m
    Command (? for help): i
    Partition number (1-10): 9
    Partition GUID code: 0FC63DAF-8483-4772-8E79-3D69D8477DE4 (Linux filesystem)
    Partition unique GUID: 614E0000-0000-4B53-8000-1D28000054A9
    First sector: 368640 (at 180.0 MiB)
    Last sector: 7708671 (at 3.7 GiB)
    Partition size: 7340032 sectors (3.5 GiB)
    Attribute flags: 0000000000000000
    Partition name: 'rootfs'

6）保存GPT分区表并退出

    Command (? for help): w
     
    Final checks complete. About to write GPT data. THIS WILL OVERWRITE EXISTING
    PARTITIONS!!
     
    Do you want to proceed? (Y/N): Y
    OK; writing new GUID partition table (GPT) to /dev/sdc.
    The operation has completed successfully.

 

额外小知识：

PARTUUID与UUID的区别？

答：

PARTUUID/PARTLABEL identifies a GPT partition. UUID/LABEL identifies a filesystem.

PARTUUID/PARTLABEL have the advantage that they don't change if your reformat the partition with another filesystem. It's also useful if you don't have a filesystem on the partition (or use LUKS, which doesn't support LABELs).

 

参考链接：

[SOLVED] What is the difference between UUID and PARTUUID?

[Solved] How to change a partitions PARTUUID ?
————————————————
版权声明：本文为CSDN博主「Vernon2csdn」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/QQ2010899751/java/article/details/81284387