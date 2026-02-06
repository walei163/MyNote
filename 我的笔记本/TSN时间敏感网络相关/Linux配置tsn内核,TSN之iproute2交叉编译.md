# Linux配置tsn内核,TSN之iproute2交叉编译

![img](https://img-home.csdnimg.cn/images/20240711042549.png)本文介绍了如何为imx8mp-evk开发板交叉编译iproute2与iptables的过程。首先说明了iproute2的作用及其在Linux网络管理中的重要性，随后详细记录了交叉编译iproute2的具体步骤，最后解决了缺少动态链接库的问题，通过交叉编译iptables获得所需文件。

原文链接：https://blog.csdn.net/weixin_34931370/article/details/116648005?spm=1001.2101.3001.6650.3&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-3-116648005-blog-128055942.235%5Ev43%5Epc_blog_bottom_relevance_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7ERate-3-116648005-blog-128055942.235%5Ev43%5Epc_blog_bottom_relevance_base3&utm_relevant_index=5

0 开发环境

笔记本：ubuntu18.04.5，内核版本为5.3

开发板：imx8mp-evk

内核版本：Linux5.4.24

[交叉编译](https://so.csdn.net/so/search?q=交叉编译&spm=1001.2101.3001.7020)工具链：fsl-imx-xwayland-glibc-x86_64-imx-image-core-aarch64-imx8mpevk-toolchain-5.4-zeus.sh

注：下面的可能会遗漏部分细节，但主要的部分基本都写出来了，应该无大碍。

注：以下步骤虽然针对的imx8mp-evk，但只要是aarch64架构的芯片，大部分操作都通用的。

注：这是个人的踩坑记录，虽然可以解决问题，但估计不是最好的解决方案，欢迎评论区一起探讨。

1 iproute2是什么

iproute2是Linux上有关TCP/IP网路的一系列工具，现时由Stephen Hemminger负责维护。这一系列工具的原作是Alexey Kuznetsov，他亦是Linux内核中有关QoS实现的作者。这一系列的工具包括了以下几个元件：

```sh
ss ip lnstat nstat rdma routef routel ctstat rtstat tc bridge devlink rtacct rtmon tipc arpd genl
```

这一系列工具是为了取代一些过时不更新的命令

![07aa30238bc8b6bec9574ed654039120.png](https://i-blog.csdnimg.cn/blog_migrate/3d63806621c37a25bb365eb6cf6ea68d.png)

以上参考自https://zh.wikipedia.org/zh-hans/Iproute2

2 为什么要交叉编译iproute2

大部分嵌入式板卡应该具有这个工具，但不知是什么原因imx8mp-evk缺少我使用的tc命令，因此我需要进行交叉编译满足需求。

可以通过git下载最新的版本进行编译

```sh
git clone https://github.com/shemminger/iproute2
```

下面是git下来的最新代码

![6a1cd5686f0df69460ad8e1b8d2eaf44.png](https://i-blog.csdnimg.cn/blog_migrate/cb89752708ad59d08a60f37db5ebfcc1.png)

3 交叉编译iproute2

```sh
source /opt/fsl-imx-xwayland/5.4-zeus/[environment](https://so.csdn.net/so/search?q=environment&spm=1001.2101.3001.7020)-setup-aarch64-poky-linux

git clone https://github.com/shemminger/iproute2

cd iproute2/

./configure --host=aarch64-poky-linux --prefix=/home/sazczmh/NXP_Intern/imx8mp_tsn/iproute2/iproute2
```

![d1d25e18e17fb4c90e2b4cce9ce3ce6f.png](https://i-blog.csdnimg.cn/blog_migrate/d2c6ba786ac1e39a876f04dd7e9f7191.png)

```sh
make -j8
```

![607e581c0a17ef267087539661cceedd.png](https://i-blog.csdnimg.cn/blog_migrate/d3210e8e4e1868c62ef836d71a3d4575.png)

没有问题成功进行了编译，然后make install为如下结果

![fb34a9c08cadbf43fa270f2af5e30836.png](https://i-blog.csdnimg.cn/blog_migrate/af258af5b21a64ff4b158b73c6659283.png)

![9a0b72cb77d5bf7e04997a768fc4532b.png](https://i-blog.csdnimg.cn/blog_migrate/950e1a9ed19377505df7051bd90dfffd.png)

这一切看似正常，没有什么问题，可是接下来导入开发板发现了缺少一个动态链接库。

![cb9dbff6902e2ad86197fe489197c855.png](https://i-blog.csdnimg.cn/blog_migrate/f0ab42fc91aec6fe7f9ee4248592b0ef.png)

然而交叉编译也没有生成相应的文件。

![8885c7739deb07a25655d78e50d1afbe.png](https://i-blog.csdnimg.cn/blog_migrate/c0783f7972f58247cd67e0ba69b8dfab.png)

通过万能的谷歌，发现了只要再交叉编译iptables就可以得到想要的动态链接库了。

4 iptables

从官网下载源码并解压

https://www.netfilter.org/projects/iptables/downloads.html

注：个人愚钝，不建议从git下载(这个iptables从git下载的源码好像有点不会搞)

```sh
source /opt/fsl-imx-xwayland/5.4-zeus/environment-setup-aarch64-poky-linux

cd iptables-1.8.5

mkdir bin

./configure --host=aarch64-poky-linux --prefix=/home/sazczmh/NXP_Intern/imx8mp_tsn/iptables-1.8.5/bin/ --disable-nftables

make -j 8

make install
```

![a5d34938cc8b7ef899dc56b305af81e4.png](https://i-blog.csdnimg.cn/blog_migrate/5c78bb86fa6d1209d2aed813bffa8fe7.png)

然后将生成的这三个文件拷贝到开发板对应的/usr/lib文件夹下即可

![ab0f0d549e44acb84de1dd4cc80bc82d.png](https://i-blog.csdnimg.cn/blog_migrate/7a395b6b724ca3f971b60f90ae013278.png)

![936ad876a30625a012f7128c2c2d40a5.png](https://i-blog.csdnimg.cn/blog_migrate/844bd79d34dd118654236b164768a97e.png)

注：我复制的比较随意，软链接？没了，不过可以用我就没管。

注：个人能力有限，欢迎批评指正。

原创不易，严禁剽窃！

欢迎大家关注我创建的微信公众号——小白仓库

原创经验资料分享：包含但不仅限于FPGA、ARM、RISC-V、Linux、LabVIEW等软硬件开发，另外分享生活中的趣事以及感悟。目的是建立一个平台记录学习过的知识，并分享出来自认为有用的与感兴趣的道友相互交流进步。