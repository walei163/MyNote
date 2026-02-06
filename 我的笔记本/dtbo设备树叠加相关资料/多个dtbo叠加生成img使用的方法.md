# 多个dtbo叠加生成img使用的方法

# SSU_DTBO功能介绍

## 1. DTB overlay介绍[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#1-dtb-overlay)

### 1.1 DTB overlay作用介绍[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#11-dtb-overlay)

DTB overlay是一种对设备树做叠加的功能，由libfdt中的fdt_overlay.c提供支持（https://github.com/dgibson/dtc），其核心功能函数：



```c
// <U-boot>/scripts/dtc/libfdt/fdt_overlay.c
int fdt_overlay_apply(void *fdt, void *fdto)
```

目前dtb overlay功能在各个厂家基本都是用于对U-boot或者Kernel的设备树做叠加修改。

例如对于SigmaStar的pioneer5 SOC平台，有SSC028A-S01A-S、SSZ028A-S01A-M、SSC028B-S01A-S等多种开发板，在配置设备树过程中，可以：

1. 将SOC相关的设备树配置集合在主设备树，编译出pioneer5.dtb
2. 将不同开发板存在差异的的设备树配置集合到dtbo中编译出SSC028A-S01A-S.dtbo、SSZ028A-S01A-M.dtbo、SSC028B-S01A-S.dtbo
3. 将这些dtbo按一定格式打包生成dtbo.img

在实际启动U-boot或者Kernel中，所有开发板都使用含SOC公共配置的设备树pioneer5.dtb，对于SSC028B-S01A-S会将dtbo.img中的SSC028B-S01A-S.dtbo取出执行：



```sh
fdt_overlay_apply(pioneer5.dtb, SSC028B-S01A-S.dtbo);
```

以得到支持开发板SSC028B-S01A-S的设备树。

### 1.2 dtbo的语法及编译介绍[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#12-dtbo)

FDT的相关语法请参考：https://github.com/devicetree-org/devicetree-specification/releases/tag/v0.3

首先本质上dtbo其实也是一个dtb文件，都是通过dtc（device tree compiler）工具编译出来的，它们甚至使用的dtc编译命令都是一样的。

常见的dtc工具用法：

```sh
./dtc -@ -I <input format> -O <output format> -o <output file name>  <input file>
```

例如编译一个dtb：

```sh
./dtc -@ -I dts -O dtb -o pionner5.dtb pionner5.dts
```

编译一个dtbo

```sh
./dtc -@ -I dts -O dtb -o SSC028A-S01A-S.dtbo SSC028A-S01A-S.dts
```

可见在编译dtb和dtbo时的“input format”均是dts，”output format“均是dtb。

dtbo和dtb的主要差异在其dts的写法上，dtbo的dts需要添加`/plugin/;`以表示该文件在编译时允许对不存在的解决做未定义引用：

```c
/dts-v1/;
/plugin/;
```

形如：

```c
// my_main_dt.dts

/dts-v1/;

/ {
    my_node: node@0 {
        status = "disabled";

        my_child: child@0 {
            value = <0xffffffff>;
        };
    };
};
```



```c
// my_overlay_dt.dts

/dts-v1/;
/plugin/;

&my_node {
    status = "okay";
};

&my_child {
    value = <0x1>;
};
```

DTB overlay是以node为单位的，并且需要被overlay的node需要在其node name前添加`<label name>:`，例如本例中的`my_node`。在dtbo中则是通过`&<lable name>`来引用要被overlay的节点，再去覆盖、新增property或者新增子节点。

其原理是：当调用dtc编译dtb时，如果额外地传入了"-@"参数，dtc会在最终的dtb中生成`__symbols__`节点，该节点中会收集所有带label的节点的在设备树中位置信息，格式`<label name> = "<node path>";`，形如：

```c
__symbols__ {
    sound = "/soc/sound";
    bach1 = "/soc/bach1";
    bach2 = "/soc/bach2";
};
```

dtc也可以用来反编译，将dtb或者dtbo反编译为dts，形如：

