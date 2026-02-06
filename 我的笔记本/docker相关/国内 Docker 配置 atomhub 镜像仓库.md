# 国内 Docker 配置 atomhub 镜像仓库

Docker Hub 镜像被禁，在国内可以使用下面这个镜像源，目前只有一些基础镜像

**编辑Docker配置文件**: 打开或创建 `/etc/docker/daemon.json` 文件

```json
{
  "registry-mirrors": ["https://atomhub.openatom.cn"]
}
```

**重启Docker服务**: 为使配置生效，请执行以下命令：

```sh
sudo systemctl daemon-reload
sudo systemctl restart docker
```

### 官网地址

[OpenAtom Foundation - 开放原子开源基金会](https://link.zhihu.com/?target=https%3A//atomhub.openatom.cn/)

加载docker镜像时，按照以下方法：

```sh
docker pull atomhub.openatom.cn/arm64v8/ubuntu:23.10
```

