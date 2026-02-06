[小僧的功课](https://www.cnblogs.com/newjiang)

## [openwrt 切换overlay文件系统为根文件系统](https://www.cnblogs.com/newjiang/p/11711719.html)

http://blog.chinaunix.net/uid-27057175-id-4584360

openwrt的overlayfs 通过/etc/preinit调用 /sbin/mount_root来完成rootfs的切换挂载和overlay

所以研究下openwrt工程中mount_root的实现。源码目录 trunk/build_dir/target-arm_cortex-a9+vfpv3_uClibc-0.9.33.2_eabi/fstools-2014-06-22/mount_root.c
通过CMakeList.txt 的依赖关系，得知mount_root的最终实现是overlay.c的mount_overlay()

**代码****流****程：**

> 1.overlay_mount_fs()
> 2.mount_extroot();
> 3.mount_move("/tmp", "", "/overlay")
> 4.fopivot("/overlay", "/rom")
>  4.1 find_filesystem("overlay") 
>  4.2 mount_f("overlayfs:/overlay", "/mnt", "overlayfs", MS_NOATIME, "lowerdir=/,upperdir=/overlay")
>  4.3 pivot("/mnt", "/rom")

**翻译为脚本：**

> 1.
> mkdir /tmp/overlay
> mount -n -t jffs2 /dev/mtdblock3 -o rw,noatime,mode=0755 /tmp/overlay
> 2.检查无 /tmp/overlay/sbin/block，ret=-1
> 3.mount("/tmp/overlay", "/overlay", 0, MS_NOATIME | MS_MOVE, 0);
> 等效
>  mount -n -t NULL /tmp/overlay -o noatime,--move /overlay
> mount -n /tmp/overlay -o noatime,--move /overlay
> 4.
>   4.1 /proc/filesystem 下确认是否支持overlay
>   4.2 
>     mount -n -t overlayfs overlayfs:/overlay -o rw,noatime,lowerdir=/,upperdir=/overlay /mnt
>   4.3 
>     mount -n /proc -o noatime,--move /mnt/proc
>     pivot_root /mnt /mnt/rom
>     mount -n /rom/dev -o noatime,--move /dev
>     mount -n /rom/tmp -o noatime,--move /tmp
>     mount -n /rom/sys -o noatime,--move /sys
>     mount -n /rom/overlay -o noatime,--move /overlay

**提炼一下，核心的mount jffs2分区并实现overlayfs的****脚本就是：**

> mount -n -t jffs2 /dev/mtdblock3 -o rw,noatime,mode=0755 /overlay
> mount -n -t overlayfs overlayfs:/overlay -o rw,noatime,lowerdir=/,upperdir=/overlay /mnt
> mount -n /proc -o noatime,--move /mnt/proc
> pivot_root /mnt /mnt/rom 
> mount -n /rom/dev -o noatime,--move /dev
> mount -n /rom/tmp -o noatime,--move /tmp
> mount -n /rom/sys -o noatime,--move /sys
> mount -n /rom/overlay -o noatime,--move /overlay
> exit 0

不想用openwrt的init，把上面的脚本放到 /etc/preinit开头。