# 刷写eMMC或TF卡映像

eMMC存储分布图

![image-20250524100523369](/home/jason/BaiduSyncdisk/VNote笔记本_20200401//我的笔记本/全志T113-i开发相关/images/image-20250524100523369.png)

1、刷写uboot：

```sh
dd if=boot_package.fex of=/dev/mmcblk1 conv=notrunc seek=32800 bs=512
dd if=boot0_sdcard.fex of=/dev/mmcblk1 conv=notrunc seek=16 bs=512
```

