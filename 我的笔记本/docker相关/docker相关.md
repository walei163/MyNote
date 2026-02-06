## 一、docker命令去掉`sudo`的方法：

是因为当前用户jason没有被添加到`docker`组里，所以只需要将用户jason添加到组：`docker`里就可以了：

```bash
sudo adduser jason docker
```

更新一下用户组：

```bash
newgrp docker
```

测试一下docker命令是否可以不带sudo的情况下正常使用：

```bash
docker version
```

---
## 二、docker常用命令：

### 1. 拉取`armhf`的`ubuntu16.04`的镜像文件：

```bash
docker pull arm32v7/ubuntu:16.04
```

### 2. 运行容器：

```bash
docker run -it --rm arm32v7/ubuntu:16.04
```

> [!tip]
>
> - `-it`表示开启一个交互式的终端。
>
> - `--rm`表示容器运行推出后自动清除挂载卷，等价在容器推出后执行`docker rm -v`。指定`--rm`参数时通常不指定`-d`参数。



### 3. 停止运行：

```bash
docker container stop container_id
```

### 4. 查看映像：

```bash
docker image ls
```

如下所示：

<div align=left><img src="https://img-blog.csdnimg.cn/260747c23f7e4f7dab93e6062fb44e36.png" width="1200" height="120"></div>

### 5. 移除`docker`映像：

```bash
docker image rm -f 映像名称
```

### 6. `docker`和宿主机之间共享文件：

只需要在启动时添加`-v`参数即可。

例如：将宿主机的`/home/jason`目录共享到docker容器的`/mnt`目录下：

```bash
docker run -it -v /home/jason:/mnt arm32v7/ubuntu:16.04 (/bin/bash)
```

### 7. 保存当前`docker`容器：

```bash
docker commit 3b1d0e62eeb7（当前运行容器的ID） ubuntu16.04_armhf:v1.0（新的镜像的名称：tag）
```

例如：将当前容器ID为：`4b8ab7a832ad`的docker保存：

```bash
docker commit -a "RAY.Wang" -m "added bc and file" 4b8ab7a832ad  ubuntu16.04_armhf:v2.2
```

> [!tip]
>
> OPTIONS说明：
>
> - `-m`：表示添加说明性的文字。
> - `-a`：提交的镜像作者。
> - `-c`：使用`Dockerfile`指令来创建镜像。
> - `-p`：在commit时，将容器暂停。
>

如果需要获取当前容器的ID，则可以使用以下命令：

```bash
docker ps
```

还可以使用`docker history`来查看容器都经历了哪些操作：

```bash
docker history ubuntu16.04_armhf:v2.2
```

### 8. 运行新的镜像：

```bash
docker run -it -v /home/jason:/mnt ubuntu16.04_armhf:v2.2
```

### 9. 将镜像保存成外部的一个文件：

`docker save`可用于将当前容器保存成一个外部的文件：

```bash
docker save ubuntu16.04_armhf:v2.1 -o ubuntu16.04_armhf_docker_image_backup_20240430.img
```

### 10. 加载保存的镜像文件：

```bash
docker load -i ubuntu16.04_armhf_docker_image_backup_20240430.img
```

### 11. 容器的退出

进入docker容器后如果退出容器，容器就会变成Exited的状态。特别的加了rm参数，关闭终端后，容器将删除。那么如何退出容器让容器不关闭呢？

1）如果要正常退出不关闭容器，先按`ctrl+p`、再按 `ctrl+q` 退出容器，这一点很重要，请牢记！

>  [!important]
>
>  <font face="微软雅黑" color=yellow>**注意：绝对不能使用exit或者ctrl+d来退出，这样整个系统就退出了！！！**</font>

此时，用`docker ps -a`查看，容器仍为UP装态。

2）或者创建时加`-d`，后台运行（不推荐后台运行）。

### 12. 容器的进入

1）对于处在UP状态的容器，可使用`docker exec`进入容器，如：
```bash
docker exec -it container_id /bin/bash
```

2）或者可以使用attach：

```bash
docker attach container_id
```