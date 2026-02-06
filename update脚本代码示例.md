# update升级脚本script代码示例

2025-05-30：

脚本更新：下面是一个脚本代码的示例，后续可以在此基础上进行扩展，在这里作为脚本代码的备份：

```sh
#!/bin/sh

# set -e

KERNEL_VERSION=5.10.198

DIR_UPDATE=./update_files
DIR_UPDATE_BOOTLOADER=${DIR_UPDATE}/bootloader
DIR_UPDATE_KERNEL=${DIR_UPDATE}/kernel
DIR_UPDATE_ROOTFS=${DIR_UPDATE}/rootfs
DIR_UPDATE_DRIVERS=${DIR_UPDATE}/drivers
DIR_UPDATE_TEST=${DIR_UPDATE}/test

DIR_ETC=/etc
DIR_DEV=/dev

FILE_BOOTLOADER_BOOT0=${DIR_UPDATE_BOOTLOADER}/boot0_sdcard.fex
FILE_BOOTLOADER_UBOOT=${DIR_UPDATE_BOOTLOADER}/boot_package.fex
FILE_BOOTLOADER_GPT_BACKUP=${DIR_UPDATE_BOOTLOADER}/backup_gpt.bin
FILE_BOOTLOADER_ENV=${DIR_UPDATE_BOOTLOADER}/env.fex
FILE_KERNEL_IMG=${DIR_UPDATE_KERNEL}/boot.img
FILE_ROOTFS_IMG=${DIR_UPDATE_ROOTFS}/sanway-emmc-rootfs-10.2-qt5.12.-jdk8-python3.8.10_t536.ext4
FILE_RC_LOCAL=rc.local

TAR_BAG_FILE_SANWAY=${DIR_UPDATE}/sanway.tar.gz

#mmc变量默认定义
MMC_DEV=${DIR_DEV}/mmcblk0
TF_DEV=${DIR_DEV}/mmcblk1
MMC_COUNT=1

#mmc分区定义
#T536 eMMC官方默认的分区定义信息：
# Number  Start (sector)    End (sector)  Size Name
#      1           73728          139263 32.0M boot-resource
#      2          139264          172031 16.0M env
#      3          172032          368639 96.0M boot
#      4          368640          401407 16.0M private
#      5          401408         2498559 1024M rootfs
#      6         2498560         2502653 2047K riscv0
#      7         2502654        30535646 13.3G UDISK
arr_part_name=("boot-resource" "env" "boot" "private" "rootfs" "riscv0" "UDISK")

PART_NO_BOOT_RESOURCE=p1
PART_NO_ENV=p2
PART_NO_KERNEL=p3
PART_NO_ROOTFS=p5
PART_NO_RISCV=p6
PART_NO_UDISK=p7

PART_MMC_BOOT_RESOURCE=${MMC_DEV}${PART_NO_BOOT_RESOURCE}
PART_MMC_ENV=${MMC_DEV}${PART_NO_ENV}
PART_MMC_KERNEL=${MMC_DEV}${PART_NO_KERNEL}
PART_MMC_ROOTFS=${MMC_DEV}${PART_NO_ROOTFS}
PART_MMC_RISCV=${MMC_DEV}${PART_NO_RISCV}
PART_MMC_UDISK=${MMC_DEV}${PART_NO_UDISK}

#默认更新模式：all，全部更新
UPDATE_MODE_DEFAULT=all

shell_color_set()
{
        #red='\[\e[0;31m\]'
        red='\e[1;31m'
        RED='\e[1;31m'
        blue='\e[0;34m'
        BLUE='\e[1;34m'
        cyan='\e[0;36m'
        CYAN='\e[1;36m'
        green='\e[0;32m'
        GREEN='\e[1;32m'
        yellow='\e[1;33m'
        YELLOW='\e[1;33m'
        PURPLE='\e[1;35m'
        purple='\e[0;35m'

        error='\e[1;41m'
        warn='\e[1;43m'

        nc='\e[0m'

        # RED='\033[0;31m'
        # GREEN='\033[0;32m'
        # NC='\033[0m' # No Color                                                                  
}

shell_display_color_words()
{
        shell_color_set

        case $1 in
        yellow)
                echo -e -n "${yellow}$2${nc}"
                ;;
        YELLOW)
                echo -e "${YELLOW}$2${nc}"
                ;;
        cyan)
                echo -e -n "${cyan}$2${nc}"
                ;;
        CYAN)
                echo -e "${CYAN}$2${nc}"
                ;;
        green)
                #echo "\033[32m$2\033[0m"
                echo -e -n "${green}$2${nc}"
                ;;
        GREEN)
                #echo "\033[32m$2\033[0m"
                echo -e "${GREEN}$2${nc}"
                ;;

        red)
                #echo "\033[31m$2\033[0m"
                echo -e "${red}$2${nc}"
                ;;

        RED)
                #echo "\033[31m$2\033[0m"
                echo -e "${RED}$2${nc}"
                ;;
        blue)
                #echo "\033[34m$2\033[0m"
                echo -e "${blue}$2${nc}"
                ;;
        BLUE)
                #echo "\033[34m$2\033[0m"
                echo -e "${BLUE}$2${nc}"
                ;;
        white)
                #echo "\033[40;37m$2\033[0m"
                echo -e "\033[37m$2\033[0m"
                ;;
        error)
                echo -e "${error}$2${nc}"
                ;;
        warn)
                #echo -e "${warn}$2${nc}"
                echo -e "${YELLOW}$2${nc}"
                ;;
        *)
                echo "$2"
                ;;
        esac
}

#检测MTD的编号，可以输入1个参数，或者2个参数。
#采用2个参数是指：当出现搜索出2个以上的结果时，第2个参数用于排除多个选项，最终只保留一个结果。
detect_mtd()
{
        if [ $# -lt 1 ]
        then
                echo Error: No paramter! You must specify at least 1 paramter.
                exit 1
        fi

        if [ $# -eq 2 ]; then
                PARAM1=$1
                PARAM2=$2
                MTD_NAME=`cat /proc/mtd | grep "${PARAM1}" | grep -v "${PARAM2}" | cut -d ":" -f1`
        else
                PARAM1=$1
                MTD_NAME=`cat /proc/mtd | grep "${PARAM1}" | cut -d ":" -f1`
        fi

        # echo param1: ${PARAM1}        
        if [ -z ${MTD_NAME} ]; then
                echo Cannot find ${PARAM1} partition!
                exit 1                
        fi

        echo ${PARAM1} partition name: ${MTD_NAME}

        MTD_NUMBER=`echo ${MTD_NAME} | cut -d "d" -f2`
        MTD_BLOCK_NAME=${DIR_DEV}/mtdblock${MTD_NUMBER}

        echo ${PARAM1} block partition: ${MTD_BLOCK_NAME}
}

check_file_exist()
{
        if [ $# -ne 1 ]
        then
                echo Error: No paramter! You must specify 1 paramter.
                exit 1
        fi

        if [ ! -e $1 ]
        then                
                echo File: $1 is not exist!
                exit 1
        fi

        return 0
}

check_directory_exist()
{
        if [ $# -ne 1 ]
        then
                echo Error: No paramter! You must specify 1 paramter.
                exit 1
        fi

        if [ ! -d $1 ]; then
                echo DIR: $1 is not exist!
                exit 1
        fi

        #echo DIR: $1 is OK.
        return 0
}

#复制文件：需要2个参数$1和$2
update_files()
{
        check_directory_exist $2
        check_file_exist $1        

        echo Copying file: $1 to $2 ...
        cp -raf $1 $2

        if [ $? -ne 0 ]; then
                shell_display_color_words error "Copying file $1 failed."
                exit 1
        fi

        shell_display_color_words warn "Copy file $1 to $2 suceeded!"
        echo ""
}

#复制文件：需要2个参数$1和$2
update_tar_bag()
{                                                            
        check_directory_exist $2
        check_file_exist $1        

        echo Extracting TAR bag file: $1 to dir: $2 ...
        tar xzvf $1 -C $2

        if [ $? -ne 0 ]; then
                shell_display_color_words error "Extracting TAR bag file $1 to dir: $2 failed."
                exit 1
        fi

        shell_display_color_words warn "Extracting TAR bag file $1 to dir: $2 suceeded!"
        echo ""
}

check_block_device_exist()
{
        if [ $# -ne 1 ]
        then
                echo Error: No paramter! You must specify 1 paramter.
                exit 1
        fi

        if [ ! -b $1 ]; then
                echo Device block: $1 is not exist!
                exit 1
        fi

        #echo Device block: $1 is OK.
        return 0
}

#采用dd命令烧写eMMC分区
#参数1为需要烧写的文件名，参数2为eMMC分区编号，如mmcblk0p3等
erase_and_write_emmc()
{
        if [ $# -ne 2 ]
        then
                echo Error: No paramter! You must specify 2 paramter.
                exit 1
        fi

        check_block_device_exist $2
        check_file_exist $1
        
        echo Writing $1 to $2 ...
        echo "dd if=$1 of=$2 bs=1M"

        dd if=$1 of=$2 bs=1M

        if [ $? -ne 0 ]; then                
                shell_display_color_words error "Writing $1 failed."
                exit 1
        fi
        
        shell_display_color_words GREEN "Writing $1 to $2 succeeded!"
        echo ""
        return 0
}

#采用dd命令烧写eMMC分区，且dd命令需要带seek偏移量
#参数1为需要烧写的文件名，参数2为eMMC分区编号，如mmcblk0p3等，参数3为seek偏移量
erase_and_write_emmc_seek()
{
        local DD_BS=512
        local DD_CONV=notrunc

        if [ $# -ne 3 ]
        then
                echo "Error: No paramter! You must specify 3 paramters."
                echo "Usage: $0 <file> <block_device> <seek_offset>"
                exit 1
        fi

        check_block_device_exist $2
        check_file_exist $1
        
        echo Writing $1 to $2 with seek offset: $3 ...
        echo "dd if=$1 of=$2 seek=$3 conv=${DD_CONV} bs=${DD_BS}"

        dd if=$1 of=$2 seek=$3 conv=${DD_CONV} bs=${DD_BS}

        if [ $? -ne 0 ]; then                
                shell_display_color_words error "Writing $1 to $2 with seek offset: $3 failed."
                exit 1
        fi
        
        shell_display_color_words warn "Writing $1 to $2 with seek offset: $3 succeeded!"
        echo ""
        return 0
}

detect_rootfs_partition()
{
        local PARTITITON_ROOTS=$(cat /proc/cmdline | grep "root=" | awk -F= '{print $6}' | awk '{print $1}')
        echo Current loading rootfs partition: ${PARTITITON_ROOTS}

        #以下判断写法是判断一个字符串是否包含子字符串
        if [[ "${PARTITITON_ROOTS}" == *"${MMC_DEV}"* ]]; then
        #if [[ "${PARTITITON_ROOTS}" == *"mmcblk0"* ]]; then
                shell_display_color_words error "Current file system is loading by: EMMC."
                shell_display_color_words error "We can not update rootfs, you must restart system in TF Card mode, and rerun script: $0"
                exit 1  
        elif [[ "${PARTITITON_ROOTS}" == *"${TF_DEV}"* ]]; then
                echo "Current file system is loading by: TF_CARD."
                shell_display_color_words GREEN "Checking rootfs OK, we can update rootfs."
        else
                shell_display_color_words error "Current file system is loading UNKNOWN, We can not update rootfs."
                exit 1
        fi                    
}

show_system_menu()
{
        arr_update_mode=("all" "kernel" "rootfs" "spl" "uboot_env" "gpt")

        echo "==================================================================="
        echo "Welcome to Sanway Linux Update Script, please choose update mode:"
        echo "1. all - Update all bootloader/kernel/rootfs/spl/uboot_env/gpt to eMMC device"
        echo "2. kernel - Update kernel to eMMC/TF_CARD only"
        echo "3. rootfs - Update rootfs to eMMC only"
        echo "4. spl - Update spl to eMMC/TF_CARD only"
        echo "5. uboot_env - Update uboot_env to eMMC/TF_CARD only"
        echo "6. gpt - Backup or restore eMMC/TF_CARD GPT partition table"
        echo ""

        echo -n "Please input your choice: [1-6] " && read items
        if [ -z ${items} ]; then
                shell_display_color_words error "You did not input any choice, please re-run script: $0"
                exit 1
        fi

        UPDATE_MODE=${arr_update_mode[$((items-1))]}
}

detect_input_paramters()
{
        # echo Paramter is: $#
        if [ $# -lt 1 ]; then
                # UPDATE_MODE=${UPDATE_MODE_DEFAULT}
                # echo You are not inputting paramters, we use default update mode: ${UPDATE_MODE}
                show_system_menu
        else
                UPDATE_MODE=$1                
        fi

	# arr_update_mode=("all" "kernel" "rootfs" "spl" "uboot_env" "gpt")

	##遍历数组
	# for item in "${arr_update_mode[@]}"; do
	    ##echo "$item"
	#     if [ "$item" = "${UPDATE_MODE}" ]; then
	        # echo Current update mode: ${UPDATE_MODE}
		# echo ""
		# return 0
	#     fi
	# done

        case ${UPDATE_MODE} in
        all)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        kernel)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        rootfs)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        spl)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        uboot_env)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        gpt)
                echo Current update mode: ${UPDATE_MODE}
                echo ""
                return 0
                ;;
        *)
                shell_display_color_words error "Update mode select error: ${UPDATE_MODE}"
                echo You should input paramter such as: all/kernel/rootfs/spl/uboot_env/gpt, etc.
                echo The default update mode is: ${UPDATE_MODE_DEFAULT}
                exit 1
                ;;
        esac
}

update_mmc_choice()
{
	echo -n "请选择想要更新哪个MMC设备的${UPDATE_MODE}镜像分区: [0 - eMMC/1 - TF_CARD] " && read items

	if [ -z ${items} ]; then
		shell_display_color_words error "你没有输入任何选项，请重新运行脚本！"
		exit 1
	elif [ "$items" = "0" ]; then
		MMC_CHOICE_DEV=${MMC_DEV}		
	elif [ "$items" = "1" ]; then
		MMC_CHOICE_DEV=${TF_DEV}
	else
		shell_display_color_words error "你输入的选项错误，请重新运行脚本！"
		exit 1
	fi
}

update_spl_boot0()
{
	if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then	
		update_mmc_choice
		MMC_DEV=${MMC_CHOICE_DEV}
	fi

        local MMC_DD_SEEK=16

        echo MMC: ${MMC_DEV}
        echo FILE: ${FILE_BOOTLOADER_BOOT0}
        echo SEEK: ${MMC_DD_SEEK}

        erase_and_write_emmc_seek ${FILE_BOOTLOADER_BOOT0} ${MMC_DEV} ${MMC_DD_SEEK}
}

update_spl_uboot()
{
	if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then
		update_mmc_choice
		MMC_DEV=${MMC_CHOICE_DEV}
	fi        

        local MMC_DD_SEEK=32800

        echo MMC: ${MMC_DEV}
        echo FILE: ${FILE_BOOTLOADER_UBOOT}
        echo SEEK: ${MMC_DD_SEEK}

        erase_and_write_emmc_seek ${FILE_BOOTLOADER_UBOOT} ${MMC_DEV} ${MMC_DD_SEEK}
}

update_uboot_env()
{
	if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then
		update_mmc_choice
		PART_MMC_ENV=${MMC_CHOICE_DEV}${PART_NO_ENV}
	else
		PART_MMC_ENV=${MMC_DEV}${PART_NO_ENV}
	fi

        echo MMC: ${PART_MMC_ENV}
        echo FILE: ${FILE_BOOTLOADER_ENV}

        erase_and_write_emmc ${FILE_BOOTLOADER_ENV} ${PART_MMC_ENV}
}

update_spl()
{
        update_spl_boot0

        sleep 1
        update_spl_uboot        

        sleep 1
        update_uboot_env
}

update_kernel()
{
	if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then									
		update_mmc_choice
		PART_MMC_KERNEL=${MMC_CHOICE_DEV}${PART_NO_KERNEL}
	else
		PART_MMC_KERNEL=${MMC_DEV}${PART_NO_KERNEL}
	fi

        echo MMC: ${PART_MMC_KERNEL}
        echo FILE: ${FILE_KERNEL_IMG}

        erase_and_write_emmc ${FILE_KERNEL_IMG} ${PART_MMC_KERNEL}
}

#将核心板上的mmc扩容至8GB
update_mmc()
{        
        local DIR_MNT=/mnt

        check_block_device_exist ${DIR_DEV}/${PARTITION_ROOTFS_EMMC}
        
        local MMC_MOUNTED=`mount | grep ${DIR_DEV}/${PARTITION_ROOTFS_EMMC}`
        if [ -z "${MMC_MOUNTED}" ]; then
                echo ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} is not mounted.
        else
                echo ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} is mounted.
                echo We shoule umount first.
                umount ${DIR_MNT}
        fi

        echo ""

        e2fsck -f ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} -y
        if [ $? -ne 0 ]; then                
                shell_display_color_words error "E2fsck ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} failed."                
                exit 1
        else
                # echo E2fsck ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} succeeded!
                shell_display_color_words warn "E2fsck ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} succeeded!"
        fi

        echo ""

        mount ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} ${DIR_MNT}
        if [ $? -ne 0 ]; then                
                shell_display_color_words error "Mount ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} to ${DIR_MNT} failed."
                exit 1
        else
                # echo Mount ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} to ${DIR_MNT} succeeded!
                shell_display_color_words warn "Mount ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} to ${DIR_MNT} succeeded!"
        fi

        echo ""

        resize2fs -p ${DIR_DEV}/${PARTITION_ROOTFS_EMMC}
        if [ $? -ne 0 ]; then                
                shell_display_color_words error "Resize ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} failed."
                exit 1
        else
                # echo Resize ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} succeeded!
                shell_display_color_words warn "Resize ${DIR_DEV}/${PARTITION_ROOTFS_EMMC} succeeded!"
        fi

        umount ${DIR_MNT}
        echo ""

        #格式化另一路SPI Flash
        update_spi_flash
}

check_and_fdisk_emmc()
{
        local EMMC_BLK_NAME="mmcblk1"
        local EMMC_BLK_PARTITION_NAME=${EMMC_BLK_NAME}p1

        local FDISK_INPUT_FILE=fdisk_emmc.txt
        local MKFS_FILE_TYPE=mkfs.ext4
                                        
        if [ ! -b ${DIR_DEV}/${EMMC_BLK_NAME} ]; then
                echo Board EMMC device: ${EMMC_BLK_NAME} is not exist!
                exit 1
        fi

        if [ ! -b ${DIR_DEV}/${EMMC_BLK_PARTITION_NAME} ]; then
                echo Board EMMC device: ${EMMC_BLK_PARTITION_NAME} is not exist, We should fdisk to create.
        else
                echo Board EMMC device: ${EMMC_BLK_PARTITION_NAME} exists.
                echo ""
                return 0                
        fi

        #将第二个分区：usr_local给划分出来并格式化
        echo Starting patitioning ${EMMC_BLK_NAME} to ${EMMC_BLK_PARTITION_NAME} ...
        fdisk ${DIR_DEV}/${EMMC_BLK_NAME} < ${DIR_UPDATE}/${DIR_UBOOT_SPL}/${FDISK_INPUT_FILE}
        ${MKFS_FILE_TYPE} ${DIR_DEV}/${EMMC_BLK_PARTITION_NAME}
        if [ $? -ne 0 ]; then                
                shell_display_color_words error "${MKFS_FILE_TYPE} failed."
                exit 1
        fi
}

install_rootfs()
{
        # if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then
                # update_mmc_choice
                # PART_MMC_ROOTFS=${MMC_CHOICE_DEV}${PART_NO_ROOTFS}
        # else
                # PART_MMC_ROOTFS=${MMC_DEV}${PART_NO_ROOTFS}
        # fi

        echo MMC: ${PART_MMC_ROOTFS}
        echo FILE: ${FILE_ROOTFS_IMG}

        erase_and_write_emmc ${FILE_ROOTFS_IMG} ${PART_MMC_ROOTFS}
}

update_rootfs()
{             
        #在update_all模式下，需要检测当前rootfs是被挂载到哪个分区，如果和MMC_DEV一致，则不能更新文件系统（因为文件系统不能自己更新自己）；
        #如果不一致，则可以更新文件系统。
        detect_rootfs_partition        

        #因为MMC_DEV已经被重新赋值，所以此处需要重新给PART_MMC_ROOTFS赋值
        PART_MMC_ROOTFS=${MMC_DEV}${PART_NO_ROOTFS}

        echo "Checking eMMC rootfs partition: ${PART_MMC_ROOTFS} ..."
        check_block_device_exist ${PART_MMC_ROOTFS}        

        echo "We'll install rootfs to eMMC partition: ${PART_MMC_ROOTFS} ..."
        install_rootfs
}

update_all()
{        
        shell_display_color_words  green "现在更新bootloader..."
        echo ""
        sleep 1
        update_spl

        shell_display_color_words  green "现在更新内核映像..."
        echo ""
        sleep 1
        update_kernel

        shell_display_color_words  green "现在更新文件系统..."
        echo ""
        sleep 1
        update_rootfs
        #update_mmc
}

detect_mmc_type()
{
	#检测系统中有几个mmc设备
	local MMC_BLK_DEV=mmcblk0
	MMC_COUNT=$(ls /dev/mmcblk* 2>/dev/null | grep -E '/dev/mmcblk[0-9]$' | wc -l)
	echo "系统中检测到 $MMC_COUNT 个 mmcblk 设备"

	if [ $MMC_COUNT -eq 0 ]; then
		shell_display_color_words error "没有检测到 mmcblk 设备，请检查连接或驱动。"
		exit 1
	fi

	for i in $(seq 0 $((MMC_COUNT-1))); do

	    MMC_BLK_DEV="mmcblk${i}"
	    
	    echo "检测到 mmcblk 设备: ${MMC_BLK_DEV}"

	    	#检测mmcblk的类型
		#类型值：MMC或者SD		
		check_block_device_exist ${DIR_DEV}/${MMC_BLK_DEV}
		MMC_TYPE=`cat /sys/block/${MMC_BLK_DEV}/device/type`        
		if [ ${MMC_TYPE} = "MMC" ]; then
		        shell_display_color_words GREEN "${MMC_BLK_DEV} type is: ${MMC_TYPE}"
		        MMC_DEV=${DIR_DEV}/${MMC_BLK_DEV}
			echo Current eMMC device is: ${MMC_DEV} 
		elif [ ${MMC_TYPE} = "SD" ]; then
		        shell_display_color_words GREEN "${MMC_BLK_DEV} type is: ${MMC_TYPE}"
			TF_DEV=${DIR_DEV}/${MMC_BLK_DEV}
			echo Current TF_CARD device is: ${TF_DEV} 
		else
		        shell_display_color_words GREEN "${MMC_BLK_DEV} type is: UNKNOWN"
		        exit 1                
		fi

		echo ""		

	done

        echo "==================================================================="
        echo ""
}

#使用sgdisk命令对eMMC进行分区
emmc_sgdisk_partitions()
{
	#sgdisk --resize-table=128 -a 1 -n 1:34:4129 -c 1:ssbl -p /dev/mmcblk1 
	#sgdisk --resize-table=128 -a 1 -n 2:4130:135201 -c 2:bootfs -p /dev/mmcblk1
	#sgdisk -n 1:1MiB:+1MiB -t 1:ef02 -c 1:grub /dev/sda

	return 0
}

#备份GPT分区表，需要带1个参数，参数为mmc设备名称，如/dev/mmcblk1
mmc_gpt_backup()
{
	if [ $# -ne 1 ]; then
		echo "Usage: $0 <mmc_device>"
		exit 1
	fi

	# 检查输入的设备是否存在
        check_block_device_exist $1

	local MMC_BACKUP=$1
	echo "Backing up GPT partition table from device: ${MMC_BACKUP}"
	# 获取磁盘总扇区数
	# SECTORS=$(cat /sys/block/${MMC_BACKUP}/size)

	#备份主分区表
	# dd if=/dev/${MMC_BACKUP} of=gpt_main_table.bin bs=512 seek=0 count=16

	# 备份最后 33 个扇区（GPT 备份表通常占用最后 33 个扇区）
	# dd if=/dev/${MMC_BACKUP} of=gpt_backup_table.bin bs=512 skip=$((SECTORS-33)) count=33

	sgdisk --backup=${FILE_BOOTLOADER_GPT_BACKUP} ${MMC_BACKUP}
	if [ $? -ne 0 ]; then
		shell_display_color_words error "Error: Failed to backup ${MMC_BACKUP} GPT partition table."
		exit 1
	fi

	shell_display_color_words GREEN "${MMC_BACKUP} GPT partition table backup successfully."
}

#恢复GPT分区表，需要带1个参数，参数为mmc设备名称，如mmcblk1
mmc_gpt_restore()
{
	if [ $# -ne 1 ]; then
		echo "Usage: $0 <mmc_device>"
		exit 1
	fi

	# 检查输入的设备是否存在
        check_block_device_exist $1

	local MMC_RESTORE=$1

	# 恢复主分区表
	# dd if=gpt_main_table.bin of=/dev/${MMC_RESTORE} bs=512 seek=0 count=16

	# 恢复最后 33 个扇区（GPT 备份表通常占用最后 33 个扇区）
	# dd if=gpt_backup_table.bin of=/dev/${MMC_RESTORE} bs=512 skip=$((SECTORS-33)) count=33

	check_file_exist ${FILE_BOOTLOADER_GPT_BACKUP}

	sgdisk --load-backup=${FILE_BOOTLOADER_GPT_BACKUP} ${MMC_RESTORE}
	if [ $? -ne 0 ]; then
		shell_display_color_words error "Error: Failed to restore ${MMC_RESTORE} GPT partition table."
		exit 1
	fi

	shell_display_color_words GREEN "${MMC_RESTORE} GPT partition table restored successfully from backup."
}

show_mmc_part()
{
        if [ $# -ne 1 ]; then
                echo "Usage: $0 <mmc_device>"
                exit 1
        fi

        # arr_part_name=("boot-resource" "env" "boot" "private" "rootfs" "riscv0" "UDISK")

        for i in "${arr_part_name[@]}"; do
                local MMC_PART_NAME=$i                
                PART_NO=$(sgdisk -p $1 | grep -E "[[:space:]]${MMC_PART_NAME}[[:space:]]*$" | awk '{print $1}')
                if [ -z "${PART_NO}" ]; then
                        echo "分区名称: $i, 未找到对应的分区号"
                        continue
                fi

                PART_NO=p${PART_NO}
                
                case $i in
                        "boot-resource")
                                PART_MMC_BOOT_RESOURCE=${1}${PART_NO}
                                PART_NO_BOOT_RESOURCE=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_BOOT_RESOURCE}"
                                ;;
                        "env")
                                PART_MMC_ENV=${1}${PART_NO}
                                PART_NO_ENV=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_ENV}"
                                ;;
                        "boot")
                                PART_MMC_KERNEL=${1}${PART_NO}
                                PART_NO_KERNEL=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_KERNEL}"
                                ;;                                                                                                
                        "rootfs")
                                PART_MMC_ROOTFS=${1}${PART_NO}
                                PART_NO_ROOTFS=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_ROOTFS}"
                                ;;
                        "riscv0")
                                PART_MMC_RISCV=${1}${PART_NO}
                                PART_NO_RISCV=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_RISCV}"
                                ;;
                        "UDISK")
                                PART_MMC_UDISK=${1}${PART_NO}
                                PART_NO_UDISK=${PART_NO}
                                echo -e "分区名称: $i, ${PART_MMC_UDISK}"
                                ;;
                        *)
                                echo -e "分区名称: $i, ${1}${PART_NO}"
                                ;;
                esac
                        
        done

        echo "==================================================================="
        echo ""
}

detect_emmc_is_parted()
{
        local PART_COUNT=$(ls ${MMC_DEV}p* 2>/dev/null | wc -l)

        if [ "$PART_COUNT" -gt 0 ]; then
            echo "eMMC 已分区，分区数: $PART_COUNT"
            show_mmc_part ${MMC_DEV}
            return 0
        else
            shell_display_color_words YELLOW "eMMC 未分区，请先分区！"
        fi
        
        echo -n "现在需要对eMMC开始分区吗？ [y/n/q] " && read items
        if [ "$items" = "q" ] || [ "$items" = "n" ]; then
            shell_display_color_words YELLOW "你选择了退出，脚本退出！"
            exit 0
        fi

        if [ "$items" = "y" ]; then
            echo "你选择了分区，现在开始分区..."
            mmc_gpt_restore ${MMC_DEV}
        fi        
}

detect_system()
{
        detect_mmc_type
        detect_emmc_is_parted
}

update_gpt()
{
	# echo "GPT脚本函数尚未完成！"
	# exit 1

	echo -n "你是要备份还是恢复GPT分区表？ [b - 备份/r - 恢复] " && read opmode
	if [ "$opmode" != "b" ] && [ "$opmode" != "r" ]; then				
		shell_display_color_words error "Error: Invalid operation mode: $opmode"
		exit 1
	fi

	if [ ${UPDATE_MODE} != "all" ] && [ ${MMC_COUNT} -gt 1 ]; then
		update_mmc_choice
		MMC_DEV=${MMC_CHOICE_DEV}
	fi        
	
	echo MMC: ${MMC_DEV}
	echo FILE: ${FILE_BOOTLOADER_GPT_BACKUP}

	if [ "$opmode" = "b" ]; then
		# 备份GPT分区表
		mmc_gpt_backup ${MMC_DEV}
	else										
		mmc_gpt_restore ${MMC_DEV}
	fi
}

###############################################################
#主脚本代码开始
###############################################################

#检测输入参数
detect_input_paramters $1
echo "==================================================================="
echo ""

detect_system

# echo Now we coming into update ${UPDATE_MODE} mode ...       
update_${UPDATE_MODE}

sync
sleep 1
echo Operating finished, Now you need poweroff, and restart system ...
exit 0
```

