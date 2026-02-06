# ubuntu下如何安装firefox-esr

## 在 Ubuntu 中安装 Firefox ESR

在进入安装之前，让我来分享一下普通 Firefox 和 Firefox-ESR 之间的版本差异是什么。在写这篇文章的时候：

- Firefox 的版本是 **107.0-2**。
- Firefox-ESR 目前的版本是 **102.5.0esr**。

所以，如果这对你来说没问题，让我们看看第一个方法。

**方法 1：使用 PPA 安装 Firefox-ESR**

Firefox-ESR 在 Ubuntu 的默认仓库中是不可用的，所以你可以使用 PPA。

PPA 只不过是一个由个别技术人员或开发者维护的仓库，拥有默认仓库所没有的东西。

如果你想了解更多关于 PPA 的信息，我建议你查看我们的其他指南，其中解释了 [如何在 Linux 上使用 PPA](https://link.zhihu.com/?target=https%3A//itsfoss.com/ppa-guide/)。

打开你的终端，使用给定的命令来添加 Firefox-ESR 的 PPA：

```sh
sudo add-apt-repository ppa:mozillateam/ppa
```

然后按回车键确认你要添加 PPA：

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/汪磊个人相关/images/v2-dc9328a4055b89c14ae4351b30af59cf_1440w.jpg)

完成后，你需要更新 Ubuntu 中的仓库索引，以便从这些变化中生效：

```sh
sudo apt update
```

现在，你可以通过使用给定的命令来安装 Firefox-ESR：

```text
sudo apt install firefox-esr
```

接下来，你可以使用给定的命令来检查你系统中 Firefox-ESR 的安装版本：

```sh
firefox-esr -v
```

![img](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/汪磊个人相关/images/v2-854dc526de0b78e57744e7b246b84f70_1440w.jpg)

check installed version of firefox esr in ubuntu



如何从 Ubuntu 卸载 Firefox-ESR？

如果 ESR 对你的工作来说感觉太过时了，或者由于其他原因你想从你的系统中删除它，你可以按照以下步骤删除 Firefox-ESR 包和仓库。

首先，让我们用下面的方法删除 Firefox-ESR 包：

```text
sudo apt remove firefox-esr
```

现在，你可以使用给定的命令来 [从 Ubuntu 删除 PPA](https://link.zhihu.com/?target=https%3A//itsfoss.com/how-to-remove-or-delete-ppas-quick-tip/)：

```text
sudo add-apt-repository --remove ppa:mozillateam/ppa
```

这就完成了！