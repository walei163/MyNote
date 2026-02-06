# [NBD（Network Block Device）简介及基本使用](https://www.cnblogs.com/yunlong-study/p/19034031)



### 一句话解释

`/dev/nbd3` 是一个 虚拟磁盘接口，可以把一个 文件（如 qcow2 镜像） 当作 真实磁盘 来读写。

### 类比理解

| 真实硬件    | 虚拟对应体      | 作用                   |
| ----------- | --------------- | ---------------------- |
| `/dev/sda`  | `/dev/nbd3`     | 都是“磁盘”设备节点     |
| SATA 控制器 | `qemu-nbd` 工具 | 把文件“插”到虚拟接口上 |

------

### 🔧 在本脚本中的用法

```bash
qemu-nbd -c /dev/nbd3 -f qcow2 encrypted-td.qcow2
```

 

这条命令把 `encrypted-td.qcow2` 这个 虚拟机磁盘文件 挂到 `/dev/nbd3`，之后你就能像操作真实磁盘一样对它分区、格式化、挂载。

------

### 📚 拓展知识点

- NBD 设备号范围默认是 `/dev/nbd0` 到 `/dev/nbd15`。

- 若提示 `/dev/nbd3` 不存在，先执行：

   

  ```bash
  sudo modprobe nbd
  ```

   

- 用完记得断开：

   

  ```bash
  sudo qemu-nbd -d /dev/nbd3
  ```

   

------

总结：`nbd3` 就是“第 3 号虚拟磁盘接口”，脚本用它来把 qcow2 镜像当作磁盘操作。

 

 

**下面的解释比较详细：**

NBD指的是[Network Block Device](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=Network+Block+Device&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJOZXR3b3JrIEJsb2NrIERldmljZSIsInpoaWRhX3NvdXJjZSI6ImVudGl0eSIsImNvbnRlbnRfaWQiOjEwMjc0ODAwLCJjb250ZW50X3R5cGUiOiJBcnRpY2xlIiwibWF0Y2hfb3JkZXIiOjEsInpkX3Rva2VuIjpudWxsfQ.q_Xeuh9IV9pVZqpGGvokU8Ec3t88ziKpSsrE3SU6QGk&zhida_source=entity)，正如其名字的意思，NBD让用户可以通过网络访问到某个块设备，或者设备镜像。

你可能会想[NFS](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=NFS&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJORlMiLCJ6aGlkYV9zb3VyY2UiOiJlbnRpdHkiLCJjb250ZW50X2lkIjoxMDI3NDgwMCwiY29udGVudF90eXBlIjoiQXJ0aWNsZSIsIm1hdGNoX29yZGVyIjoxLCJ6ZF90b2tlbiI6bnVsbH0.9jxv1snunHf4mLqCIGeULouIvcufABwS5-G_2KDTyQE&zhida_source=entity)（Network File System）不是已经可以实现通过网络挂载各种文件系统并进行访问了吗，为什么还要整个NBD呢？

如果用户的目的只是为了简单地访问对文件系统内的文件进行读写，那么NFS已经完全足够用了，但是有时候用户可能需要对存储块设备执行一些更底层的操作，这时候NFS可能就无法满足，需要NBD的方式了。简单地来说，NFS的方式只能管到文件系统层，而NBD的方式则不仅可以管到文件系统层，还可以管到更下面的块设备层。

比如，用户可能想要格式化哪个存储块设备，或者对存储块设备进行重新分区，这时候NFS的方式就无法满足该要求，因为在这些操作过程中，将会导致存在该存储块设备中的文件系统被卸载。对于NFS而言，卸载该文件系统的话，就会失去对该设备的连接。但是如果存储块设备是以NBD的方式挂载到系统中的话，则用户可以像操作本地存储块设备一样操作该通过NBD方式挂载的存储块设备，就像该存储块设备在本地就是通过[SCSI](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=SCSI&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJTQ1NJIiwiemhpZGFfc291cmNlIjoiZW50aXR5IiwiY29udGVudF9pZCI6MTAyNzQ4MDAsImNvbnRlbnRfdHlwZSI6IkFydGljbGUiLCJtYXRjaF9vcmRlciI6MSwiemRfdG9rZW4iOm51bGx9.Y189J91cBurBL6ZrPb1YujrKwCuJcIHPrhO9ZBxePwE&zhida_source=entity)或者[SATA](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=SATA&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJTQVRBIiwiemhpZGFfc291cmNlIjoiZW50aXR5IiwiY29udGVudF9pZCI6MTAyNzQ4MDAsImNvbnRlbnRfdHlwZSI6IkFydGljbGUiLCJtYXRjaF9vcmRlciI6MSwiemRfdG9rZW4iOm51bGx9.9iZoUddl7hlo6PVZHhqvAgyQh5g-6aVGRmloFVo-s40&zhida_source=entity)线连接着的。简单地说，NBD针对的是存储块设备，而NFS针对的是文件系统。

 

