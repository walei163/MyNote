# rk3568-快速部署codesys runtime

## 前期准备

```sh
PC-win10系统
RK3568-debian系统，内核已打入实时补丁，开启ssh服务。
```

## PC下载安装Codesys Development System V3.15.17.0

```sh
https://store.codesys.com/en/codesys.html#product.attributes.wrapper
```

## PC下载安装Codesys Control for linux ARM64 SL 4.1.0.0 package

```sh
https://store.codesys.com/en/codesys-control-for-linux-arm-sl-1.html
```

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/8143b81b33ba48229e73b668be2080bc.png)

```sh
CODESYS Development System -> 工具 -> 包管理器进行安装
注意：codesys runtime如果没有license，软件只能运行2个小时。
```

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/23a052c1d4024366b9f8faa6facf1122.png)

```sh
安装过程中如果报错：
```

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/7e86f299468d4c5eb244957066e9b6e6.png)

```sh
在搜索栏搜索相关package安装即可。
```

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/8d21598602ce45c4a22d0da2afc8aa51.png)

安装完CODESYS Control for Linux ARM64 SL 4.1.0.0.package之后

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/8143b81b33ba48229e73b668be2080bc.png)

CODESYS Development System -> 工具 -> Update Linux ARM64进行RK3568配置
网线连接PC和RK3568，保证两者在同一网段，能ping通。
填写ssh账号和密码，ip地址，点击Install

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/0dd5704e2c4d4f20af42b92012538db1.png)

安装完成，查看RK3568是否运行codesys

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/95f1c25d85104caea51386cf49056627.png)

## PC建立工程，进行验证

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/ddec0eb3acde433e9a60e08c3ecbfd75.png)

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/d0a8ece05aff4dd992b5d4fbc7c69ece.png)

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/2df79b997f074c5f83ae45e6d818aaca.png)

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/9c0cc8e8e7be424aba837f05dea6365d.png)

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/1e80f35e957c4596ad55564a8d1eb3a7.png)

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/c9fed8ae6f5e46faaced0b163edbaa85.png)

点击在线->登录，配置用户名和密码，点击确定

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/72651e51f1a34c48acc64ff7df0f440d.png)

点击调试，启动和停止

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/2e195d89a4a9481c8e487e5080575e66.png)

点击在线->退出，完成调试

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/c9fed8ae6f5e46faaced0b163edbaa85.png)