```sh
./dtc -@ -I dtb -O dts -o pionner5.dts pionner5.dtb
```

### 1.3 DTB Overlay支持的行为[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#13-dtb-overlay)

#### 1.3.1 Overriding（覆盖）[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#131-overriding)

```c
// my_main_dt.dts

/dts-v1/;

/ {
    compatible = "corp,foo";

    my_node: node@0 {
        status = "disabled";
    };
};
```

```c
// my_overlay_dt.dts

/dts-v1/;
/plugin/;

&my_node {
    status = "okay";
};
```

Merge Result:

```c
/dts-v1/;

/ {
    compatible = "corp,foo";
      ......

      node@0 {
        linux,phandle = <0x1>;
        phandle = <0x1>;
        status = "okay";
      };
};
```

#### 1.3.2 Appending（追加）[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#132-appending)

```c
// my_main_dt.dts

/dts-v1/;

/ {
    compatible = "corp,foo";

    my_node: node@0 {
        status = "okay";
    };
};
```

```c
// my_overlay_dt.dts

/dts-v1/;
/plugin/;

&my_node {
    new_prop = "bar";
};
```

Merge Result：

```c
/dts-v1/;

/ {
    compatible = "corp,foo";

    ......

      node@0 {
        linux,phandle = <0x1>;
        phandle = <0x1>;
        status = "okay";
        new_prop = "bar";
      };
};
```

#### 1.3.3 Child nodes（子节点的新增、追加、覆盖）[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#133-child-nodes)

```c
// my_main_dt.dts

/dts-v1/;

/ {
    compatible = "corp,foo";

    my_nodes: nodes {
        compatible = "corp,bar";

        node@0 {
              status = "disabled";
        };
      };
};
```

```c
// my_overlay_dt.dts

/dts-v1/;
/plugin/;

&my_nodes {
     new_prop1 = "abc";

      node@0 {
        status = "okay";
        new_prop2 = "xyz";
      };
};
```

Merge Result：

```c
/dts-v1/;

/ {
      compatible = "corp,foo";

      ......

      nodes {
        linux,phandle = <0x1>;
        phandle = <0x1>;
        compatible = "corp,bar";
        new_prop1 = "abc";

        node@0 {
              linux,phandle = <0x2>;
              phandle = <0x2>;
              status = "okay";
              new_prop2 = "xyz";
        };
      };
};
```

## 2. SigmaStar平台DTB overlay实现[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#2-sigmastardtb-overlay)

### 2.1 DTB overlay相关的Kconfig[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#21-dtb-overlaykconfig)

**U-boot：**

SigmaStar平台的DTB overlay都是在U-boot中完成的，所其开关以及相关的配置都是在U-boot的Kconfig中配置：

```sh
#arch/arm/mach-sstar/Kconfig

config SSTAR_BOOT_DEV
    string "boot device"
    default "mmc"
    help
      specify boot device type

config SSTAR_BOOT_DEV_ID
    int "boot device id"
    default 0
    help
      specify boot device id

config SSTAR_ENV_PART_NAME
    string "env patition name"
    default "env"
    help
      secify the real 'env' partition name in partition table

config SSTAR_DTBO_PART_NAME
    string "dtbo patition name"
    default "dtbo"
    help
      secify the real 'dtbo' partition name in partition table

config SSTAR_UBOOT_DTB_OVERLAY
    bool "Uboot DTB overlay"
    select SSTAR_DTB_OVERLAY_SUPPORT
    default n
    help
      Select this config to enable uboot dtb overlay feature, this feature
      will load fdt table from 'dtbo' partition. all dtbo specify in env
      'uboot_dtbo_list' will be overlay to uboot dtb. If your driver cares
      about the result of dtb overlay, please try to get property from
      the device node after 'board_init'.

config SSTAR_KERNEL_DTB_OVERLAY
    bool "Kernel DTB overlay"
    select SSTAR_DTB_OVERLAY_SUPPORT
    default n
    help
      Select this config to enable kernel dtb overlay feature, this feature
      will load fdt table from 'dtbo' partition. all dtbo specify in env
      'kernel_dtbo_list' will be overlay to kernel dtb.

config SSTAR_DTB_OVERLAY_SUPPORT
    bool
    select OF_LIBFDT_OVERLAY
    select CMD_ADTIMG
    default n

config SSTAR_BUILTIN_KERNEL_DTB
    bool "Built-in dtb support"
    default y
    help
      When Kernel use SigmaStar built-in dtb, uboot should pass
      atags instead fdt to kernel from x0.
```

