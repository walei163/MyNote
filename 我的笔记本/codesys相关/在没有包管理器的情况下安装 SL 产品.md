# 在没有包管理器的情况下安装 SL 产品

SL 产品的更新工具在后台将安装包（.deb 或 .ipk）复制到目标设备，然后进行安装。这需要在目标设备上安装软件包管理器（dpkg 或 ipkg）。如果未安装包管理器，则更新工具无法安装运行时系统。

但是，如果需要，仍然可以手动安装软件包。对于通用产品，这在下面使用 CODESYS Control for Linux 以 V4.7.0.0 为例。

### 重要

CODESYS Control 不支持具有（部分）写保护文件系统的设备。

如果您的设备具有写保护文件系统，并且您在安装或启动产品时遇到问题，请检查禁用写保护后问题是否仍然出现。

## 在目标设备上手动安装运行时系统

1. 安装后 CODESYS Control for Linux 软件包，所需的文件位于 Windows 电脑上：

   - `<user>/CODESYS Control for Linux SL/Delivery/codesyscontrol_linux_4.7.0.0-b.trunk.39_amd64.deb` 或者

     `<user>/CODESYS Control for Linux SL/Delivery/codesyscontrol_linux_4.7.0.0-b.trunk.39_amd64.ipk`

   - `<user>/CODESYS Control for Linux SL/Dependency/codemeter-lite_<Version>.deb` （用于许可）

   将这些文件复制到目标设备。

2. 打开包装：

   ```sh
   $ ar -x codesyscontrol_linux_<Version>
   ```

创建了以下文件：

- `control.tar.gz`：包含软件包管理器在安装或卸载前后运行的脚本。这与手动安装无关。
- `data.tar.gz`：包含运行时系统二进制文件、库、启动脚本和配置
- `debian-binary`: 指定 .deb 包的版本。这与手动安装无关。

打开包装 `data.tar.gz` 文件。事先为此创建一个新文件夹：

```sh
$ mkdir data
$ tar -xf data.tar.gz -C data
```

在 `data/`，您现在将找到对运行时系统的操作很重要的文件：

```sh
data
├── etc
│   ├── default
│   │   └── codesyscontrol
│   ├── init.d
│   │   └── codesyscontrol
│   └── codesyscontrol
│       ├── 3S.dat
│       ├── CODESYSControl.cfg
│       └── CODESYSControl_User.cfg
├── opt
│   └── codesys
│       ├── bin
│       │   └── codesyscontrol.bin
│       ├── lib
│       │   ├── libCmpHilscherCIFX.so
│       │   └── libSysPci.so
│       └── scripts
│           ├── init-functions
│           ├── init-vars
│           ├── PlcWink.sh
│           └── rts_set_baud.sh
├── usr
│   └── share
│       └── doc
│           └── codesyscontrol
│               └── copyright
└── var
    └── opt
        └── codesys
            ├── bacstac.ini
            ├── cmact_licenses
            └── .SoftContainer_CmRuntime.wbb 
```

现在包管理器（dpkg 或 ipkg）通常会将此目录结构复制到系统的根目录。这必须在没有包管理器的情况下手动完成：

更改为 `data/` 目录所在的 `data.tar.gz` 文件被解压：

```sh
$ cd data/
```

将各个目录复制到目标的相应根目录：

### 重要

如果您犯了错误，那么此步骤可能会使您的目标设备无法使用，您将不得不重置它。

```sh
$ sudo cp -r etc/* /etc
$ sudo cp -r opt/* /opt
$ sudo cp -r usr/* /usr
$ sudo cp -r var/* /var
```

现在包管理器将执行一些安装后步骤来完成安装。手动执行这些步骤：

```sh
$ sudo chmod a+rw /etc/codesyscontrol/CODESYSControl.cfg
$ sudo chmod a+rw /etc/codesyscontrol/CODESYSControl_User.cfg
```

如果 `codesysuser` 目标设备上尚不存在群组，然后将其添加。

```sh
$ groupadd codesysuser
```

