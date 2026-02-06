# 设置虚拟python环境

1、新建一个项目目录，如`proj_pyecharts`:

```sh
mkdir proj_pyecharts
cd proj_pyecharts
```

2、设置虚拟环境：

```sh
python3 -m venv .venv
```

3、激活虚拟环境：

```sh
source .venv/bin/activate
```

如果激活成功，shell终端提示符将变为：

```sh
(.venv) jason@sparky7:/media/jason/btrfs_disk_M/home/python/proj_pyecharts
```

4、在该环境下，可以用pip来安装相应的包：

```sh
pip install pyecharts
```

同时，还可以用vscode来打开源代码文件：

```sh
code .
```