每个Kconfig的作用请参考其help信息，其中`CONFIG_SSTAR_BUILTIN_KERNEL_DTB`这一项的作用是将对builtin DTB的支持关闭。

**Kernel**：

Kernel涉及的Kconfig并不多，最主要的是将builtin DTB的功能关闭，使Kernel接收并使用由U-boot传递过来的设备树。

```sh
config SSTAR_BUILTIN_DTB
    bool "SigmaStar built-in DTB"
    default y
```

### 2.2 U-boot DTB overlay流程[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#22-u-boot-dtb-overlay)

U-boot中的 **CONFIG_SSTAR_UBOOT_DTB_OVERLAY** 用于使能U-boot DTB overlay功能，其在启动过程中的大致被调用的位置如下图：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/dtbo设备树叠加相关资料/image-20221129202406194.png)

如上图，SigmaStar平台的U-boot DTB overlay并非在前级loader做好dtb overlay再将merged dtb传给U-boot。而是在U-boot init早期，storage设备准备好之后（dm_init后）根据U-boot env **"uboot_dtbo_list"** 的设定立即执行U-boot DTB overlay，因为U-boot的dtb在overlay之后会发生变化，所以在U-boot DTB overlay之后会再做一次dm_init。

详细代码流程参考：

```sh
arch/arm/mach-sstar/board_common.c #board_uboot_dtb_overlay
```

### 2.3 Kernel DTB overlay流程[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#23-kernel-dtb-overlay)

U-boot中的 **CONFIG_SSTAR_KERNEL_DTB_OVERLAY** 用于使能Kernel DTB overlay功能，其在启动过程中的大致被调用的位置如下图：

![img](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/mymedia/DTB_overlay_feature_introduction/image-20221129203838495.png)

如上图，SigmaStar的Linux和Android平台在启动Kernel时，虽然入口不一样，但是在准备好Kernel、DTB、Rootfs之后，都会通过booti/bootm命令启动Kernel，最终都是走一下流程启动Kernel：

```sh
arch/arm/lib/bootm.c #do_bootm_linux
arch/arm/lib/bootm.c #boot_prep_linux
    -> arch/arm/mach-sstar/board_common.c #sstar_prep_linux
        -> arch/arm/mach-sstar/board_common.c #board_kernel_dtb_overlay
arch/arm/lib/bootm.c #boot_jump_linux
```

可以看到，在跳Kernel之前（boot_jump_linux），会在board_kernel_dtb_overlay中会根据U-boot env **"kernel_dtbo_list"** 的设定对即将传递给kernel的dtb做dtb overylay动作，详细流程请参考：

```sh
arch/arm/mach-sstar/board_common.c #board_kernel_dtb_overlay
```

### 2.4 uboot_dtbo_list和kernel_dtbo_list[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#24-uboot_dtbo_listkernel_dtbo_list)

前文有提到，这两个uboot的env是U-boot DTB Overlay和Kernel DTB overlay流程中用于列出有哪几个dtbo要从dtbo.img中取出来做Overlay。这里涉及到libfdt做DTB overlay过程中的一个特性：dtbo源文件中可以指定根节点"/"的"compatible"属性，但是实际做DTB overlay过程中，dtbo中的该属性不会覆盖掉主设备树的根节点"/"的"compatible"属性。

基于这个特性，我们可以根据每个dtbo的作用设定其根节点"/"的"compatible"属性，形如：

```c
/ {
    compatible = "sstar_dtbo,ssz028a_s01a_m";
};

&padmux {
    schematic = <xxxxx>;
}
```