1. 该组用于扩展 API。有关详细信息，请参阅： [安全机制](https://content.helpme-codesys.com/zh-CHS/CODESYS Control/_rtsl_extension_security.html)

运行时系统现已安装。但是，它仍然需要 Codemeter 运行时环境。

## 在目标设备上手动安装 Codemeter 运行时环境

1. 安装 CODESYS Control SL 软件包后，.deb 软件包会保存在 `<User>/CODESYS Control für Linux SL/Dependency/codemeter-lite_<Version>.deb` 中。

   还将此 .deb 包复制到目标系统上新创建的空文件夹中。

2. 解压 Debian 软件包。

   ```sh
   $ ar -x codemeter-lite_<Version>.deb
   ```

这 `control.tar.gz`, `data.tar.gz`, `debian-binary`，并且 `_gpgorigin` 文件被提取。

打开包装 `data.tar.gz` 文件到新创建的文件夹中。

```sh
$ mkdir data
$ tar -xf data.tar.gz -C data
```

之后，二进制文件和其他文件（例如库和脚本）可在 `data/`:

```sh
data
├── etc
│   ├── init.d
│   │   ├── codemeter
│   │   └── codemeter-webadmin
│   └── wibu
│       └── CodeMeter
│           └── Server.ini
├── lib
│   ├── systemd
│   │   └── system
│   │       ├── codemeter.service
│   │       └── codemeter-webadmin.service
│   └── udev
│       └── rules.d
│           └── 60-codemeter-lite.rules
├── usr
│   ├── bin
│   │   ├── cmu
│   │   └── codemeter-info
│   ├── lib
│   │   └── x86_64-linux-gnu
│   │       ├── jni
│   │       │   ├── libwibucmJNI64.so -> libwibucmJNI.so
│   │       │   └── libwibucmJNI.so
│   │       ├── libwibucmlin64-4.so -> libwibucm.so
│   │       ├── libwibucmlin64.so -> libwibucm.so
│   │       ├── libwibucmlin.so -> libwibucm.so
│   │       └── libwibucm.so
│   ├── sbin
│   │   ├── CmWebAdmin
│   │   └── CodeMeterLin
│   └── share
│       ├── bash-completion
│       │   └── completions
│       │       └── cmu
│       ├── doc
│       │   ├── CodeMeter
│       │   │   ├── OpenSource_en.pdf
│       │   │   └── README
│       │   └── codemeter-lite
│       │       ├── changelog.gz
│       │       └── copyright
│       └── man
│           └── man1
│               └── codemeter-info.1.gz
└── var
    ├── lib
    │   └── CodeMeter
    │       ├── Backup
    │       ├── CmAct
    │       ├── CmCloud
    │       ├── NamedUser
    │       └── WebAdmin
    └── log
        └── CodeMeter
```

更改为 `data/` 目录所在的 `data.tar.gz` 文件被解压：

```sh
$ cd data/
```

现在将整个目录复制到目标设备的根目录：

### 重要

如果您犯了错误，那么此步骤可能会使您的目标设备无法使用，您将不得不重置它。

```sh
$ sudo cp -r etc/* /etc
$ sudo cp -r lib/* /lib
$ sudo cp -r usr/* /usr
$ sudo cp -r var/* /var
```

现在，Codemeter 包将执行一些安装后步骤来完成安装。手动执行这些步骤：

```sh
$ sudo udevadm trigger -vn --subsystem-match=usb --attr-match=idVendor=064f | xargs -rn1 -d\\n udevadm trigger -b
$ sudo mkdir -p "/etc/systemd/system/multi-user.target.wants/"
$ sudo ln -sT /lib/systemd/system/codemeter.service /etc/systemd/system/multi-user.target.wants/codemeter.service
```

现在你需要使用以下命令设置 Codemeter 服务（启动为 `root`)：

```sh
CodemeterLin -x
```

Codemeter Lite 和运行时系统的安装现已完成。

## 手动安装边缘网关：

您可以按照与运行时系统和 Codemeter 相同的方式安装边缘网关。

1. 在 Windows PC 上，包存储在 `<user>/CODESYS Edge Gateway for Linux/Delivery/codesysedge_edgeamd64_<Version>.deb`.

   将 Debian 软件包复制到目标设备上的一个新的空文件夹中。

2. 解压 Debian 软件包：

   ```sh
   $ ar -x codesysedge_edgeamd64_<Version>.deb
   ```

文件 `control.tar.gz`, `data.tar.gz` 和 `debian-binary` 被提取。

打开包装 `data.tar.gz` 将文件放入新创建的文件夹中：

```sh
$ mkdir data
$ tar -xf data.tar.gz -C data
```

之后，二进制文件和其他文件（例如库和脚本）可在 `data/`:

```sh
data
├── etc
│   ├── default
│   │   └── codesysedge
│   ├── init.d
│   │   └── codesysedge
│   └── codesysedge
│       ├── Gateway.cfg
│       ├── GatewayvControl.cfg
│       └── Gateway_User.cfg
├── opt
│   └── codesysedge
│       ├── bin
│       │   └── codesysedge.bin
│       ├── lib
│       └── scripts
│           ├── rts_set_baud.sh
│           └── startup.sh
├── usr
│   └── share

│       └── doc
│           └── codesysedge
│               └── copyright
└── var
    └── opt
        └── codesysedge
```

更改为 `data/` 目录所在的 `data.tar.gz` 文件被解压：

```sh
$ cd data/
```

现在将整个目录复制到目标设备的根目录：

### 重要

如果您犯了错误，那么此步骤可能会使您的目标设备无法使用，您将不得不重置它。

```sh
$ sudo cp -r etc/* /etc
$ sudo cp -r opt/* /opt
$ sudo cp -r usr/* /usr
$ sudo cp -r var/* /var
```

再次执行包管理器的后处理步骤：

```sh
$ chmod a+rw /etc/codesysedge/Gateway.cfg
$ chmod a+rw /etc/codesysedge/Gateway_User.cfg
```

边缘网关现已安装。

## 启动 Codemeter、运行时系统和边缘网关

现在可以启动已安装的包：

- **Codemeter**

  ```sh
  $ sudo /usr/sbin/CodeMeterLin
  ```

您可以使用以下命令启动 Codemeter `-v` 标志以获得扩展输出。

随着输入 `-Help`，您将获得有关更多功能的帮助。

**运行时系统和边缘网关**

```sh
$ sudo /etc/init.d/codesyscontrol start
$ sudo /etc/init.d/codesysedge start
```