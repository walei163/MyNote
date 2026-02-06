# Linux桌面特效（Xfce4）使用compiz

1、需要vmware打开：

![image-20251029103713124](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/image-20251029103713124.png)

---

![image-20251029103911486](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/image-20251029103911486.png)

---

2、在虚拟机里的Linux系统下安装compiz：

```sh
$ sudo apt install compiz
compiz                         compiz-mate
compiz-bcop                    compiz-plugins
compiz-boxmenu                 compiz-plugins-default
compizconfig-settings-manager  compiz-plugins-experimental
compiz-core                    compiz-plugins-extra
compiz-dev                     compiz-plugins-main
compiz-gnome                   
```

---

3、修改xfce4的配置文件：

```sh
kate /etc/xdg/xfce4/xfconf/xfce-perchannel-xml/xfce4-session.xml
```

```sh
<property name="Client0_Command" type="array">
        <value type="string" value="compiz"/>
</property>
```

其中value="xfwm4"改为：value="compiz"。然后保存。

---

4、清除原来的会话：

Xfce通常会在logout时保存当前会话，以便下次登录时可以恢复之前打开的应用程序，当我们调整窗口管理器为compiz后，需要清除之前已经保存的会话，并在第一次注销时不保存当前会话。

```sh
rm -rf ~/.cache/sessions
```

其它的一些配置可以参照：https://blog.csdn.net/kewen_123/article/details/115871744