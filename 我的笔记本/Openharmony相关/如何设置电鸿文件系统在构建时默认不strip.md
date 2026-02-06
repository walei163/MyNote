# 如何设置电鸿文件系统在构建时默认不strip

默认电鸿系统在构建文件系统时，会对系统进行strip，这一步在yocto的do_install任务中完成的。如果不想strip，则需要修改：

[/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/conf/local.conf](/media/jason/btrfs_disk_M/Develop/powereuler-embedded/build/powereuler-dgri/conf/local.conf)这个文件。在文件中添加以下内容即可：

```sh
#===========the content is user added==================
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIPS = "1"
```

然后重新构建文件系统：

```sh
[openeuler@mint22 powereuler-dgri]$ bitbake openeuler-image
```

