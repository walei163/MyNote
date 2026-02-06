# 在 Linux 下使用 Notion

方法引自：[Sae10138：在Ubuntu中使用Notion的方法](https://zhuanlan.zhihu.com/p/267455189)，个人使用 [openSUSE](https://zhida.zhihu.com/search?content_id=197331303&content_type=Article&match_order=1&q=openSUSE&zhida_source=entity)，其实是一样的，记录一下。

1. 新建 Notion 运行脚本

- `vim notion.sh`创建脚本文件
- 编辑完后，用`:wq` 退出
- 脚本内容

```bash
#!/usr/bin/sh
# 根据实际使用的 Chromium 系浏览器修改就行了，我这里用的是 Edge 浏览器
# 本质是把网页版作为 Chromium 应用运行（隐藏浏览器其他元素），所以Firefox 不支持
# 
# google-chrome --app=https://www.notion.so
#microsoft-edge --app=https://www.notion.so
microsoft-edge --new-window notion.so
```

3. 执行权限

- `chmod u+x notion.sh`，给文件加执行权限
- 再加个桌面图标

- 自己画个简单的 Logo，做成 png 文件（有需要的就直接用）

![img](https://pic1.zhimg.com/v2-d4447c9a30ab1ca2a2c48dc4e59057de_1440w.jpg)

自己简单画了个图标，网页版就不那么讲究了

![img](https://pic2.zhimg.com/v2-7c1ef605f2bf3ca759e536c1a122a967_1440w.jpg)

为了适配主题图标，又改了个方的

- `vim notion.desktop`，创建桌面图标

```bash
#!/usr/bin/env xdg-open
[Desktop Entry]
Name=Notion
Exec=/home/uname/bin/bin/Notion-linux/notion.sh
Icon=/home/uname/bin/bin/Notion-linux/notion.png
Type=Application
Terminal=false
Categories=Office;
MimeType=text/plain
```

- 写完图标的内容后，把文件放到 `/home/uname/.local/share/applications/` 里面
- 使用方法

- 现在应该在程序列表的 Office 分类中看到 Notion 了，点开就会调用上面指定的浏览器访问网页版（或者叫浏览器应用）的 Notion
- 登录个人账号就可以了
- 使用时会感觉比 Windows 下的客户端略慢一些

------

2022年，发现三方的客户端（**notion**-**enhancer**），是 Windows，Mac，Linux 都支持，而且和官方客户端没什么区别。

2024年，notion-enhancer无法使用，清理其配置文件也没有作用。还是用网页创建快捷应用了。