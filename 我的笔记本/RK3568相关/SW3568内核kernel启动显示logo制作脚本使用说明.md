# SW3568内核kernel启动显示logo制作脚本使用说明
该目录下的脚本用于定制制作`SW3568核心板`启动时显示的客户logo图片。
软件压缩包:
[SW3568内核kernel启动显示logo制作脚本.tar.gz](file:///media/sf_D_DRIVE/BaiduNetdiskWorkspace/Linux_work/RK3568相关_20240311/SW3568内核kernel启动显示logo制作脚本_20240325-1.tar.gz)

脚本文件名称：`update_kernel_logo.sh`
脚本执行时需要配合配置文件：`config.ini`来操作。`config.ini`用于设置需要修改内核logo的图片文件名称，以及需要修改内核设备文件树名称等。
## 一、`config.ini`配置文件说明
其内容如下:
```bash
[DIR_SET]
DIR_ROOT=/media/jason/Rockchip1/rk356x

[PIC_SET]
LOGO_BMP=logo_rockchip.bmp
LOGO_KERNEL_BMP=logo_kernel_rockchip.bmp
LOGO_KERNEL_PPM=penguin_logo_linux_clut224.ppm

[DTB_SET]
DIR_BOARD_CONFIG=/media/jason/Rockchip1/rk356x/device/rockchip/rk356x
DTB_FILE_NAME=sanway-rk3568-ecu4000s-v30-hdmi
```
其内容字段详细说明如下：
* **`DIR_ROOT`**
    指定RK SDK系统的根目录，根据你安装的RK SDK的目录来指定。一般一旦你安装好了RK官方的SDK开发包，则该目录就确定了。默认情况下，一旦设置好则不需要修改。
   
* **`LOGO_BMP`**
    用于设定系统开机刚上电时显示的画面，一般为`boot loader`启动时显示的画面。
* **`LOGO_KERNEL_BMP`**
    用于设置系统`boot loader`启动完成后，进入到`Linux Kernel`内核启动时显示的画面。
    `LOGO_KERNEL_BMP`可以和`LOGO_BMP`指定为不同的图片，也可以指定为相同的图片。
    
* **`LOGO_KERNEL_PPM`**
    当`Linux Kernel`内核打开了`Framebuffer`配置开关，那么内核在初始化完成后，就会显示由`LOGO_KERNEL_PPM`指定的显示图片。
    注意该图片为ppm格式，需要通过相关的转换命令将`png`格式图片转换为`ppm`格式才能使用。
    一个转换图片的脚本实例如下所示：
    ```bash
    if [ $# -eq 1 ]; then
        pngtopnm  $1  >  linux_logo.pnm
        pnmquant  224  linux_logo.pnm  >  logo_clut224.pnm
        pnmtoplainpnm  logo_clut224.pnm  >  logo_clut224.ppm
        ./ppmtobin_64  logo_clut224.ppm  -t  clut224  -o  logo.bin
    else
    	echo "Usage: $0 file.png"
    fi
    ```
    
* **`DIR_BOARD_CONFIG`**
    用于设定RK SDK对应芯片型号：`rk356x`板级配置文件的所在目录。一般RK SDK开发包安装好了之后，该目录也确定了，默认情况下一旦设定好就不需要修改。
    
* **`DTB_FILE_NAME`**
    用于指定你要替换的内核设备文件树（DTB）的名称。该文件一般位于内核目录下：
    ```bash
    kernel/arch/arm64/boot/dts/rockchip/
    ```
    每次运行脚本之前，需要将要替换的新的内核设备文件树（DTB）的名称写入该字段，然后再运行脚本。
    


## 二、运行脚本：`update_kernel_logo.sh`

### 1. 脚本可以带参数运行，也可以不带参数运行。不带参数运行时，界面显示如下：

```bash
$ ./update_kernel_logo.sh
config.ini's KEY value:
[DIR_ROOT]         = /media/jason/Rockchip1/rk356x
[LOGO_BMP]         = logo_rockchip.bmp
[LOGO_KERNEL_BMP]  = logo_kernel_rockchip.bmp
[LOGO_KERNEL_PPM]  = penguin_logo_linux_clut224.ppm
[DIR_BOARD_CONFIG] = /media/jason/Rockchip1/rk356x/device/rockchip/rk356x
[DTB_FILE_NAME]    = sanway-rk3568-ecu4000s-v30-hdmi

Setup SW3568 Linux kernel start logo ...
Available options:
0 - default            -switch to Rockchip Kernel logo config
1 - wanlida            -switch to Wanlida Kernel logo config
2 - kechuang_800x600   -switch to KeChuang 800x600 Kernel logo config
3 - kechuang_1024x600  -switch to KeChuang 1024x600 Kernel logo config
100 - Set kernel dtb   -Set kernel DTB file name with new DTB file: config.ini key value [DTB_FILE_NAME].
q                      -quit script: ./update_kernel_logo.sh

processing option: [0 - 100, or "q"]: 
        
```

脚本运行后，会打印`config.ini`配置文件中所有字段的内容，然后出现菜单界面，等待输入按键值。注意按键值需要为菜单中显示的数字。
    * 如果只键入`Enter`回车键，则会重复打印菜单界面，等待用户输入。
    * 如果键入`q`键，则会退出脚本。
    * 如果键入非菜单中提示的数字值，则会提示输入出错，然后重复打印菜单界面，继续等待用户输入，直到用户按下`q`键退出脚本。

### 2. 带参数运行脚本
   脚本也可以带参数运行，如下所示：

```bash
$ ./update_kernel_logo.sh 0
```
   此时如果参数正确，则脚本不会运行交互式菜单界面，而是直接根据参数来运行对应的选项，然后打印执行信息，再退出脚本。如下所示：

```bash
$ ./update_kernel_logo.sh 0
config.ini's KEY value:
[DIR_ROOT]         = /media/jason/Rockchip1/rk356x
[LOGO_BMP]         = logo_rockchip.bmp
[LOGO_KERNEL_BMP]  = logo_kernel_rockchip.bmp
[LOGO_KERNEL_PPM]  = penguin_logo_linux_clut224.ppm
[DIR_BOARD_CONFIG] = /media/jason/Rockchip1/rk356x/device/rockchip/rk356x
[DTB_FILE_NAME]    = sanway-rk3568-ecu4000s-v30

0
switch to build default kernel logo config
Starting update now ...
Current_dir: /media/jason/Rockchip1/rk356x/sanway/SW3568内核kernel启动显示logo制作脚本
root_dir: /media/jason/Rockchip1/rk356x
kernel_dir: /media/jason/Rockchip1/rk356x/kernel
kernel_logo_dir: /media/jason/Rockchip1/rk356x/kernel/drivers/video/logo

logo_bmp: ./config/default/logo_rockchip.bmp
logo_kernel_bmp: ./config/default/logo_kernel_rockchip.bmp
logo_kernel_video_logo: ./config/default/penguin_logo_linux_clut224.ppm

lrwxrwxrwx 1 jason jason 17  3月22日 14:19 /media/jason/Rockchip1/rk356x/kernel/logo.bmp -> logo_rockchip.bmp
lrwxrwxrwx 1 jason jason 24  3月22日 14:19 /media/jason/Rockchip1/rk356x/kernel/logo_kernel.bmp -> logo_kernel_rockchip.bmp
-rw-rw-rw- 1 jason 1001 76845  3月22日 14:19 /media/jason/Rockchip1/rk356x/kernel/drivers/video/logo/logo_linux_clut224.ppm
Updating succeeded! Now You can rebuild kernel image ...

```
   如果输入的参数不正确，则脚本运行如下：

```bash
$ ./update_kernel_logo.sh 10
config.ini's KEY value:
[DIR_ROOT]         = /media/jason/Rockchip1/rk356x
[LOGO_BMP]         = logo_rockchip.bmp
[LOGO_KERNEL_BMP]  = logo_kernel_rockchip.bmp
[LOGO_KERNEL_PPM]  = penguin_logo_linux_clut224.ppm
[DIR_BOARD_CONFIG] = /media/jason/Rockchip1/rk356x/device/rockchip/rk356x
[DTB_FILE_NAME]    = sanway-rk3568-ecu4000s-v30

10
You choiced wrong item: 10
Setup SW3568 Linux kernel start logo ...
Available options:
0 - default            -switch to Rockchip Kernel logo config
1 - wanlida            -switch to Wanlida Kernel logo config
2 - kechuang_800x600   -switch to KeChuang 800x600 Kernel logo config
3 - kechuang_1024x600  -switch to KeChuang 1024x600 Kernel logo config
100 - Set kernel dtb   -Set kernel DTB file name with new DTB file: config.ini key value [DTB_FILE_NAME].
q                      -quit script: ./update_kernel_logo.sh

```

### 3. 更换内核启动logo

选择需要设置logo的选项，选择数字0，1，2，3，...（未来会根据客户定制的情况逐渐增加数字选项），然后键入`Enter`回车键，脚本提示如下：
    
```bash
processing option: [0 - 100, or "q"]: 0

switch to build default kernel logo config
Starting update now ...
Current_dir: /media/jason/Rockchip1/rk356x/sanway/SW3568内核kernel启动显示logo制作脚本
root_dir: /media/jason/Rockchip1/rk356x
kernel_dir: /media/jason/Rockchip1/rk356x/kernel
kernel_logo_dir: /media/jason/Rockchip1/rk356x/kernel/drivers/video/logo

logo_bmp: ./config/default/logo_rockchip.bmp
logo_kernel_bmp: ./config/default/logo_kernel_rockchip.bmp
logo_kernel_video_logo: ./config/default/penguin_logo_linux_clut224.ppm

lrwxrwxrwx 1 jason jason 17  3月22日 13:52 /media/jason/Rockchip1/rk356x/kernel/logo.bmp -> logo_rockchip.bmp
lrwxrwxrwx 1 jason jason 24  3月22日 13:52 /media/jason/Rockchip1/rk356x/kernel/logo_kernel.bmp -> logo_kernel_rockchip.bmp
-rw-rw-rw- 1 jason 1001 76845  3月22日 13:52 /media/jason/Rockchip1/rk356x/kernel/drivers/video/logo/logo_linux_clut224.ppm
Updating succeeded! Now You can rebuild kernel image ...

```

说明logo替换成功，系统退出脚本。运行完成后，可以直接编译内核，参考5。
    
### 4. 更换内核设备文件树（DTB）

脚本还可以替换内核设备文件树（DTB）。在运行脚本之前，先要修改`config.ini`中的`DTB_FILE_NAME`字段，指定新的设备文件树的名称。然后再运行脚本，键入数字：`101`，回车，脚本运行如下所示：

```bash
processing option: [0 - 200, or "q"]: 101

INI set DTB dir: /media/jason/Rockchip1/rk356x/device/rockchip/rk356x
INI set DTB file: sanway-rk3568-ecu4000s-v30
Board config file: /media/jason/Rockchip1/rk356x/device/rockchip/rk356x/BoardConfig-sanway-rk3568-evb1-ddr4-v10-32bit.mk
old DTB file name: sanway-rk3568-ecu4000s-v30-hdmi
new DTB file name: sanway-rk3568-ecu4000s-v30
Updating succeeded! Now You can rebuild kernel image ...
```
说明设备文件树（DTB）名称修改成功，系统退出脚本。然后可以编译内核了。
注意，如果`config.ini`中的**`DTB_FILE_NAME`**字段中指定的设备文件树（DTB）名称和RK板级配置文件中指定的名称重复了，则脚本会输出以下内容提示：

```bash
processing option: [0 - 200, or "q"]: 101

INI set DTB dir: /media/jason/Rockchip1/rk356x/device/rockchip/rk356x
INI set DTB file: sanway-rk3568-ecu4000s-v30-hdmi
Board config file: /media/jason/Rockchip1/rk356x/device/rockchip/rk356x/BoardConfig-sanway-rk3568-evb1-ddr4-v10-32bit.mk
old DTB file name: sanway-rk3568-ecu4000s-v30-hdmi
new DTB file name: sanway-rk3568-ecu4000s-v30-hdmi
DTB file set doesn't need to be replaced!
```
此时并不会直接替换，而是给出不需要替换的提示，直接退出脚本。

### 5. 设置`config.ini`中`DTB_FILE_NAME`字段的值
脚本新增了可以直接设置`config.ini`配置文件中`DTB_FILE_NAME`字段的值，这样可以不用打开`config.ini`来进行编辑，以免出现改错的情况。
运行脚本，并选择`100`项：
```bash
processing option: [0 - 200, or "q"]: 100

Please input DTB file name you want to replace: 

```
此时会出现提示菜单，要求输入新的DTB文件名称，来替换`config.ini`中原来指定的DTB文件名称。输入新的DTB文件名称，如果无误，则`config.ini`中的`DTB_FILE_NAME`字段就被更新了。

如下所示：

```bash
Please input DTB file name you want to replace: sanway-rk3568-ecu4000s-v30
current config.ini path: ./config/default/config.ini
Replaced success! Now  ./config/default/config.ini's [DTB_FILE_NAME]: sanway-rk3568-ecu4000s-v30
Updating succeeded! Now You can rebuild kernel image ...

```
> [!note] 
>
> 注意
> 1）如果输入的DTB文件名称和`config.ini`配置文件中的名称重复，则不会更新`config.ini`配置文件；
> 2）更新之前，脚本会检查内核对应dts目录下是否有该文件，如果没有，则出现出错提示，并且不会更新`config.ini`配置文件。

如下所示：
```bash
processing option: [0 - 200, or "q"]: 100

Please input DTB file name you want to replace: 123456
File: /media/jason/Rockchip1/rk356x/kernel/arch/arm64/boot/dts/rockchip/123456.dts is not exist!

```

### 6. 重新编译内核

脚本还新增了直接编译内核选项。在脚本运行主菜单中选择`200`，则会启动内核编译。
如下所示：

```bash
processing option: [0 - 200, or "q"]: 200

processing option: kernel
============Start building kernel============
TARGET_ARCH          =arm64
TARGET_KERNEL_CONFIG =sanway_rockchip_linux_defconfig
TARGET_KERNEL_DTS    =sanway-rk3568-ecu4000s-v30-hdmi
TARGET_KERNEL_CONFIG_FRAGMENT =
==========================================
#
# configuration written to .config
#

```
如果修改无误，则内核编译过程中，会重新编译设备文件树，和内核logo文件。具体可参阅内核编译打印输出来获取相关信息。