# debian13相关

## 2026-01-16：

### 1、今天发现了一个新的现象，在`debian13`下，`ping`命令直接运行时会出现错误提示：

```sh
$ ping 192.168.1.5
ping: socktype: SOCK_RAW
ping: socket: 不允许的操作
ping: => missing cap_net_raw+p capability or setuid?
```

百度一下，原来是默认将`ping`命令的`cap_net_raw`权限去掉了。可以手动将其恢复回来：

```sh
sudo setcap cap_net_raw+p /usr/bin/ping
```

这样之后，就可以直接ping了：

```sh
$ ping 192.168.1.5
PING 192.168.1.5 (192.168.1.5) 56(84) bytes of data.
64 bytes from 192.168.1.5: icmp_seq=1 ttl=64 time=1.97 ms
64 bytes from 192.168.1.5: icmp_seq=2 ttl=64 time=2.61 ms
```

---

## 2025-12-16：

### 关于在debian13以及相关衍生发行版下，编译Linux内核源码时提示找不到`ncurses`库的问题：

#### 1、经过copilot的指导，修改了检查脚本：`scripts/kconfig/lxdialog/check-lxdialog.sh`，编译通过了。

主要是要修正一个`check`的函数。新的`check`函数如下：

```sh
check() {
	# Split the compiler+flags string into positional parameters so we
	# can exec the compiler directly without re-parsing via sh -c.
	oldifs="$IFS"
	IFS=' '
	set -- $cc
	IFS="$oldifs"

	# Try a few likely header names by creating a temp C file that
	# directly includes the header. This avoids passing '<...>' to
	# the shell in compiler flags which can be misinterpreted as
	# redirection on some /bin/sh implementations.
	for hdr in "<ncursesw/curses.h>" "<ncurses.h>" "<curses.h>"; do
		cat > ${tmp}.c <<EOF
#include $hdr
int main(void) { return 0; }
EOF
		"$@" -x c ${tmp}.c -o $tmp 2>/dev/null
		ret=$?
		rm -f ${tmp}.c
		if [ $ret -eq 0 ]; then
			break
		fi
	done
	if [ $ret != 0 ]; then
		echo " *** Unable to find the ncurses libraries or the"       1>&2
		echo " *** required header files."                            1>&2
		echo " *** 'make menuconfig' requires the ncurses libraries." 1>&2
		echo " *** "                                                  1>&2
		echo " *** Install ncurses (ncurses-devel) and try again."    1>&2
		echo " *** "                                                  1>&2
		exit 1
	fi
}
```



最后形成了一个patch文件。内容如下：

```sh
*** Begin Patch
*** Update File: scripts/kconfig/lxdialog/check-lxdialog.sh
@@
 check() {
-        $cc -x c - -o $tmp 2>/dev/null <<'EOF'
-#include CURSES_LOC
-main() {}
-EOF
-    if [ $? != 0 ]; then
-        echo " *** Unable to find the ncurses libraries or the"       1>&2
-        echo " *** required header files."                            1>&2
-        echo " *** 'make menuconfig' requires the ncurses libraries." 1>&2
-        echo " *** "                                                  1>&2
-        echo " *** Install ncurses (ncurses-devel) and try again."    1>&2
-        echo " *** "                                                  1>&2
-        exit 1
-    fi
+    # Split the compiler+flags string into positional parameters so we
+    # can exec the compiler directly without re-parsing via sh -c.
+    oldifs="$IFS"
+    IFS=' '
+    set -- $cc
+    IFS="$oldifs"
+
+    # Try a few likely header names by creating a temp C file that
+    # directly includes the header. This avoids passing '<...>' to
+    # the shell in compiler flags which can be misinterpreted as
+    # redirection on some /bin/sh implementations.
+    for hdr in "<ncursesw/curses.h>" "<ncurses.h>" "<curses.h>"; do
+        cat > ${tmp}.c <<EOF
+#include $hdr
+int main(void) { return 0; }
+EOF
+        "$@" -x c ${tmp}.c -o $tmp 2>/dev/null
+        ret=$?
+        rm -f ${tmp}.c
+        if [ $ret -eq 0 ]; then
+            break
+        fi
+    done
+    if [ $ret != 0 ]; then
+        echo " *** Unable to find the ncurses libraries or the"       1>&2
+        echo " *** required header files."                            1>&2
+        echo " *** 'make menuconfig' requires the ncurses libraries." 1>&2
+        echo " *** "                                                  1>&2
+        echo " *** Install ncurses (ncurses-devel) and try again."    1>&2
+        echo " *** "                                                  1>&2
+        exit 1
+    fi
 }
*** End Patch
```

#### 2、patch文件下载链接：

- patch补丁文件：

[/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/fix-check-lxdialog.patch](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/fix-check-lxdialog.patch)

- 以及修改后的`check-lxdialog.sh`脚本文件：

[/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/check-lxdialog.sh](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/debian12相关/check-lxdialog.sh)

#### 3、补丁的应用方法：

应用补丁（选一）：

- 使用 git:


```sh
git apply fix-check-lxdialog.patch
```

- 或使用 patch:


```sh
patch -p0 < fix-check-lxdialog.patch
```

应用后可验证：

```sh
make HOSTCC=gcc ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```