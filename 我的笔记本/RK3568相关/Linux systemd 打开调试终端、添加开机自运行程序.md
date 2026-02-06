## [Linux systemd 打开调试终端、添加开机自运行程序](https://www.cnblogs.com/zengjfgit/p/6097078.html)

/*************************************************************************
 *           Linux systemd 打开调试终端、添加开机自运行程序
 * 说明：
 *     听说Linux Systemd启动流程要替代以前的SysV init启动流程，于是了解一
 * 下Systemd工作机制，在ARM产品上，打开调试终端和添加开机自启动程序是用
 * 得最频繁的两个功能，不过目前还没有测试过，不可全信。
 * 
 *                                     2016-11-24 深圳 南山平山村 曾剑锋
 ************************************************************************/

一、参考文档：
    1. 走进Linux之systemd启动过程
        http://www.codeceo.com/article/linux-systemd-start.html
    2. 浅析 Linux 初始化 init 系统，第 3 部分: Systemd
        https://www.ibm.com/developerworks/cn/linux/1407_liuming_init3/
    3. Systemd FAQ (简体中文)
        https://wiki.archlinux.org/index.php/Systemd_FAQ_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)
    4. ARCH SYSTEMD 开机自启动脚本
        http://lvpro.wpblog.jp/?p=14

二、打开调试终端（debug console）：
    

```sh
ln -sf /usr/lib/systemd/system/getty@.service /etc/systemd/system/getty.target.wants/getty@ttymxc0.service
```

   

```sh
systemctl start getty@ttymxc0.service
```

三、添加开机自启动程序：
    移步参考文档开机自启动脚本中的说明，因为没实际测试。 :)