从dtbo.img找dtbo时通过该property进行查找就不需要担心诸如通过index取dtbo自己的dtbo放在dtbo.img中某个index，某一天index突然变化取出错误dtbo的问题。所以这两个env的写法形如：

```c
sstar_dtbo,ssz028a_s01a_m;sstar_dtbo,panel-RM68200;...
```

### 2.5 dtbo.img介绍[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#25-dtboimg)

在第1.1章中有提到SigmaStar平台会有一个dtbo.img用于包含支持各种规格板子的dtbo.img。通过1.2章的介绍已经了解到dtc编译出来的.dtb(.dtbo)文件其实就是单独的一个dtb文件，为了能够包含多个dtb文件到一个dtbo.img中，我们选择使用Android的mkdtimg工具来制作dtimg。dtimg的格式大致如下：

![img](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/mymedia/DTB_overlay_feature_introduction/treble_dto_partition_layout.png)

核心信息就在于位于镜像头的dt_table_header以及dt_table_entry，dt_table_header用于描述该镜像的基本信息，以及该镜像包含的dt_table_entry相关信息，dt_table_entry则是描述了每个包含在该镜像中的dtb的位置信息以及一些标记信息：

```c
#define DT_TABLE_MAGIC 0xd7b7ab1e

struct dt_table_header {
  uint32_t magic;             // DT_TABLE_MAGIC
  uint32_t total_size;        // includes dt_table_header + all dt_table_entry
                              // and all dtb/dtbo
  uint32_t header_size;       // sizeof(dt_table_header)

  uint32_t dt_entry_size;     // sizeof(dt_table_entry)
  uint32_t dt_entry_count;    // number of dt_table_entry
  uint32_t dt_entries_offset; // offset to the first dt_table_entry
                              // from head of dt_table_header

  uint32_t page_size;         // flash page size we assume
  uint32_t version;       // DTBO image version, the current version is 0.
                          // The version will be incremented when the
                          // dt_table_header struct is updated.
};

struct dt_table_entry {
  uint32_t dt_size;
  uint32_t dt_offset;         // offset from head of dt_table_header

  uint32_t id;                // optional, must be zero if unused
  uint32_t rev;               // optional, must be zero if unused
  uint32_t custom[4];         // optional, must be zero if unused
};
```

制作dtbo.img的mkdtimg的用法如下：

```sh
mkdtimg create <image_filename> (<global-option>...) \
    <ftb1_filename> (<entry1_option>...) \
    <ftb2_filename> (<entry2_option>...) \
    ...
```

其中entryX_option的写法如下：

```sh
--id=<number|path>
--rev=<number|path>
--custom0=<number|path>
--custom1=<number|path>
--custom2=<number|path>
--custom3=<number|path>
```

### 2.6 dtbo以及dtbo.img制作[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#26-dtbodtboimg)

#### 2.6.1 Linux[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#261-linux)

采用命令：mkdtimg配合dtboimg.cfg来制作多个dtbo的叠加img。

注：本章提供的diff修改没有完全覆盖到所有的情况下的log打印，仅作抛砖引玉，读者需要根据自己遇到的情况进行完善。

### 3.2 U-boot命令adt[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#32-u-bootadt)

adt命令可以直接执行不带任何参数获取帮助：

```sh
SigmaStar # adtimg
adtimg - manipulate dtb/dtbo Android image

Usage:
adtimg addr <addr> - Set image location to <addr>
adtimg dump        - Print out image contents
adtimg get dt --index=<index> [avar [svar]]         - Get DT address/size by index

Legend:
  - <addr>: DTB/DTBO image address (hex) in RAM
  - <index>: index (hex/dec) of desired DT in the image
  - <avar>: variable name to contain DT address (hex)
  - <svar>: variable name to contain DT size (hex)
```

根据帮助信息，可以执行如下操作：

