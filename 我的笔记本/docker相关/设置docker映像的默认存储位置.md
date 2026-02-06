# 设置docker映像的默认存储位置

Docker 的默认存储位置通常在 `/var/lib/docker`。如果你想改变默认存储位置，可以通过配置 Docker 的守护进程来实现。

1. 停止 Docker 守护进程:

   ```sh
   bash sudo systemctl stop docker
   ```

2. 复制现有的 Docker 存储目录到新的位置:

   ```sh
   bash sudo rsync -aP /var/lib/docker/ /new/path/docker/ 
   ```

3. 编辑 Docker 服务文件 `/etc/docker/daemon.json`（如果不存在，则创建），如下所示:

   ```json
   {
     "registry-mirrors": ["https://atomhub.openatom.cn"],
     "data-root": "/media/jason/user_data_btrfs/docker_root_dir"
   }
   ```

4. 重新启动 Docker 守护进程

```sh
sudo systemctl start docker
```

确保 `/new/path/docker` 替换为你希望 Docker 存储镜像和容器的新路径。

注意：在你执行这些步骤之前，请确保你有足够的磁盘空间来存储 Docker 数据，并且你已经备份了任何重要数据。 

此时，可以用`docker info`来确认是否更改成功：

```sh
Docker Root Dir: /media/jason/user_data_btrfs/docker_root_dir
```

