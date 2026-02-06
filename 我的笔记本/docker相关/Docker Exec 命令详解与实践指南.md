![[Pasted image 20240501181219.png]]
# 简介

[Docker](https://so.csdn.net/so/search?q=Docker&spm=1001.2101.3001.7020) Exec 是 Docker 中一个非常有用的命令，它允许您在正在运行的容器内部执行命令。这对于调试、管理和与容器进行交互非常有帮助。在本篇文章中，我们将深入探讨 Docker Exec 命令的使用方法，并提供一些实用的示例，旨在帮助初学者更好地理解和运用这一功能。

#### 什么是 Docker [Exec](https://so.csdn.net/so/search?q=Exec&spm=1001.2101.3001.7020) 命令？

Docker Exec 命令用于在正在运行的 Docker [容器](https://so.csdn.net/so/search?q=%E5%AE%B9%E5%99%A8&spm=1001.2101.3001.7020)内部执行命令。它允许用户与容器进行交互，从而可以在容器中执行各种操作，如运行 shell 脚本、安装软件包、查看日志等。使用 Docker Exec 命令，我们可以避免进入容器的复杂步骤，直接在容器内执行所需的操作。

Docker Exec 基本语法
```bash
docker exec [OPTIONS] CONTAINER COMMAND [ARG...]
```

- OPTIONS: 可选参数，用于指定一些附加选项，比如 -i（交互式）、-t（分配伪终端）等。
- CONTAINER: 要执行命令的目标容器名称或容器ID。
- COMMAND [ARG…]: 要在容器内执行的命令及其参数。

**Docker Exec 常用选项**

- -i, --interactive: 保持标准输入打开，允许用户与命令交互。
- -t, --tty: 分配一个伪终端，通常与 -i 一起使用以便在容器内部使用终端命令行。
- -u, --user: 指定执行命令的用户名或用户ID。
- -d, --detach: 在后台模式下执行命令。
- -e, --env: 设置环境变量。
- -w, --workdir: 指定命令的工作目录。
**注：更多参数可以运行`docker exec --help`查看**

#### Docker Exec 实例演示

1. 进入运行中的容器并执行命令
```bash
docker exec -it my_container bash
```
这个命令将在名为 `my_container` 的容器内启动一个交互式 bash 终端。

2. 在容器内运行命令
```bash
docker exec my_container ls /app
```
这个命令会在 `my_container` 容器内列出 `/app` 目录下的文件和文件夹。

3. 以非交互模式运行命令
```bash
docker exec -d my_container python script.py
```
这个命令会在后台模式下以非交互模式运行 script.py Python [脚本](https://so.csdn.net/so/search?q=%E8%84%9A%E6%9C%AC&spm=1001.2101.3001.7020)。

4. 在容器内设置环境变量并执行命令
```bash
docker exec -e MY_VAR=value my_container echo $MY_VAR
```
这个命令会在 `my_container` 容器内设置环境变量 `MY_VAR` 的值为 `value`，然后输出该变量的值。

5. 以root身份进入容器
```bash
docker exec -it -uroot tomcat_muller bash
```
这样的优势是我们可以用root权限在容器内进行操作，不用担心我们的权限不足。

#### 总结

Docker Exec 命令是 Docker 中一个非常实用的工具，可以让我们在运行中的容器内执行各种操作，而无需进入容器本身。通过本文的介绍和示例，相信您已经对 Docker Exec 的基本用法有了一定的了解。在实际应用中，您可以根据自己的需求灵活运用这一功能，提高 Docker 容器的管理效率。

[原文链接](https://blog.csdn.net/Hogwartstester/article/details/136211192)：https://blog.csdn.net/Hogwartstester/article/details/136211192