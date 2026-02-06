# 思创FUXI板卡文件系统修改

# 1.设置环境变量从TF卡启动

```sh
setenv dtb_file_name "fx6evb_SD_burn.dtb"
setenv kernel_file_name "uImage"
setenv mmc_tf_dev "0"
setenv mmc_load_kernel 'fatload mmc ${mmc_tf_dev} 0x08000000 ${kernel_file_name}; cp.b 0x08000000 ${load_kernel_addr} ${filesize}'
setenv mmc_load_dtb 'fatload mmc ${mmc_tf_dev} 0x08000000 ${dtb_file_name}; cp.b 0x08000000 ${load_fdt_addr} ${filesize}'
setenv boot_tf_card 'mmc dev ${mmc_tf_dev}; run mmc_load_kernel; run mmc_load_dtb; bootm ${load_kernel_addr}'
setenv bootcmd 'if test -e mmc 0:1 boot_from_sd; then run boot_tf_card; else run os_load; fi;'
saveenv
```

> [!note]
>
> 从客户寄过来的核心板已经烧写了系统，为了从TF卡启动需要设置以上uboot环境变量，设置好环境变量后在tf卡的第一个fat32分区创建boot_from_sd文件，即可从TF卡加载内核，设备树了。如果需要从TF卡加载文件系统，则需要设备树配置bootargs从TF卡加载。

## 批量更新SPI Flash（从TF卡启动）

```sh
1.将上述环境变量设置好后，从SPI Flash启动

2.将环境变量分区的数据dd出来：
dd if=/dev/mtdblock4 of=bootload_env.img

3.将导出的uboot环境变量分区的数据写到新的板卡上即可

这样更新完，在不改变板卡原生uboot和内核的情况下即可从TF卡启动
```

# 2.文件系统的优化

## 1.优化profile,便于知晓当前所在路径

```sh\
if [ "$PS1" ]; then
        if [ "`id -u`" -eq 0 ]; then
                export PS1='\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\# '
        else
                export PS1='\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
        fi
fi

alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
```

## 2.优化udev,支持tf卡/u盘的自动挂载和更新

**/usr/bin/block-mount.sh内容：**

```sh
#!/bin/sh

# This script is called from our systemd unit file to mount or unmount
# a USB drive.


LOG_FILE="/var/log/udev_mount.log"
MOUNT="/usr/bin/systemd-mount"

usage() {
    echo "Usage: $0 {add|remove} device_name (e.g. sdb1)" >> LOG_FILE
    exit 1
}

if [[ $# -ne 2 ]]; then
    usage
fi

ACTION=$1
DEVBASE=$2
DEVICE="/dev/${DEVBASE}"

# See if this drive is already mounted, and if so where
MOUNT_POINT=$(/bin/mount | /bin/grep ${DEVICE} | /usr/bin/awk '{ print $3 }')

echo "Check new device $DEVICE " 
do_mount()
{
：
    echo "mount point: $MOUNT_POINT"
    if [ -n "$MOUNT_POINT" ]; then
        echo "Warning: ${DEVICE} is already mounted at ${MOUNT_POINT}"
        exit 1
    fi

    fsType=$(/sbin/blkid -o udev $DEVICE  | grep "ID_FS_TYPE" | cut -d'=' -f2)
    if [ -z "$fsType" ]
    then
        echo "current part $DEVICE without filesystem!"
        exit 1 
    fi

    # Get info for this drive: $ID_FS_LABEL, $ID_FS_UUID, and $ID_FS_TYPE
    eval $(/sbin/blkid -o udev ${DEVICE})

    echo "Mount point: ${MOUNT_POINT}"
    MOUNT_POINT="/media/$DEVBASE"
    /bin/mkdir -p ${MOUNT_POINT}
    chmod 777 ${MOUNT_POINT}

    # Global mount options
    OPTS="rw,relatime,user,async"

    case $ID_FS_TYPE in
    vfat|fat)
        OPTS+=",gid=100,umask=000,shortname=mixed,utf8=1,flush"
        ;;
    
    exfat)
        OPTS="-o nonempty "
        MOUNT="/sbin/mount.exfat $OPTS"
        ;;

    *)
        ;;
    esac


    #if ! /bin/mount -o ${OPTS} ${DEVICE} ${MOUNT_POINT}; then
    #    echo "Error mounting ${DEVICE} (status = $?)"
    #    /bin/rmdir ${MOUNT_POINT}
    #    exit 1
    #fi

    #ubuntu18 of after,must use systemd-mount and systemd-umount operate block device
    $MOUNT  ${DEVICE}  $MOUNT_POINT    

    echo "**** Mounted ${DEVICE} at ${MOUNT_POINT} ****"
    if [ -x $MOUNT_POINT/sanway/auto_update.sh ]; then
        logger "##sanway## --auto update-- Start......"
        cd $MOUNT_POINT/sanway
        #systemctl start auto_update@${DEVBASE}.service 
        ./auto_update.sh &
        logger "##sanway## --auto update-- End......"
    fi          
}

do_unmount()
{
    if [[ -z ${MOUNT_POINT} ]]; then
        echo "Warning: ${DEVICE} is not mounted"
    else
        systemd-umount  $MOUNT_POINT
        echo "**** Unmounted ${DEVICE}"
        rmdir $MOUNT_POINT  
    fi

}

case "${ACTION}" in
    add)
        do_mount
        ;;
    remove)
        do_unmount
        ;;
    *)
        usage
        ;;
esac

exit 0
```

