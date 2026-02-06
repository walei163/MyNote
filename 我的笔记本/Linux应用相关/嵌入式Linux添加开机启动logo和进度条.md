# 嵌入式Linux添加开机启动logo和进度条

嵌入式Linux添加开机启动进度条和logo

快成大叔的小伙

工程师

### 下载软件psplash：

[官方源码下载地址,](http://git.yoctoproject.org/cgit/cgit.cgi/psplash/) 

编译：需要安装automake工具。

        aclocal
        autoheader
        automake --add-missing
        autoconf
        #./configure --host=arm-linux CC=arm-linux-gcc #设置交叉编译库
        #./make-image-header.sh base-images/xxx.png POKY#会生成xxx-img.h 
        #make

最后将psplash复制到/usr/bin/psplash 若修改了psplash也要做相应覆盖

制作脚本

psplash.sh

        #!/bin/sh
        export TMPDIR=/mnt/.psplash
        mkdir -p $TMPDIR
        mount tmpfs -t tmpfs $TMPDIR -o,size=40k
        /usr/bin/psplash &

使用：

        echo "PROGRESS 50" > /mnt/.psplash/psplash_fifo

可以观察到进度条的变化，亲测可用，不会留言吧。

### 更多功能介绍：

修改相关源文件，

psplash-config.h

        /* Text to output on program start; if undefined, output nothing */
        #define PSPLASH_STARTUP_MSG ""

        /* Bool indicating if the image is fullscreen, as opposed to split screen */
        #define PSPLASH_IMG_FULLSCREEN 1

        /* Position of the image split from top edge, numerator of fraction */
        #define PSPLASH_IMG_SPLIT_NUMERATOR 5

        /* Position of the image split from top edge, denominator of fraction */
        #define PSPLASH_IMG_SPLIT_DENOMINATOR 6

psplash-colors.h颜色配置文件（背景色 进度条颜色等）

        /* This is the overall background color */
        #define PSPLASH_BACKGROUND_COLOR 0x00,0x00,0x00 //背景色rgb值

        /* This is the color of any text output */
        #define PSPLASH_TEXT_COLOR 0xFF,0xFF,0xFF//文本内容颜色

        /* This is the color of the progress bar indicator */
        #define PSPLASH_BAR_COLOR 0xff,0x00,0x00 //进度条颜色
        /* This is the color of the progress bar background */
        #define PSPLASH_BAR_BACKGROUND_COLOR 0x00,0x00,0x00//进度条背景颜色

发布于 2019-03-25