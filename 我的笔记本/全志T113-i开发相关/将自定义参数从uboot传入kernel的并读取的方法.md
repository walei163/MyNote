# 将自定义参数从uboot传入kernel的并读取的方法

    uboot向kernel传参机制以及kernel如何解析cmdline参数大家可以参考其他前辈的博文，这方面的讲解十分详尽。
    
    在这里我总结以下自己在uboot自定义一个参数并从kernel中取得的方法：

## 一.uboot添加自定义参数number

在uboot命令行下，我通过`setenv bootargs`以及`saveenv`命令来在`bootargs`参数中添加自己的一个参数，例如：

```sh
=>setenv bootargs xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx number=${num}
=>setenv num 0 
=>saveenv
```


其中`bootargs`前面部分省略，number为在原`bootargs`后添加的参数，它会去取num的值，因此以后只要修改num的值即可(这里先设置为0)     

## 二.kernel读取uboot传入的参数number：

在想要取得uboot传入参数的文件中进行如下操作：

  1.添加头文件:

```c
 #include <linux/init.h>
```

  2.添加如下内容：

```c
static unsigned int number;
static int __init number_setup (char *num)
{
     number = *num - '0';
}
__setup("number=", number_setup);
```


  宏`__setup("number=", number_setup)`会在`bootargs`中取得`"number="`后的值作为函数`number_setup`的参数`num`，因此只要在`number_setup`函数中用全局变量`number`接收该值即可(此处我将传入的值作为数字处理)。

至此，我们就能在kernel中读取到uboot传入的自定义参数值

**注意：我们在早期的am335x（内核版本：V3.2）的内核驱动：dm9000.c中，曾经实现过这一方式：**

```c
static int dm9000_fiber_setup(char *s)
{
	//printk(KERN_INFO "DM9000 Fiber mode: [%s].\n", s);
	//cmdline_fiber_is_on = 1;
	
	cmdline_fiber_is_on = simple_strtoul(s, NULL, 10);
	
	printk(KERN_INFO "DM9000 Fiber mode: [%d].\n", cmdline_fiber_is_on);
	
	return 0;
}

__setup("fiber=", dm9000_fiber_setup);
```

这样，从U-boot环境变量中传过来的fiber是否为1，来确定以太网驱动是否要采用光口模式。



****************************************************************************************************************

另：作为用户来说，不可能每次启动都先在uboot命令行下通过setenv，saveenv来修改传入kernel的环境变量，因此uboot提供了fw_printenv工具来让用户在文件系统下来查看和修改环境变量，具体步骤如下：

## 1.编译fw_printenv工具：

在uboot源码目录下，执行：

```sh
make env ARCH=xxx CROSS_COMPILE=yyy
```

其中xxx和yyy根据自己的交叉编译环境来配置

接着会在tools/env下生成fw_printenv的可执行文件，将其拷贝到文件系统中的/usr/bin(此路径可以自己指定)目录下，再创建fw_printenv的软链接（直接cp也可以）：

```sh
ln -s /usr/bin/fw_printenv /usr/bin/fw_setenv
```

## 2.配置fw_env.config文件

    根据mtd分区、UBOOT环境变量的位置、大小等内容修改uboot源码目录的tools/env下的fw_env.config文件，具体的修改方法见fw_env.config文件中的说明及/tools/env/README文件，我的修改内容如下作为参考(需根据实际情况来修改)：

# MTD device name       Device offset   Env. size       Flash sector size
/dev/mtd0               0xc0000         0x2000          0x10000

其中Device offset，Env size和Flash sector size应该分别对应于uboot源码目录中include/configs/xxxx.h相关文件中的

CONFIG_ENV_OFFSET,CONFIG_ENV_SIZE和CONFIG_ENV_SECT_SIZE三个宏定义

 

修改完成后，将fw_env.config文件拷贝到文件系统的/etc目录下。

 

## 3.fw_printenv的使用

    在文件系统中输入fw_printenv命令来查看环境变量，输入fw_setenv xxx yyy 来修改环境变量(xxx为变量名，yyy为变零值)，方法类似setenv,修改后会自动保存，详细用法通过fw_printenv --help命令来查看
-----------------------------------
将自定义参数从uboot传入kernel的并读取的方法【转】
https://blog.51cto.com/u_8475449/5629364