**/lib/systemd/system/block-mount@.service**

```sh
[Unit]
Description=Mount block Drive on %i

[Service]
User=root

#RemainAfterExit=true
ExecStart=/usr/bin/block-mount.sh add %i
#ExecStop=/usr/bin/block-mount.sh remove %i

IgnoreSIGPIPE=no
```

**/etc/udev/rules.d/block-mount.rules**

```sh
KERNEL=="mmcblk0p[0-9]", SUBSYSTEMS=="block", ACTION=="add",TAG+="systemd", ENV{SYSTEMD_WANTS}="block-mount@%k.service"
KERNEL=="sd[a-z][0-9]", SUBSYSTEMS=="block", ACTION=="add", TAG+="systemd", ENV{SYSTEMD_WANTS}="block-mount@%k.service"

KERNEL=="sd[a-z][0-9]", SUBSYSTEMS=="block", ACTION=="remove",  RUN+="/usr/bin/block-mount.sh remove %k"
KERNEL=="mmcblk0p[0-9]", SUBSYSTEMS=="block", ACTION=="remove", RUN+="/usr/bin/block-mount.sh remove %k"
```

## 3.编译busybox，补充telnetd

```sh
编译选项：-mcpu=ck860fv -march=ck860v -mfdivdu -mhard-float -mdouble-float -static -mlittle-endian
```

**/lib/systemd/system/telnetd.service**

```sh
[Unit]
Description=OpenSSH server daemon
After=syslog.target network.target auditd.service

[Service]
Type=forking
ExecStart=/usr/sbin/telnetd 
TimeoutSec=0
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
root@CSG-FUXI:~# systemctl enable telnetd.service
Created symlink /etc/systemd/system/multi-user.target.wants/telnetd.service → /usr/lib/systemd/system/telnetd.service.

root@CSG-FUXI:~# systemctl start telnetd.service
```

## 4.修改tar命令

```sh
原生系统的tar命令不支持解压tar.gz ,将tar命令连接到新的busybox
ln -fs /bin/busybox_csky /bin/tar 
```

## 5.编译vsftpd,补充ftp

```sh
编译选项：-mcpu=ck860fv -march=ck860v
```

**/lib/systemd/system/vsftpd.service**

```sh
[Unit]
Description=OpenSSH server daemon
After=syslog.target network.target auditd.service

[Service]
Type=forking
ExecStart=/usr/sbin/vsftpd
TimeoutSec=0
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
systemctl enable vsftpd.service
Created symlink /etc/systemd/system/multi-user.target.wants/vsftpd.service → /usr/lib/systemd/system/vsftpd.service.
```

## 6.修改fstab 挂载一些内存

```sh
/dev/root / auto rw 0 1
proc                 /proc                proc       defaults              0  0
devpts               /dev/pts             devpts     mode=0620,gid=5       0  0
tmpfs                /run                 tmpfs      mode=0755,nodev,nosuid,strictatime 0  0
tmpfs                /var/volatile        tmpfs      defaults,size=64M     0  0
tmpfs                /media/ram           tmpfs      defaults,size=16M     0  0
```

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/https%3A%2F%2Fprod-files-secure.s3.us-west-2.amazonaws.com%2F288b3cd2-4757-4aa7-818d-902dc0344485%2F48c90e4a-93c4-4ca9-9db6-4ac8663137da%2Fimage.png)

原生fstab

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/FUXI项目相关_20220424/vx_images/https%3A%2F%2Fprod-files-secure.s3.us-west-2.amazonaws.com%2F288b3cd2-4757-4aa7-818d-902dc0344485%2Fd99d2779-2b92-4ec0-9547-a212df2482a5%2Fimage.png)

新的fstab

## 7.编译libgpiod，补充gpiod命令

```sh
编译参数：./configure --prefix=/usr/local/gpiod_csky --enable-tools CFLAGS="-mcpu=ck860fv -march=ck860v -mfdivdu -mhard-float -mdouble-float -static -mlittle-endian" CC=/opt/csky-gcc/bin/csky-abiv2-linux-gcc --host=arm-linux-gnueabihf
将gpio命令安(gpiodetect  gpiofind  gpioget  gpioinfo  gpiomon  gpioset)装在/usr/bin下面
```

## 8.补充uart测试命令

```sh
自己写的程序编译好放在/usr/bin下 分别为uartsend,uartrecv
```

## 9.补充can测试命令

```sh
放在/usr/bin下 分别为cansend,canrecv
```

## 10.补充pps工具

```sh
ppsctl  ppsfind  ppsldisc  ppstest  ppswatch
放在/usr/bin下 
```

## 11.补充paho-mqtt库

```sh
库放在了/usr/lib 命令(paho_c_pub  paho_cs_pub  paho_cs_sub  paho_c_sub  paho_c_version)放在了/usr/bin 
需要说明的是mqtt另外依赖了openssl库，所以也补充了openssl命令(系统内的openssl版本是1.1.1g,我移植的是1.1.1m，由于原生文件系统缺少openssl命令
这里只安装了命令，库还是原生的库)
```

## 12.补充sqlite3数据库

```sh
库放在了/usr/lib 命令(sqlite3)放在了/usr/bin 
```

## 13.补充mdio,spidev-cmd-16bit,spidev-fifo,taskset

```sh
mdio,spidev-cmd-16bit,spidev-fifo是以前编的测试程序

原生的taskset功能单一，将taskset链接到新的busybox
```