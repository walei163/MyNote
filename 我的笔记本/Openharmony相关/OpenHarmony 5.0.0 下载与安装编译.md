# OpenHarmony 5.0.0 下载与安装编译

## 一、写在前面

遥想当年，Android 5.0可以说是Android的一个划时代版本。Android 5.0 全面使用ART替代Dalvik（4.4版本引入），原先的Dalvik可以看做类似JVM的字节码解释器，最直观的区别是——JVM的字节码是.class文件，而Dalvik的字节码文件是.dex文件（当然具体实现细节上有非常多差异，这你不在展开）。应用程序由原来解释执行dex字节码变成安装时预编译为本地代码，运行时只需要执行本地代码就可以了。ART特性极大的提升了应用程序的执行效率，让系统和应用的流畅度有了非常大的的提升，很大程度上提升了用户体验。

而现在，OpenHarmony也迎来了5.0版本，希望也能和Android 5.0版本一样在整个版本历史上成为一个令人难忘的里程碑。Android ART类似的，OpenHarmony的应用程序最早使用JS和C++开发，早期标准版和小型版的JS执行环境都是QuickJS，后来标准版换为了Ark Compiler和Ark Runtime（方舟编译器和方舟运行时）、小型版也逐步从QuickJS换为了Ark。

OpenHarmony的Ark和Android的ART类似的是，最终运行时执行的都是native代码，都解决了解释执行效率低的问题。

## 二、准备命令工具

本节介绍如何准备命令行工具，具体包括`git/repo/python`。

OpenHarmony代码托管在码云服务器上，下载单个代码仓通常使用git命令行工具，下载所有代码仓需要使用repo命令行工具。repo命令行工具是由Python开发的，因此开始之前，需要线安装git/git-lfs/python等命令行工具。

安装git/git-lfs/python/curl，执行如下命令：

```sh
sudo apt install git git-lfs python3-pip curl
```

安装repo命令，执行如下命令：

```sh
# 创建目录
mkdir ~/bin

# 下载repo脚本
curl https://gitee.com/oschina/repo/raw/fork_flow/repo-py3 > ~/bin/repo

# 下载repo依赖的软件包
pip3 install -i https://repo.huaweicloud.com/repository/pypi/simple requests
```

### 三、配置用户信息

本节介绍如何配置本地git用户名和邮箱，以及如何配置码云SSH公钥（使用ssh协议下载必须配置）：

1. 注册码云gitee帐号。

2. 注册码云SSH公钥，请参考[码云帮助中心](https://gitee.com/help/articles/4191)。

3. 配置git用户信息，执行如下命令：

```sh
git config --global user.name "yourname"
git config --global user.email "your-email-address"
git config --global credential.helper store
```

4. 创建目录，用于存放OpenHarmony源码，执行如下命令：

```sh
mkdir -p ~/ohos/openharmony
cd ~/ohos/openharmony
```

### 四、下载OpenHarmony源码

#### 4.1 使用ssh协议下载（推荐）

通过repo + ssh 下载（需注册公钥，请参考[码云帮助中心](https://gitee.com/help/articles/4191)）。

- 从版本分支获取源码。可获取该版本分支的最新源码，包括版本发布后在该分支的合入。

```sh
repo init -u git@gitee.com:openharmony/manifest.git -b OpenHarmony-5.0.0-Release --no-repo-verify
repo sync -c
repo forall -c 'git lfs pull'
```

- 从版本发布Tag节点获取源码。可获取与版本发布时完全一致的源码。

```sh
repo init -u git@gitee.com:openharmony/manifest.git -b refs/tags/OpenHarmony-v5.0.0-Release --no-repo-verify
repo sync -c
repo forall -c 'git lfs pull'
```

#### 4.2 使用https协议下载

通过repo + https 下载。

- 从版本分支获取源码。可获取该版本分支的最新源码，包括版本发布后在该分支的合入。

```sh
repo init -u https://gitee.com/openharmony/manifest -b OpenHarmony-5.0.0-Release --no-repo-verify
repo sync -c
repo forall -c 'git lfs pull'
```

- 从版本发布Tag节点获取源码。可获取与版本发布时完全一致的源码。

```sh
repo init -u https://gitee.com/openharmony/manifest -b refs/tags/OpenHarmony-v5.0.0-Release --no-repo-verify
repo sync -c
repo forall -c 'git lfs pull'
```

代码下载完成，占用的磁盘空间为51G：

![image-20241001214001202](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/Openharmony相关/d12df805abed6ed54bbb921d15b0801b.png)

### 五、下载编译工具链

代码下载完成后，还需要执行prebuilts_download.sh脚本，下载编译工具链，执行如下命令：

```sh
bash build/prebuilts_download.sh
```

这个脚本会创建prebuilts目录，并将边回忆工具链下载、解压到这个目录下。

以上全部完成后，磁盘空间占用67G：

![image-20241001214628323](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/Openharmony相关/12423063847971248ad71f42a1744c0d.png)

### 六、参考链接

本文主要参考了OpenHarmony文档仓，里面由其他版本的发布说明（Release Note），根据其他版本的发布说明，也可以下载其他版本的源代码。

https://gitee.com/openharmony/docs/tree/master/zh-cn/release-notes
https://gitee.com/openharmony/docs/blob/master/zh-cn/release-notes/OpenHarmony-v5.0.0-release.md

---

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/xusiwei1236/article/details/142675221