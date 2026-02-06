# Typora 免费激活方法

1. 搜索[Typora中文官网](https://typoraio.cn/)自行下载自己想要的版本，安装完毕

2. 找到自己Typora的安装目录

![img](https://img-blog.csdnimg.cn/direct/ce291a4074eb434e9cb816f9136bccd7.png)

3. 打开 `resources` 文件夹，找到`D:\Typora\resources\page-dist\static\js\LicenseIndex.180dd4c7.c77b6991.chunk.js`文件，文件名称会有出入，认准`LicenseIndex`就行了

![img](https://img-blog.csdnimg.cn/direct/224c4e3877f944ee9a80efcd4146b64c.png)

4. 打开js文件，[Vscode](https://so.csdn.net/so/search?q=Vscode&spm=1001.2101.3001.7020),文本编辑器都可以，我这里有Vscode;查找 **e.hasActivated 将 == 后面的值都换成true**

![img](https://img-blog.csdnimg.cn/direct/858077783b1e4fa0959ea4c045438c43.png)

5. 最后效果

![img](https://img-blog.csdnimg.cn/direct/1c43e05e1eb542268262ba1a383c388b.png)