# 编写C语言动态库并调用（完整篇）

目录

[TOC]



## 1. 安装好codesys V3.5 SP21 Patch 1+（64-bit），新建工程：

![image-20250730150629091](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730150629091.png)

如上图所示，新建一个`Empty library`，例如名称叫做：`lib_test2`。

## 2. 定义功能块：

### 2.1. 添加对象 -> POU...，选择功能块（B），点击“添加”。

### 2.2. 修改功能块的名称，例如叫做：“`TestDiv_cext`”。

**注意：外部c函数必须要加`_cext`后缀。**

### 2.3. 在POU功能块的上下文菜单中选择“属性”，然后在“编译”选项卡上选择“外部实现（E）（稍后在runtime系统中链接）”。

如下图所示：

![image-20250730151538256](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730151538256.png)

点击“确定“后，原来的`TestDiv_cext`功能块旁边会多出一个标识：（EXT）。如下图：

![image-20250730151743056](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730151743056.png)

### 2.4. 双击`TestDiv_cext`功能块，开始定义声明：

```stylus
FUNCTION_BLOCK TestDiv_cext
VAR_INPUT
	rDiv1 : REAL;
	rDiv2 : REAL;
END_VAR
VAR_OUTPUT
	rResult : REAL;
END_VAR
VAR
END_VAR
```

这样定义了2个输入参数：rDiv1，rDiv2，和一个输出返回值：rResult。

## 3. 填写“工程信息”。如图：

![image-20250730152147749](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730152147749.png)

**注意：这一步非常重要！尤其是“标题”那一栏的内容，会决定后续动态库的名称！**

## 4. 生成运行时系统文件：

如果代码编写完成且没有问题，则可以点击：

编译 -> 生成运行时系统文件...

![image-20250730152631723](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730152631723.png)

选择“`M4接口文件`”和“`C存根文件`”，点击“确定”，将会导出2个文件：

```sh
lib_test2.c
lib_test2Itf.m4
```

这2个文件就是用来编写C语言动态库的代码文件。

## 5. Linux下编程并编译动态库：

切换到Linux系统下，新建一个动态库目录。例如叫做：“test”。

```sh
mkdir test
```

### 5.1. 指定CODESYS系统的`Extension SDK`包的路径，并且创建一个新的项目。

为了便于操作，这里写了一个脚本：`make_codesys_project`

```sh
SDKDIR=../codesys_extension_sdk
New_Project="newproject"

MAKE_CONTENT="make -f ${SDKDIR}/makefile ${New_Project}"

echo "${MAKE_CONTENT}"
${MAKE_CONTENT}
```

其中，`codesys_extension_sdk`是一个链接，指向`Extension SDK`包的路径：

```sh
$ ll codesys_extension_sdk
lrwxrwxrwx 1 jason jason 116  7月30日 11:39 codesys_extension_sdk -> '/media/sf_D_DRIVE/Program Files/CODESYS 3.5.21.10/CODESYS/CODESYS Control SL Extension Package/4.16.0.0/ExtensionSDK'
```

这个路径就是你的CODESYS软件的安装路径。

### 5.2. 运行脚本：`make_codesys_project`，会在项目目录下创建一个makefile文件。

其内容为：

```sh
##############################################################################
#	Copyright:		Copyright CODESYS Development GmbH
#	Program:		Extension API for the Linux SL products
##############################################################################

#
# with this makefile you can influence the build of your component 
#
#
# Hints:
#  - the name of the *Itf.m4 file in this folder will be used as name of the shared object (and component name)
#  - all C files in this folder will be compiled by default
# 

# set versions of your shared object
# must be single digit, decimal
MAJORVER=0
MINORVER=1

# set a component version (must match your IEC library version)
# must be 4 bytes, hexadecimal
CMPVERSION=0x01000000

# set a component ID 
# must be 2 bytes >= 0x2000 and hexadecimal and can be used to differentiate different components
CMPID=0x2000

# set your tools
DOS2UNIX = dos2unix
M4 = m4

# set a compiler
CROSS_COMPILE=aarch64-linux-gnu-
CC = ${CROSS_COMPILE}gcc

# add some compiler flags (with += )
#CFLAGS += -g

# add some include paths (with +=)
#INCLUDES += -I.

# add some linker flags (with += )
#LDFLAGS += 

# add some libraries that you might need (with += )
#LDLIBS += -lc



# include of the SDK makefiles
SDKDIR=/media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk
include ${SDKDIR}/makefile
```

