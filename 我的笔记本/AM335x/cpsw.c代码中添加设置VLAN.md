# cpsw.c代码中添加设置VLAN

---

> [!caution]
>
> <font face="微软雅黑" color=yellow>**问题描述：廖工这边反馈，深圳电业局在基于`GOOSE`报文收发时，明确要求需要在以太网包中添加指定的`VLAN`的ID号。**</font>
>
> **因为`AM335x`的2路以太网在`dual_emac`模式下，其实是采用的按照`VLAN`的ID号来区分以太网包的流向，且默认在DTB设备文件树中将其设死为01和02。结果导致在收发报文时，可以发出不同`VLAN`ID的报文，但是接收报文时，如果和其默认设置的ID不同，就收不到该报文。**

因此，需要在`AM335x`自带的2路以太网驱动：`cpsw.c`中，增加一种方法，能够可以让用户层的程序去设置这2路以太网的`VLAN` ID号，这样才能解决该问题。

经过研究`cpsw.c`，可以采用以下模式来实现：

1. **利用以太网`socket`的`ioctl`方法，向内核层传递要设置的`VLAN` ID号；**
2. **修改驱动代码，将`VLAN` ID作为新的ID，然后去过滤`GOOSE`数据报文。**

我们可以在内核层修改以太网设备`net_device_ops`的`ndo_do_ioctl`函数，增加`SIOCDEVPRIVATE`作为设置。代码如下：

```c
//	RAY.Wang:2024-0524，修改该函数，
//	增加2个ioctl：SIOCDEVPRIVATE和SIOCDEVPRIVATE + 1（具体定义见include/uapi/linux/sockios.h），
//	用于获取或者设置在dual_mac方式下的dual_emac_res_vlan的值
static int cpsw_ndo_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	struct cpsw_priv *priv = netdev_priv(dev);
	struct cpsw_common *cpsw = priv->cpsw;
	int slave_no = cpsw_slave_index(cpsw, priv);

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCSHWTSTAMP:
		return cpsw_hwtstamp_set(dev, req);
	case SIOCGHWTSTAMP:
		return cpsw_hwtstamp_get(dev, req);
	case SIOCSWITCHCONFIG:
		return cpsw_switch_config_ioctl(dev, req, cmd);
	case SIOCDEVPRIVATE:
		return cpsw_port_vlan_get(dev, req);
	case (SIOCDEVPRIVATE + 1):
		return cpsw_port_vlan_set(dev, req);
	}

	if (!cpsw->slaves[slave_no].phy)
		return -EOPNOTSUPP;
	return phy_mii_ioctl(cpsw->slaves[slave_no].phy, req, cmd);
}
```

实现2个函数：

- `cpsw_port_vlan_get(dev, req)`：获取当前的`VLAN` ID号；

该函数实现如下：

```c
static int cpsw_port_vlan_get(struct net_device *dev, struct ifreq *ifr)
{
	struct cpsw_common *cpsw = ndev_to_cpsw(dev);
	struct cpsw_priv *priv = netdev_priv(dev);
	u32 slave_no = cpsw_slave_index(cpsw, priv);
	u32 port_vlan = cpsw->slaves[slave_no].port_vlan;
	int ret = -1;

	dev_info(priv->dev, "eth%d's dual_emac_res_vlan(port_vlan) value: 0x%04x.\n", 
		 slave_no, port_vlan);

	ret = copy_to_user(ifr->ifr_ifru.ifru_data, &port_vlan, sizeof(port_vlan));
	if (ret < 0) {
		dev_err(priv->dev, "Copy to user space failed, ret: %d", ret);
	}

	return ret;
}
```



- `cpsw_port_vlan_set(dev, req)`：设置新的`VLAN` ID号。

该函数实现如下：

```c
static int cpsw_port_vlan_set(struct net_device *dev, struct ifreq *ifr)
{
	struct cpsw_common *cpsw = ndev_to_cpsw(dev);
	struct cpsw_priv *priv = netdev_priv(dev);
	u32 slave_no = cpsw_slave_index(cpsw, priv);
	u32 port_vlan;
	int ret = -1;

	ret = copy_from_user(&port_vlan, ifr->ifr_ifru.ifru_data, sizeof(port_vlan));
	if (ret < 0) {
		dev_err(priv->dev, "Copy from user space failed, ret: %d", ret);
		return -EFAULT;
	}

	if (port_vlan == cpsw->slaves[slave_no].port_vlan) {
		dev_info(priv->dev, "eth%d's current port_vlan: 0x%04x, user set value: 0x%04x, not need change.\n", 
			 slave_no, cpsw->slaves[slave_no].port_vlan, port_vlan);
		return ret;
	}

	cpsw->slaves[slave_no].port_vlan = port_vlan;


	if (cpsw->data.dual_emac) {
		//struct cpsw_slave *slave = &cpsw->slaves[cpsw->data.active_slave];
		struct cpsw_slave *slave = &cpsw->slaves[slave_no];
		u32 slave_port = cpsw_get_slave_port(slave->slave_num);

		cpsw_add_dual_emac_def_ale_entries(priv, slave, slave_port);
		dev_info(priv->dev, "eth%d's cpsw_add_dual_emac_def_ale_entries: slave_num: 0x%04x, slave_port: 0x%04x.\n",
			 slave_no, slave->slave_num, slave_port);
	}

	dev_info(priv->dev, "eth%d's port_vlan value is now set to: 0x%04x.\n", 
		 slave_no, cpsw->slaves[slave_no].port_vlan);

	return ret;
}
```

