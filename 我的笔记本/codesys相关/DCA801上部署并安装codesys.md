# DCA801上部署并安装codesys

---

## 1、安装CODESYS 64 3.5.21.10.exe：

在Windows下安装：[/mnt/hgfs/H_DRIVE/Codesys相关_20260728/CODESYS Development System V3/CODESYS 64 3.5.21.10.exe](/mnt/hgfs/H_DRIVE/Codesys相关_20260728/CODESYS Development System V3/CODESYS 64 3.5.21.10.exe)

安装时选择全部安装。

---

## 2、安装库：

必须安装以下两个库：

- **CODESYS Control for Linux ARM64 SL**
- **CODESYS Edge Gateway for Linux**

运行Codesys软件，在系统界面上点击：“工具”，然后选择“CODESYS安装程序”，如下图：

![image-20260130113919651](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130113919651.png)

浏览并搜索上面2个库文件，并安装：

![image-20260130114047776](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130114047776.png)

---

## 3、Deploy Control SL：

### 3.1. 连接DCA801装置：

点击菜单项：

“工具” -> "Deploy Control SL"，在弹出的界面中设置装置的IP地址，用户名和密码（用户名：`sanway`，密码：`sanway1929`），然后连接：

![image-20260130114404839](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130114404839.png)

如果连接成功，会有提示信息如下：

![image-20260130120314309](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130120314309.png)

### 3.2. 将前面安装的2个组件部署到装置上：

![image-20260130114613660](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130114613660.png)

### 3.3. 如果部署成功，则可操作并启动这两项服务：

![image-20260130114756778](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130114756778.png)

此时可以通过ssh登录到DCA801装置上，查看一下Codesys的服务：

```sh
sanway@DCA-801:~$ 
ps ax |grep codesys
  15298 ?        Sl     0:47 /opt/codesysedge/bin/codesysedge.bin /etc/codesysedge/Gateway.cfg
  17238 ?        SLl    0:08 /opt/codesys/bin/codesyscontrol.bin /etc/codesyscontrol/CODESYSControl.cfg
  17473 pts/0    S+     0:00 grep --color=auto codesys
```

**表明Codesys组件已经正确部署到装置并运行了。**

---

## 4、创建新项目验证：

### 4.1. 新建一个标准工程：

![image-20260130115024930](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130115024930.png)

设备和PLC编程选择如下：

![image-20260130115128665](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130115128665.png)

新建项目之后的界面如下：

![image-20260130115231573](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130115231573.png)

### 4.2. 登录DCA801装置，配置网关：

点击菜单：

“在线” -> “登录”，弹出如下界面：

![image-20260130120619547](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130120619547.png)

点击：“是”，弹出设备网关界面：

![image-20260130120812866](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130120812866.png)

此时需要配置网关。点击：“扫描网络”，在弹出的界面中选网关：“dca801”，然后点击：“确定”：

![image-20260130120941892](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130120941892.png)

如果网关激活成功，则界面如下：

![image-20260130121059180](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130121059180.png)

可以看到，网关和设备在右下角均有一个绿色圆形符号，表示连接成功。

> [!important]
>
> **这里表示，新建的设备已经和DCA801装置内的网关建立了正确的连接，可以开始通信了。**

### 4.3. 登录并调试项目代码：

再次点击菜单：

“在线”-> “登录”，弹出以下界面：

![image-20260130121612990](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130121612990.png)

> [!note]
>
> 如果是第一次登录装置，会弹出让你设置用户名和密码的界面，注意这里的用户名和密码不是装置ssh的登录名和密码，而是Codesys网关服务的用户名和密码。
>
> 我已经设置了用户名：`dca801`，密码：`Sanway@1929`（密码默认必须包含大小写，并且还要有特殊字符）。

键入用户名和密码，可能会弹出以下信息界面：

![image-20260130134353784](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130134353784.png)

选择：“是”，当前设备的代码就下载到了装置上。

然后点击菜单：

“调试” -> “启动”，PLC代码就开始运行了。

可以双击：“PLC_PRG(PRG)”来看到代码中各个变量的运行状态，如下图：

![image-20260130134720727](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130134720727.png)

至此，整个安装流程就完成了。

> [!note]
>
> **上面的界面是在线模式下的代码界面，此时不能对代码进行修改、编辑等。**

如果需要编辑PLC代码，则可以右键点击：“PLC_PRG(PRG)”，在弹出的菜单中选择：“编辑对象（离线）”，如下图：

![image-20260130150156525](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130150156525.png)

则会打开代码编辑界面，此时可以对整个代码进行编辑设计了：

![image-20260130150355158](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130150355158.png)

---

## 5、自定义库及库管理器：

上图中可以看到，我们加载了一个自己编写的库：`dca801`，该库中实现了3个函数：

- dca801.cpld_init_cext
- dca801.cpld_exit_cext
- dca801.buzzer_ctrl_cext

这个库及函数的说明可以在：“库管理器”中查看到：

![image-20260130151159203](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260130151159203.png)

如果自定义库：`dca801`加载正确，则运行程序后，会在装置上启动蜂鸣器，此时可以听到蜂鸣器将会按照PLC代码设计的那样进行一段一续的蜂鸣。

关于自定义库的设计方法，可以参照文档：《编写C语言动态库并调用（完整篇）.pdf》。