修改makefile文件，添加以下内容，以满足ARM交叉编译的要求：

```sh
CROSS_COMPILE=aarch64-linux-gnu-
CC = ${CROSS_COMPILE}gcc
```

### 5.3. 将之前在CODESYS下导出的2个文件复制到该项目目录下，现在项目目录下的文件应该如下所示：

```sh
$ ll
总计 16K
-rwxrwx--- 1 jason jason  245  7月30日 14:06 lib_test2.c
-rwxrwx--- 1 jason jason 1.0K  7月30日 14:09 lib_test2Itf.m4
-rwxr-xr-x 1 jason jason  310  7月30日 11:43 make_codesys_project
-rwxr-x--- 1 jason jason 1.4K  7月30日 14:09 makefile
```

修改`lib_test2.c`的内容如下：

```c
#include "CmpStd.h"
#include "CmpErrors.h"
#include "CmpItf.h"
#include "lib_test2Dep.h"

void CDECL CDECL_EXT testdiv_cext__main(testdiv_cext_main_struct *p)
{
        p->pInstance->rResult = p->pInstance->rDiv1 / p->pInstance->rDiv2;
}
```

### 5.4. 编译项目：

```sh
make all

**********************
create out/lib_test2Dep.m4 from template
echo 'SET_COMPONENT_NAME(`'lib_test2"')" > out/lib_test2Dep.m4
echo 'COMPONENT_VERSION(`'0x01000000"')" >> out/lib_test2Dep.m4
echo 'COMPONENT_VENDORID(`'0xFFFF"')" >> out/lib_test2Dep.m4
echo "#define CMPID_lib_test2		0x2000" >> out/lib_test2Dep.m4
echo "IMPLEMENT_ITF("'`'"lib_test2Itf.m4')" >> out/lib_test2Dep.m4	
**********************

**********************
build dependency header: out/lib_test2Dep.h from out/lib_test2Dep.m4
dos2unix out/lib_test2Dep.m4
dos2unix: 正在转换文件 out/lib_test2Dep.m4 为Unix格式...
m4 -I/media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/m4 -I. Implementation.m4 - < out/lib_test2Dep.m4 > out/lib_test2Dep.h
**********************

**********************
build interface header: out/lib_test2Itf.h from lib_test2Itf.m4
dos2unix lib_test2Itf.m4
dos2unix: 正在转换文件 lib_test2Itf.m4 为Unix格式...
m4 -I/media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/m4 Exports.m4 - < lib_test2Itf.m4 > out/lib_test2Itf.h
**********************

**********************
compile: out/lib_test2.o from lib_test2.c
aarch64-linux-gnu-gcc  -c -fPIC -g -O0 -Wall -Wextra -Wno-unused -DDEP_H=\"lib_test2Dep.h\" -I/media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/include/ -I./out/ -I. -o out/lib_test2.o  lib_test2.c
**********************

**********************
compile: out/CmpFrame.o from /media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/src/CmpFrame.c
aarch64-linux-gnu-gcc  -c -fPIC -g -O0 -Wall -Wextra -Wno-unused -DDEP_H=\"lib_test2Dep.h\" -I/media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/include/ -I./out/ -I. -o out/CmpFrame.o  /media/jason/btrfs_disk_M/home/codesys/library/codesys_extension_sdk/src/CmpFrame.c
**********************

