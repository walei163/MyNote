# Debian11安装配置相关

## 2025-11-18：

### typora修改系统字体：

#### 1、修改正文字体

偏好设置 → 外观 → 打开主题文件夹，里面的css文件即为主题文件

为了防止修改错误，或者保留原来的github.css，我们复制出一个[my_github.css](https://zhida.zhihu.com/search?content_id=240889968&content_type=Article&match_order=1&q=my_github.css&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NjM2MjY4MjQsInEiOiJteV9naXRodWIuY3NzIiwiemhpZGFfc291cmNlIjoiZW50aXR5IiwiY29udGVudF9pZCI6MjQwODg5OTY4LCJjb250ZW50X3R5cGUiOiJBcnRpY2xlIiwibWF0Y2hfb3JkZXIiOjEsInpkX3Rva2VuIjpudWxsfQ.t52oVT7qLy7D8TADidGKHUzuJJmalPUAXK8uPGxXZ0o&zhida_source=entity)

将字体文件`LXGWWenKai-Regular.ttf`放在特定目录，此处我放在新建的font目录下

修改my_github.css，在其中新定义一个@font-face

```css
@font-face {
    font-family: 'LXGWWenKai';
    font-style: normal;
    font-weight: normal;
    src: url("font/LXGWWenKai-Regular.ttf");  /*字体路径*/
}
```

搜索`body {`, 在`font-family:`前两项添加字体

```css
body {
    font-family: "Consolas", "LXGWWenKai", "Open Sans", "Clear Sans", "Helvetica Neue", Helvetica, Arial, 'Segoe UI Emoji', sans-serif;
    color: rgb(51, 51, 51);
    line-height: 1.6;
}
```

修改图如下：

![img](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/v2-9d7ece337dba9669f34aea3b0159a438_1440w.jpg)



此时重启typora，并将外观设置为mygithub，可看到正文字体的更改（代码块字体并未更改）

#### 2、修改代码块字体

修改my_github.css，搜索code，在`tt {`下加上`font-family`字段

```css
.md-fences,
code,
tt {
    border: 1px solid #e7eaed;
    background-color: #f8f8f8;
    border-radius: 3px;
    padding: 0;
    padding: 2px 4px 0px 4px;
    font-size: 0.9em;
    font-family: "Consolas", "LXGWWenKai";
}
```

修改图如下：



![img](/home/jason/BaiduSyncdisk//VNote笔记本_20200401/我的笔记本/debian12相关/v2-dba54613d66aafdf8f550027bf92803c_1440w.jpg)

此时重启typora，可看到代码段字体的更改

#### 3、修改标题字体

修改my_github.css，搜索header，在`font-family:`前两项添加字体

```css
header,
.context-menu,
.megamenu-content,
footer {
    font-family: "Consolas", "LXGWWenKai", "Segoe UI", "Arial", sans-serif;
}
```

修改图如下：



![img](/home/jason/BaiduSyncdisk//VNote笔记本_20200401/我的笔记本/debian12相关/v2-4535e703b75285d7fd74a80399d5ccb8_1440w.jpg)

此时重启typora，可看到左侧列表字体的更改。但此时，软件/编辑器的字体还是原来的，但平常可忽略，就懒得搞了哈哈哈。

---

## 2025-11-14：

typora的采用源的方式安装方法：

```sh
# add Typora's key

sudo mkdir -p /etc/apt/keyrings

curl -fsSL https://typoraio.cn/linux/typora.gpg | sudo tee /etc/apt/keyrings/typora.gpg > /dev/null

# add Typora's repository securely

echo "deb [signed-by=/etc/apt/keyrings/typora.gpg] https://typoraio.cn/linux ./" | sudo tee /etc/apt/sources.list.d/typora.list

sudo apt update

# install typora

sudo apt install typora
```



---

## 2025-11-09:

我在安装了Sparky 7.8后，发现系统自带的apt在更新时总是把我安装好的vscode降级版本。此时就需要将我自己已经安装好的高版本的vscode在apt下标记为保留：

```sh
sudo apt-mark hold code
```

这样在apt更新升级时，就不会将vscode降级了。

> [!tip] 
>
> 查看系统中被标记为hold的软件：
>
> ```sh
> apt-mark showhold
> ```
>
> 

---

## 2025-11-05：

### 修改xfce4的菜单项：

需要安装：

```sh
sudo apt-get install menulibre
```

然后可以在“开始”菜单中 -> “附件” -> “菜单编辑器”中去修改相关的菜单项配置。

> [!important]
>
> <font face="微软雅黑" color=cyan>其实，菜单项中的这些内容全在/usr/share/applications中，也就是/usr/share/applications的那些.desktop文件。
> 编辑这些.desktop文件就会发现，每个文件中都有Categories=，它决定了在那个菜单下显示。</font>
>
> <font face="微软雅黑" color=cyan>如：
> Categories=Application;Network; ＃就会在网络菜单里显示</font>
>
> <font face="微软雅黑" color=cyan>Categories=Application;Office; ＃就会在办公菜单里显示
> 以此类推...</font>

---

## 2025-10-29:

### 安装master-pdf-editor-5：

根据官方网站的说明，可以直接通过apt增加源来安装：

第一步：

```sh
wget --quiet -O - http://repo.code-industry.net/deb/pubmpekey.asc | sudo tee /etc/apt/keyrings/pubmpekey.asc
```

第二步：

```sh
echo "deb [signed-by=/etc/apt/keyrings/pubmpekey.asc arch=$( dpkg --print-architecture )] http://repo.code-industry.net/deb stable main" | sudo tee /etc/apt/sources.list.d/master-pdf-editor.list
```

第三步：

```sh
sudo apt update
sudo apt install master-pdf-editor-5
```



---

## 1、  设置全局变量

和ubuntu不同，不能直接修改/etc/environment文件，需要修改/home/jason/.bashrc，如下所示：

```sh
# Add sbin directories to PATH.  This is useful on systems that have sudo
echo $PATH | grep -Eq "(^|:)/sbin(:|)"     || PATH=$PATH:/sbin
echo $PATH | grep -Eq "(^|:)/usr/sbin(:|)" || PATH=$PATH:/usr/sbin

PATH=$PATH:/media/ubuntu16.04/opt/gcc-arm-10.2-2020.11-x86_64-arm-none-linux-gnueabihf/bin:/media/ubuntu16.04/opt/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin:/media/jason/develop/fuxi_20220225/toolchains/bin

# If this is an xterm set the title to user@host:dir
```

## 2、 启动自动挂载需要的磁盘（采用UUID方式）

先查看磁盘的UUID：

```sh
ls -l /dev/disk/by-uuid
```

可以查看各个磁盘的UUID。

也可以采用blkid命令来查看：

```sh
jason@wanglei:~$ blkid
/dev/sdf2: UUID="4372-88B2" BLOCK_SIZE="512" TYPE="vfat" PARTLABEL="EFI System Partition" PARTUUID="a3693431-a7f2-49de-955c-cc1cab900783"
/dev/sdf3: UUID="4e69a0d9-0d22-49ab-86b6-7be5819f7f5e" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="e1a90255-e0b5-4ab7-b737-653e4a4a6a15"
/dev/sdf4: UUID="b45f198a-2c45-4e3a-af3a-dda03296cfaf" BLOCK_SIZE="512" TYPE="xfs" PARTUUID="ba0326d8-49fd-43b4-86aa-a87071518273"
/dev/sdd1: LABEL="develop" UUID="eafb369d-2e4f-40f5-a9f8-6109b8db73ad" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="d1361f9c-2174-4db0-9c9c-9e14d4c2809c"
/dev/sdb2: LABEL="ubuntu13.04" UUID="529758d8-41d0-4750-a3f1-c2897bbf2676" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="000e7e0a-02"
/dev/sde1: LABEL="Rockchip1" UUID="f4a0725b-d4ba-46da-a9ab-12f6f2f86a19" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="923e98a4-1e42-4914-9bb5-31f15acdcf89"
/dev/sdc2: UUID="8059b2c3-4b9c-40ab-ad76-56dd9d3274cd" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="cc10d4f3-02"
/dev/sda1: UUID="ee9bcd48-03e9-4f4e-bc11-19ce637434aa" BLOCK_SIZE="4096" TYPE="ext4" PARTUUID="132d2029-01"
```

然后可以修改/etc/fstab：

```sh
UID=529758d8-41d0-4750-a3f1-c2897bbf2676	/media/ubuntu13.04	ext4	errors=remount-ro	0	1
UUID=8059b2c3-4b9c-40ab-ad76-56dd9d3274cd	/media/ubuntu16.04	ext4	errors=remount-ro	0	1
```

保存并重启系统即可。

## 3、修改apt为国内的源：

```sh
deb https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye main contrib non-free
# deb-src https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye main contrib non-free
deb https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-updates main contrib non-free
# deb-src https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-updates main contrib non-free
deb https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-backports main contrib non-free
# deb-src https://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-backports main contrib non-free
deb https://mirrors.tuna.tsinghua.edu.cn/debian-security bullseye-security main contrib non-free
# deb-src https://mirrors.tuna.tsinghua.edu.cn/debian-security bullseye-security main contrib non-free
```

## 4、解决关机时间过长的问题：

关机的默认等待时间通常设置为 90 秒。在这个时间之后，你的系统会尝试强制停止服务。

如果你想让你的 Linux 系统快速关闭，你可以改变这个等待时间。

你可以在位于 `/etc/systemd/system.conf` 的配置文件中找到所有的 systemd 设置。这个文件中应该有很多以 `#` 开头的行。它们代表了文件中各条目的默认值。

在开始之前，最好先复制一份原始文件。

```sh
sudo cp /etc/systemd/system.conf /etc/systemd/system.conf.orig
```

在这里寻找 `DefaultTimeoutStopSec`。它可能被设置为 90 秒。

```sh
#DefaultTimeoutStopSec=90s
```

你得把这个值改成更方便的，比如 5 秒或 10 秒。

```sh
DefaultTimeoutStopSec=5s
```

如果你不知道如何在终端中编辑配置文件，可以使用这个命令在系统默认的文本编辑器（如 Gedit）中打开文件进行编辑：

```sh
sudo xdg-open /etc/systemd/system.conf
```

![image-20241228151901795](/media/sf_E_DRIVE/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/image-20241228151901795.png)

不要忘记删除 DefaultTimeoutStopSec 前的 `#` 号。保存文件并重启系统。

这将帮助你减少 Linux 系统的关机延迟。

## 5、 如何挂载LVM卷文件系统：

新的Trisquel Mini（11.0）安装时采用了LVM卷文件系统，不能直接用mount命令来挂载，需要做以下几步工作：

### 1）安装LVM2：

```sh
sudo apt install lvm2
```

### 2）扫描卷：

```sh
sudo vgscan
Found volume group "vgtrisquel" using metadata type lvm2
```

### 3）激活扫描到的卷：

```sh
sudo vgchange -a y  vgtrisquel
3 logical volume(s) in volume group "vgtrisquel" now active
```

此时可以在/dev/目录下看到这个卷设备对应的目录：

```sh
$ ll /dev/vgtrisquel/

总用量 0
lrwxrwxrwx 1 root root 7  4月  3 20:07 home -> ../dm-2
lrwxrwxrwx 1 root root 7  4月  3 20:07 root -> ../dm-0
lrwxrwxrwx 1 root root 7  4月  3 20:07 swap_1 -> ../dm-1
```

### 4）挂载卷：

```sh
sudo mount /dev/vgtrisquel/home /mnt
```

这样就可以访问LVM上的卷文件了。

## 6、设置lxde自动登录：

编辑`/etc/lightdm/lightdm.conf`文件：

```sh
[Seat:*]
user-session=lxde
autologin-user=jason
autologin-user-timeout=0
```

这样就设置好了从lightdm自动登录了。

## 7、设置Samba服务：

安装完成samba服务后，打开`/etc/samba/smb.conf`，添加以下内容（或者直接打开/etc/samba/smb.conf，在global字段中添加以下内容）：

```sh
 usershare allow guests = yes
   force user = jason
   force group = jason
   create mask = 0664
   directory mask = 0775
   usershare owner only = false
#======================= Share Definitions =======================
```

然后重启服务即可。

> [!note]
>
> **除了重启服务之外，还需要将当前用户加入到共享组中才行**：
>
> ```sh
> sudo adduser jason sambashare
> ```
>
> 

在Linux系统下，可以安装nautilus或者caja文件管理器，这样可以通过图形方式来共享文件夹。

## 8、设置nfs网络服务：

安装完成`nfs-kernel-server`后，需要修改`/etc/default/nfs-kernel-server`，添加以下内容：

```sh
RPCNFSDOPTS="--nfs-version 2,3,4 --debug –syslog"
```

然后再修改`/etc/exports`文件，将需要共享的目录添加，例如如下：

```sh
/media/ubuntu13.04/home/wanglei/arm/am335x/cce-307/rootfs_linaro_4.9     *(rw,sync,no_root_squash,no_subtree_check)
/media/ubuntu13.04/home/wanglei/arm/STM32MPU_workspace/swa1530_200318/images  *(rw,sync,no_root_squash,no_subtree_check)
/media/ubuntu16.04/mnt/fuxi_ck860fv     						*(rw,sync,no_root_squash,no_subtree_check)
/media/ubuntu16.04/mnt/armhf_linux         						*(rw,sync,no_root_squash,no_subtree_check)
/media/ubuntu16.04/mnt/rootfs10.2_linaro10.2_qt5.12.8			*(rw,sync,no_root_squash,no_subtree_check)
/media/ubuntu16.04/mnt/sanway-emmc-rootfs-4.9-stm32mp153a-swa1530	*(rw,sync,no_root_squash,no_subtree_check)
```

然后重启服务：

```sh
sudo service nfs-kernel-server restart
```

如果服务启动成功，则可以通过如下命令看到共享出来的目录：

```sh
$ sudo exportfs 
/media/ubuntu13.04/home/wanglei/arm/am335x/cce-307/rootfs_linaro_4.9	​		<world>
/media/ubuntu13.04/home/wanglei/arm/STM32MPU_workspace/swa1530_200318/images	​		<world>
/media/ubuntu16.04/mnt/fuxi_ck860fv				​		<world>
/media/ubuntu16.04/mnt/armhf_linux				​		<world>
```

## 9、设置thunar挂载其它硬盘分区时不用输入root密码：

可以直接修改：`/usr/share/polkit-1/actions/org.freedesktop.UDisks2.policy`文件，找到以下内容：

```sh
<action id="org.freedesktop.udisks2.filesystem-mount-system">
```

将其中的<default></default>段修改为如下内容：

```sh
<defaults>
      <allow_any>auth_admin</allow_any>
      <allow_inactive>auth_admin</allow_inactive>
      <allow_active>yes</allow_active>
</defaults>
```

