# RK3568开发笔记-buildroot挂载overlay联合文件系统

![image-20250109111316126](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111316126.png)

![image-20250109111459321](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111459321.png)

![image-20250109111525637](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111525637.png)

![image-20250109111542313](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111542313.png)

```sh
#!/bin/sh
### BEGIN INIT INFO
# Provides:       mount-overlayfs
# Default-Start:  S
# Default-Stop:
# Description:    Mount Overlayfs
### END INIT INFO
 
case "$1" in
        start|"")
                if [ ! -d /home/data/upper ];then
                        mkdir -p /home/data/upper
                fi
                if [ ! -d /home/data/work ];then
                        mkdir -p /home/data/work
                fi
 
                #mount -n /dev/mmcblk0p6 /overlay/
                mount --bind -n / /overlay
                mount -t overlay overlay -o lowerdir=/overlay/,upperdir=/home/data/upper/,workdir=/home/data/work/ /mnt/
                mount -n /proc --move /mnt/proc
                pivot_root /mnt /mnt/overlay
                mount -n /overlay/dev --move /dev
                mount -n /overlay/tmp --move /tmp
                mount -n /overlay/run --move /run
                mount -n /overlay/mnt/ramfs --move /mnt/ramfs
                mount -n /overlay/sys --move /sys
                mount -n /overlay/home/user --move /home/user
                mount -n /overlay/mnt/sconf --move /mnt/sconf
                mount -n /overlay/home/data --move /home/data
                mount -n /overlay/overlay --move /overlay
                #mount -t overlay overlay -o lowerdir=/,upperdir=/userdata/upper,workdir=/userdata/work /
                ;;
        restart|reload|force-reload)
                # No-op
                ;;
        stop|status)
                # No-op
                ;;
        *)
                echo "Usage: start" >&2
                exit 3
                ;;
esac
```

![image-20250109111618647](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111618647.png)

![image-20250109111643745](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111643745.png)

![image-20250109111732715](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111732715.png)

![image-20250109111817260](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111817260.png)

![image-20250109111843951](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111843951.png)

![image-20250109111909852](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111909852.png)

![image-20250109111931025](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111931025.png)

![image-20250109111947868](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/RK3568相关/vx_images/image-20250109111947868.png)