**********************
link: out/liblib_test2.so
aarch64-linux-gnu-gcc   -shared ./out/lib_test2.o ./out/CmpFrame.o -Wl,-soname,liblib_test2.so -o out/liblib_test2.so 
**********************
```

如果编译无误，则项目目录下会生成`out`目录，动态库文件就在这个目录下：

```sh
$ ll out
总计 64K
-rw-r--r-- 1 jason jason  14K  7月30日 15:45 CmpFrame.o
-rwxr-xr-x 1 jason jason  76K  7月30日 15:45 liblib_test2.so
-rw-r--r-- 1 jason jason 7.8K  7月30日 15:45 lib_test2Dep.h
-rw-r--r-- 1 jason jason  158  7月30日 15:45 lib_test2Dep.m4
-rw-r--r-- 1 jason jason 4.3K  7月30日 15:45 lib_test2Itf.h
-rw-r--r-- 1 jason jason 4.6K  7月30日 15:45 lib_test2.o
```

至此，动态库编译完成。

### 5.5. 将动态库文件从Linux系统下复制到你刚刚建立的CODESYS库的工程目录下，然后将其合并到库工程中。

在lib_test2的上下文菜单中：

添加对象 -> C实现库...

![image-20250730155321426](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730155321426.png)

名称例如叫做：`CImplLibrary`，点击“确定”。工程项目变成如下所示：

![image-20250730155638794](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730155638794.png)

双击`CImplLibrary`，出现“已编译组件”选项卡，再点击添加，将你刚刚编译好的动态库文件加入进来：

![image-20250730155929052](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730155929052.png)

**注意：**

**1）动态库的名称一定要和“工程信息”中“标题”那一栏的内容保持一致！一般动态库文件名应该：lib + 工程信息标题内容.so**

**2）如上图所示，一定要选择对应的PLC设备。如：`CODESYS Control for Linux ARM64 SL`，表示这个动态库需要运行在Linux ARM64位的系统上。**

### 5.6. 最后，将动态库导出为编译库以供CODESYS项目调用使用：

菜单“文件” -> 将工程另存为编译库...

![image-20250730160635090](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730160635090.png)

## 6. 动态库加载到CODESYS项目中并调用

### 6.1. 新建一个CODESYS工程项目，将动态库加载进来并调用：

双击“库管理器”，在“库管理器”选项卡中，点击“库存储库”，然后弹出的菜单中点击“安装”，选择在5.6节导出的编译库文件。如果加载无误，则如下图所示：

![image-20250730161037203](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730161037203.png)

### 6.2. 再继续在“库管理器”选项卡中，点击“添加库”，将`lib_test2`这个库加入到你当前的工程项目中来。

如果加入成功，则如下图所示：

![image-20250730161323365](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730161323365.png)

### 6.3. 调用`lib_test2`中的函数：

```stylus
PROGRAM PLC_PRG
VAR
my_test_div : sanway_lib_test2.TestDiv_cext;
_rdiv1 : REAL;
_rdiv2 : REAL;
_rResultDiv : REAL;
END_VAR

_rdiv1 := deInstWrite.diValue1;
_rdiv2 := 2.0;
my_test_div(rDiv1 := _rdiv1, rdiv2 := _rdiv2,  rResult => _rResultDiv);
```
这样，一个C语言动态库函数调用就完成了。

## 7. 动态库加载到装置runtime系统中并调用

### 7.1. 将动态库文件（本文中文件名为：`liblib_test2.so`）通过ssh上传到运行`runtime`装置的`/usr/lib`目录下。

```sh
sanway@DCA-801:~$ ll /usr/lib/liblib_test2.so 
-rw-rw-r-- 1 sanway sanway 77384 Jul 30 13:55 /usr/lib/liblib_test2.so
```

### 7.2. 停止`runtime`守护进程：

```
sudo /etc/init.d/codesyscontrol stop
```

### 7.3. 修改`runtime`的配置文件：

```sh
vi /etc/codesyscontrol/CODESYSControl_User.cfg
```

在“`[ComponentManager]`”栏中，增加对`lib_test2`动态库的描述。如下所示：

```sh
[ComponentManager]
Component.1=CmpBACnet
Component.2=CmpBACnet2
Component.3=CmpPLCHandler
Component.4=CmpGwClient
Component.5=CmpXMLParser
Component.6=CmpGwClientCommDrvTcp
Component.7=CmpRetain
;Component.8=CmpGwClientCommDrvShm ; enable only if necessary, can result in higher cpu load 
Component.8=lib_test2
```

### 7.4. 重新启动`runtime`守护进程：

```sh
/etc/init.d/codesyscontrol start
```

### 7.5. 查看`runtime`启动日志，看看启动是否成功：

```sh
cat /var/opt/codesys/codesyscontrol.log | grep Dynamic
```

如果启动成功，则日志如下所示：

```sh
...
2025-07-30T06:36:01.673Z, 0x00000001, 1, 0, 10, Dynamic: <cmp>lib_test2</cmp>, <id>0xffff2000</id> <ver>1.0.0.0</ver>
```

表明动态库加载成功了。

---

最后，将CODESYS创建的包含C语言动态库的工程项目加载到`runtime`装置上并运行，显示结果如下：

![image-20250730163433424](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本\codesys相关\images\image-20250730163433424.png)

[测试工程和库的源码工程目录：](/home/jason/BaiduSyncdisk/CodeSys/SANWAY)

