# FUXI核心板降频测试

2025-02-17： 菲森邓工希望能够将FUXI核心板的功耗降低一些，就咨询能否降频使用。经过测试，可以修改这个头文件：

```sh
/media/jason/btrfs_disk_M/Develop/FUXI-H_20240713/v3.0_sdk_20240810/u-boot-810_sdk_v3.0/u-boot-810/include/configs/fx6evb-common.h
```

其中的定义：

```c
#define CPU_DEFAULT_FREQ  750000000
#define LSP_DEFAULT_FREQ  93750000
#define HSP_DEFAULT_FREQ  187500000

// #define CPU_DEFAULT_FREQ  667000000
// #define LSP_DEFAULT_FREQ  83375000
// #define HSP_DEFAULT_FREQ  166750000
```

将CPU的主频从750MHz降到667MHz。

经过修改后的CPU核心板的功耗：

| CPU主频 | 功耗 | 备注              |
| ------- | ---- | ----------------- |
| 750MHz  | 2.7W | SWAS831核心板     |
| 667MHz  | 2.5W | SWAS831核心板     |
| 750MHz  | 2.8W | FUXI-H2官方核心板 |

U-boot810编译方法：

```sh
$ cat sh_csky 
#!/bin/sh

make CROSS_COMPILE=csky-abiv2-linux- -j8


$ cat make_uboot_img_csky 
#!/bin/sh

CMD_DIR_PATH=/media/jason/btrfs_disk_M/Develop/FUXI-H_20240713/v3.0_sdk_20240810/tools
echo ${CMD_DIR_PATH}/makefw -l0 -f 0x00 -r 0xa04c0000 -o u-boot-spl.brn -p spl/u-boot-spl.bin
${CMD_DIR_PATH}/makefw -l0 -f 0x00 -r 0xa04c0000 -o u-boot-spl.brn -p spl/u-boot-spl.bin
```

