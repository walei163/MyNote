# Markdown进阶（更改字体、颜色、大小，设置文字背景色，调整图片大小设置居中）

原创 置顶 heimu24 最后发布于2018-07-24 18:08:20 阅读数 61824 收藏

展开

    基础知识：
    Markdown 通过简单标记语法，使普通文本内容具有一定格式。但它本身不支持修改字体、字号与颜色等功能的。
    CSDN-markdown 编辑器是其衍生版本，支持基于 PageDown ( Stack Overflow）所使用的编辑器的扩展功能（如表格、脚注、内嵌HTML、内嵌 LaTeX 等等）。
    Size：规定文本的尺寸大小，取值从 1 到 7 ，浏览器默认值是 3.
    
    一、更改字体、大小、颜色
    二、为文字添加背景色
    三、设置图片大小
        1、设置设置图片百分比
        2、设置图片大小
        3、设置图片居中
    参考链接

一、更改字体、大小、颜色

Markdown语法

    <font face="黑体">我是黑体字</font>
    <font face="微软雅黑">我是微软雅黑</font>
    <font face="STCAIYUN">我是华文彩云</font>
    <font color=red>我是红色</font>
    <font color=#008000>我是绿色</font>
    <font color=Blue>我是蓝色</font>
    <font size=5>我是尺寸</font>
    <font face="黑体" color=green size=5>我是黑体，绿色，尺寸为5</font>

效果如下：

<font face="黑体">我是黑体字</font>

<font face="微软雅黑">我是微软雅黑</font>

<font face="STCAIYUN">我是华文彩云</font>

<font color=red>我是红色</font>

<font color=#008000>我是绿色</font>

<font size=5>我是尺寸</font>

 <font face="黑体" color=green size=5>我是黑体，绿色，尺寸为5</font>

二、为文字添加背景色

由于 style 标签和标签的 style 属性不被支持，所以这里只能是借助 table, tr, td 等表格标签的 bgcolor 属性来实现背景色。故这里对于文字背景色的设置，只是将那一整行看作一个表格，更改了那个格子的背景色（bgcolor）

Markdown语法

    <table><tr><td bgcolor=yellow>背景色yellow</td></tr></table>

效果如下
<table><tr><td bgcolor=yellow>背景色yellow</td></tr></table>

三、设置图片大小

1. 设置设置图片百分比

Markdown语法

    <img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="50%" height="50%">

效果如下

<img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="50%" height="50%">

2. 设置图片大小

Markdown语法

    <img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="251" height="350" >
效果如下

<img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="251" height="350" >

3. 设置图片居中

Markdown语法

    <div align=middle><img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="50%" height="50%"></div>

效果如下

<div align=middle><img src="http://pic11.photophoto.cn/20090626/0036036341009653_b.jpg" width="50%" height="50%"></div>

ps：center,left,left
参考链接

RGB颜色对照表

Markdown 语法大全 包括设置字体 颜色

Markdown: Basics （快速入门）

CSDN-markdown 文字样式设置（字体, 大小, 颜色, 高亮底色）
————————————————
版权声明：本文为CSDN博主「heimu24」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/heimu24/java/article/details/81189700