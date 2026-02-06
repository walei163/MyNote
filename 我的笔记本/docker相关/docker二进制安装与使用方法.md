# ubuntu 使用docker

# 1.安装

```bash
方法1：
更新软件包索引： sudo apt-get update
允许APT使用HTTPS： sudo apt-get install ca-certificates curl gnupg lsb-release
添加Docker官方GPG密钥：： curl -fsSL http://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg | sudo apt-key add -
添加Docker的稳定版本仓库：sudo add-apt-repository "deb [arch=amd64] http://mirrors.aliyun.com/docker-ce/linux/ubuntu $(lsb_release -cs) stable"
再次更新软件包索引： sudo apt-get update
安装Docker CE（社区版）： sudo apt-get install docker-ce
验证Docker是否安装成功并运行： sudo systemctl status docker

方法2：
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker ${USER}
```

方法3: //[Index of linux/static/stable/ (docker.com)](https://download.docker.com/linux/static/stable/)

```bash

下载解压：
cd /opt
wget https://download.docker.com/linux/static/stable/x86_64/docker-20.10.1.tgz
tar -xf docker-20.10.1.tgz
mv docker/* /usr/bin/

配置镜像加速：
mkdir -p /etc/docker
tee /etc/docker/daemon.json <<-'EOF'
{
  "registry-mirrors": ["https://b9pmyelo.mirror.aliyuncs.com"]
}
EOF

作为服务启动：
cat > /lib/systemd/system/docker.service << EOF
[Unit]
Description=Docker Application Container Engine
Documentation=https://docs.docker.com
After=network-online.target firewalld.service
Wants=network-online.target

[Service]
Type=notify
ExecStart=/usr/bin/dockerd --selinux-enabled=false --insecure-registry=127.0.0.1
ExecReload=/bin/kill -s HUP $MAINPID
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity
#TasksMax=infinity
TimeoutStartSec=0
Delegate=yes
KillMode=process
Restart=on-failure
StartLimitBurst=3
StartLimitInterval=60s

[Install]
WantedBy=multi-user.target
EOF

作为服务启动:
systemctl daemon-reload
systemctl start docker
systemctl enable docker
systemctl status docker

```

# 2.常用命令

```bash

docker ps 查看当前运行中的容器
docker ps -a 查看所有的容器

docker images 查看镜像列表

docker rm container-id 删除指定 id 的容器

docker stop/start container-id 停止/启动指定 id 的容器

docker rmi image-id 删除指定 id 的镜像

docker network ls 查看网络列表

docker volume ls 查看 volume 列表

docker build -t test:v1 . 编译 .-t 设置镜像名字和版本号

```

# 3.安装镜像  [Explore Docker's Container Image Repository | Docker Hub](https://hub.docker.com/search?q=)

```bash
1.安装ubuntu18.04镜像：sudo docker pull ubuntu:bionic  或者 sudo docker pull ubuntu:18.04
2.安装ubuntu16.04镜像：sudo docker pull ubuntu:xenial  或者 sudo docker pull ubuntu:16.04
3.安装ubuntu14.04镜像：sudo docker pull ubuntu:trusty  或者 sudo docker pull ubuntu:14.04
4.安装armhf ubuntu:14.04 docker pull arm32v7/ubuntu:14.04

删除镜像：sudo docker rmi ubuntu:trusty

镜像重命名：
sudo docker tag ubuntu:bionic ubuntu18.04:bionic
sudo docker rmi ubuntu:bionic
```

# 4.停止，启动，杀死，重启一个容器

```bash
docker stop Name或者ID  
docker start Name或者ID  
docker kill Name或者ID  
docker restart name或者ID

```

# 5.停止，启动，进入镜像(创建容器)

```bash

启动： sudo docker run -it <镜像名称>
停止： sudo docker stop <容器ID>

查看运行中的容器： docker ps 
查看所有挂起来的容器： docker ps -a

```

```bash
已经运行起来的容器，新增目录挂载：
# 在宿主机上创建目录
mkdir /path/on/host
 
# 进入容器
docker exec -it [容器ID或名称] /bin/bash
 
# 在容器内部创建挂载点（如果需要）
mkdir /path/in/container
 
# 挂载宿主机目录到新的挂载点
mount -t bind /path/on/host /path/in/container
```

```bash
创建容器时，将宿主机的目录映射到容器中： sudo docker run -it -v 宿主机目录:docker容器内的目录  ubuntu:18.04
容器启动后：ctr+p, +q退出容器，挂在后台运行；

```

```bash
进入一个挂在后台的容器： sudo docker attach  <容器id> 或 sudo docker exec -it <容器id>

```

# 6.容器退出时保持修改

```bash
步骤一：进入容器并进行修改
docker exec -it container_name /bin/bash
其中，container_name 是容器的名称或 ID。上述命令将以交互式的方式进入容器，并启动一个新的 Bash 终端。
在容器中，你可以执行任何需要的操作，例如安装软件包、修改配置文件等。完成修改后，可以通过 exit 命令退出容器。

步骤二：提交容器的修改
在退出容器之前，我们需要将容器的修改提交。可以使用以下命令提交容器的修改：
docker commit container_name new_image_name
其中，container_name 是容器的名称或 ID，new_image_name是新的镜像名称。上述命令将创建一个新的镜像，其中包含容器的修改。

步骤三：使用新的镜像运行容器
docker run -d --name container_name image_name
其中，container_name 是容器的名称，image_name 是新的镜像名称。上述命令将以后台模式运行一个新的容器。
```

# 7.双向拷贝文件

```bash
docker cp mysql8.0:/var/lib/mysql /var/data/MYSQL

docker cp /var/data/MYSQL mysql8.0:/var/lib/mysql 
```

错误1

```bash
[root@hadoop14 ~]# docker rmi ubuntu:v2
Failed to remove image (ubuntu:v2): Error response from daemon: conflict: unable to remove repository reference "ubuntu:v2" (must force) - container d63a819c3eaf is using its referenced image a8edd5cf3708

1.查看所有的容器：
docker ps -a -q
docker rm 容器id
```