```sh
SigmaStar # mmc read 0x21000000 0x0004602a 0x2000

MMC read: dev # 0, block # 286762, count 8192 ... 8192 blocks read: OK
SigmaStar # adt addr 0x21000000
SigmaStar # adt dump
dt_table_header:
               magic = d7b7ab1e
          total_size = 19853
         header_size = 32
       dt_entry_size = 32
      dt_entry_count = 10
   dt_entries_offset = 32
           page_size = 2048
             version = 0
dt_table_entry[0]:
             dt_size = 2707
           dt_offset = 352
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 2707
     (FDT)compatible = sstar_dtbo,ssz028a_s01a_m
dt_table_entry[1]:
             dt_size = 123
           dt_offset = 3059
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 123
     (FDT)compatible = sstar_dtbo,ssc028b_s01a_s
dt_table_entry[2]:
             dt_size = 3014
           dt_offset = 3182
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 3014
     (FDT)compatible = sstar_dtbo,ssz028b_s01a_s
dt_table_entry[3]:
             dt_size = 1467
           dt_offset = 6196
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 1467
     (FDT)compatible = sstar_dtbo,panel-SAT070AT50H18BH
dt_table_entry[4]:
             dt_size = 1480
           dt_offset = 7663
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 1480
     (FDT)compatible = sstar_dtbo,panel-BT601OUTPUT_720x480
.....
dt_table_entry[9]:
             dt_size = 1690
           dt_offset = 18163
                  id = 00000000
                 rev = 00000000
           custom[0] = 00000000
           custom[1] = 00000000
           custom[2] = 00000000
           custom[3] = 00000000
           (FDT)size = 1690
     (FDT)compatible = sstar_dtbo,panel-DV150X0M
SigmaStar #
```

### 3.3 U-boot命令fdt命令[¶](https://wx.comake.online/doc/doc/Opera_DLM00V2.0.0_dispcam_sigdoc_zh/platform/Android/DTB_overlay_feature_introduction.html#33-u-bootfdt)

dt命令可以直接执行不带任何参数获取帮助：

```sh
fdt - flattened device tree utility commands

Usage:
fdt addr [-c]  <addr> [<length>]   - Set the [control] fdt location to <addr>
fdt apply <addr>                    - Apply overlay to the DT
fdt move   <fdt> <newaddr> <length> - Copy the fdt to <addr> and make it active
fdt resize [<extrasize>]            - Resize fdt to size + padding to 4k addr + some optional <extrasize> if needed
fdt print  <path> [<prop>]          - Recursive print starting at <path>
fdt list   <path> [<prop>]          - Print one level starting at <path>
fdt get value <var> <path> <prop>   - Get <property> and store in <var>
fdt get name <var> <path> <index>   - Get name of node <index> and store in <var>
fdt get addr <var> <path> <prop>    - Get start address of <property> and store in <var>
fdt get size <var> <path> [<prop>]  - Get size of [<property>] or num nodes and store in <var>
fdt set    <path> <prop> [<val>]    - Set <property> [to <val>]
fdt mknode <path> <node>            - Create a new node after <path>
fdt rm     <path> [<prop>]          - Delete the node or <property>
fdt header [get <var> <member>]     - Display header info
                                      get - get header member <member> and store it in <var>
fdt bootcpu <id>                    - Set boot cpuid
fdt memory <addr> <size>            - Add/Update memory node
fdt rsvmem print                    - Show current mem reserves
fdt rsvmem add <addr> <size>        - Add a mem reserve
fdt rsvmem delete <index>           - Delete a mem reserves
fdt chosen [<start> <end>]          - Add/update the /chosen branch in the tree
                                        <start>/<end> - initrd start/end addr
NOTE: Dereference aliases by omitting the leading '/', e.g. fdt print ethernet0.
```

结合前面adt命令，可以做如下操作查看dtbo.img中某个dtbo的内容：

```sh
SigmaStar # adtimg get dt --index=0
0x21000160, 0xa93 (2707)
SigmaStar # fdt list /
/ {
    compatible = "sstar_dtbo,ssz028a_s01a_m";
    __fixups__ {
    };
    fragment@0 {
    };
    fragment@1 {
    };
    fragment@2 {
    };
};
```

再例如需要看U-boot dtb时，可以通过U-boot环境变量`fdtcontroladdr`获取到U-boot dtb在内存中的位置后，使用fdt命令查看内容。