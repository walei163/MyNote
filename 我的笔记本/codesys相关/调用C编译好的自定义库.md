# 调用外部C编译好的自定义库

1. 按照[《Extension SDK》](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/Extension SDK.md)所描述的那样，实现外部C语言库之后，打包为CODESYS格式的lib库。

2. 建议新的工程项目，然后将之前编译好的库加进到工程中：

   打开“库管理器”，点击“库存储库”：

   ![](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/屏幕截图 2025-07-30 105923.png)

如上图，点击“安装”，在打开的文件夹中选择之前编译好的库文件路径，将其加入到“System“系统库中。

这样就可以调用使用了。