## 一、 如何使用NBD

NBD的工作方式也是遵循client/server的基本架构。在server端，通过设置将一个存储块设备或者镜像文件变成一个可以使用的网络块设备，然后在client端，连接挂载该设备进行使用。

 

Server端：Ubuntu IP:192.168.1.5

默认情况下，系统是没有安装NBD server的，在Ubuntu下可以执行下面命令进行安装：

sudo apt install nbd-server

安装完毕后，通过下面命令将NBD模块加载起来：

modprobe nbd

![img](https://pic4.zhimg.com/v2-74e53dd5d8c2bfce17d1c58855147e4d_1440w.jpg)

执行命令nbd-server 9999 nbd.file，将一个win7的安装镜像作为目标文件，查看nbd相关进程可以发现nbd-server已经运行起来了

![img](https://pic1.zhimg.com/v2-8077fcd9c49105076f9691e99d5b39e2_1440w.jpg)

 

Client端

执行apt install nbd-client指令安装nbd客户端

客户端主机页需要安装nbd模块，执行modprobe nbd，然后可以在/dev/目录下发现多了几个/dev/nbd#的设备。

![img](https://pic4.zhimg.com/v2-db3f4c28b85a478ec0af355d76509ebb_1440w.jpg)

执行nbd-client 192.168.1.5 9999 /dev/nbd0，将nbd设备关联到/dev/nbd0设备上。

![img](https://pic2.zhimg.com/v2-fddf778bc9961418fe2faa0270a8b7bf_1440w.jpg)

将/dev/nbd0p1挂载到/mnt/目录下，可以看到iso文件里面的内容了。

![img](https://pic3.zhimg.com/v2-a4525b28d910bd80716a83e5897ab77c_1440w.jpg)

 

## 二、[QEMU](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=QEMU&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJRRU1VIiwiemhpZGFfc291cmNlIjoiZW50aXR5IiwiY29udGVudF9pZCI6MTAyNzQ4MDAsImNvbnRlbnRfdHlwZSI6IkFydGljbGUiLCJtYXRjaF9vcmRlciI6MSwiemRfdG9rZW4iOm51bGx9.2wQgvj_MxSksoS5RF58MsE0d8NIcD-y5Fg21E4WXhnw&zhida_source=entity)中的NBD

虚拟化程序QEMU中的[qemu-nbd](https://zhida.zhihu.com/search?content_id=10274800&content_type=Article&match_order=1&q=qemu-nbd&zd_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJ6aGlkYV9zZXJ2ZXIiLCJleHAiOjE3NTUxNTcwMzIsInEiOiJxZW11LW5iZCIsInpoaWRhX3NvdXJjZSI6ImVudGl0eSIsImNvbnRlbnRfaWQiOjEwMjc0ODAwLCJjb250ZW50X3R5cGUiOiJBcnRpY2xlIiwibWF0Y2hfb3JkZXIiOjEsInpkX3Rva2VuIjpudWxsfQ.wQ328Fv5Q-3rrKRONjeVgqrGRvR5ycmnOi9YD9obXrM&zhida_source=entity)程序也是基于NBD方式对块设备或文件进行挂载，例如可以通过qemu-nbd命令查看一个虚拟机镜像文件的内部信息。

以一个win7.img的虚拟机镜像为例，首先用qemu-nbd命令将该镜像关联到NBD设备/dev/nbd3上（当然需要先加载ndb内核模块）

![img](https://pic4.zhimg.com/v2-22e8f0410b76bbeeb748f4dc386fe01f_1440w.jpg)

然后查看NBD设备，可以发现多出了两个nbd设备，即/dev/nbd3p1和/dev/nbd3p2，说明win7.img镜像中有两个分区。

![img](https://pic1.zhimg.com/v2-aacda42d75abf4b34b15558394780226_1440w.jpg)

分别将这两个设备文件挂载到文件系统中查看一下里面的内容：

![img](https://picx.zhimg.com/v2-d39829e478f422284fcd7d8e46b654e3_1440w.jpg)![img](https://pic1.zhimg.com/v2-6586675dff2c44c5f5d96c52d444154a_1440w.jpg)

可以发现/dev/nbd3p1其实就是系统的启动分区，而/dev/nbd3p2就是Win7的主分区，即Win7系统的C盘。

