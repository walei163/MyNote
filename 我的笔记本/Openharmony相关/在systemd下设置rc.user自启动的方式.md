# 在systemd下设置rc.user自启动的方式

FUXI的文件系统采用`systemd`的管理方式，因此需要重新设置`rc.user`启动方式：

我们可以通过打开`rc.local`作为启动服务，来启动`rc.user`脚本：

## 一、设置`rc.local`服务

1、建立链接：

```sh
ln -sf /lib/systemd/system/rc-local.service /etc/systemd/rc-local.service
```

2、修改rc-local.service内容如下：

```sh
#  SPDX-License-Identifier: LGPL-2.1+
#
#  This file is part of systemd.
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.

# This unit gets pulled automatically into multi-user.target by
# systemd-rc-local-generator if /etc/rc.local is executable.
[Unit]
Description=/etc/rc.local Compatibility
Documentation=man:systemd-rc-local-generator(8)
ConditionFileIsExecutable=/etc/rc.local
After=network.target

[Service]
Type=forking
ExecStart=/etc/rc.local start
StandardOutput=tty
StandardError=tty
TimeoutSec=0
RemainAfterExit=yes
GuessMainPID=no

[Install]
WantedBy=multi-user.target
```

3、启动服务：

```sh
systemctl enable rc-local.service
```

## 二、设置rc.user自启动：

1、在etc目录下建立`rc.local`文件，内容如下：

```sh
#!/bin/sh
DIR_SANWAY=/usr/local/sanway
FILE_RC_USER=rc.user

if [ -x ${DIR_SANWAY}/${FILE_RC_USER} ]
then
	${DIR_SANWAY}/${FILE_RC_USER} start
	exit 0
else
	echo ${DIR_SANWAY}/${FILE_RC_USER} can not execute.
	exit 1
fi
```

2、然后就可以和以前一样，将rc.user、ini等文件放置到/usr/local/sanway下即可。