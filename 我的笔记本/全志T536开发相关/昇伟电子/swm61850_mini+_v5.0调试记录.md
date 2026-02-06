# SWM61850_MINI+_V5.0调试记录

## 2026-01-05：

### 1、今天又修改了内核驱动代码，以增加PHY芯片：**AR8031/AR8033的光口通讯模式**。

驱动代码文件：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/kernel/linux-5.10-rt/drivers/net/phy/at803x.c](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/kernel/linux-5.10-rt/drivers/net/phy/at803x.c)

```c
//-----------------------------------------------------------
//RAY.Wang: 2026-01-04，增加8031/8033光纤模式寄存器定义
#define AR8031_PHY_SSR	0x11 /* PHY specific status register */
#define AR8031_CFSR	0x1b /* copper/fiber status register */
#define AR8031_CCR	0x1f /* chip configure register */

#define AR8031_FIBER	(1 << 11)
#define AR8031_COPPER	(1 << 3)
#define AR8031_FIBER_1000X	(1 << 8)

#define AR8031_COPPER_PAGE	(1 << 15)
//-----------------------------------------------------------

...
    
//RAY.Wang：2026-01-04，增加8031/8033光纤模式寄存器配置
static int at803x_config_init(struct phy_device *phydev)
{
	int ret;
	int i;
	int status, val;

	if(phydev->drv->phy_id == ATH8031_PHY_ID){
		val = phy_read(phydev, AR8031_CCR);

		for(i = 0; i < 10; i++){
			mdelay(100);
			status = phy_read(phydev, AR8031_CFSR);
		}
		
		phydev_warn(phydev, "AR8031 CFSR : %08x\n", status);
		if(status & AR8031_FIBER){
			//fiber
			phydev_warn(phydev, "Ar8031 fiber link up...\n");
			val &= ~AR8031_COPPER_PAGE;
			val |= AR8031_FIBER_1000X;
			phy_write(phydev, AR8031_CCR, val);

			#if 0
			if((status & (0x3 << 8)) == 0x0200){
				adver = phy_read(phydev, MII_ADVERTISE);
				//printk("MII_ADVERTISE : %08x\n", adver);
				adver |= (1 << 5);
				phy_write(phydev, MII_ADVERTISE, adver);
				
				ctrl1000 = phy_read(phydev, MII_CTRL1000);
				ctrl1000 |= ADVERTISE_1000FULL;
				phy_write(phydev, MII_CTRL1000, ctrl1000);

				phydev->supported |= SUPPORTED_1000baseT_Full;
				phydev->advertising |= SUPPORTED_1000baseT_Full;

				phydev->supported &= ~SUPPORTED_1000baseT_Half;
				phydev->advertising &= ~SUPPORTED_1000baseT_Half;
			}
			#endif

			phydev->autoneg = AUTONEG_DISABLE;

			if(status & (1 << 10))
				phydev->duplex = DUPLEX_FULL;
			else
				phydev->duplex = DUPLEX_HALF;
			if(((status >> 8) & 0x3) == 2)
				phydev->speed = SPEED_1000;
			else
				phydev->speed = SPEED_100;		
		} else {
			phydev_warn(phydev, "Ar8031 copper link up...\n");
			val |= AR8031_COPPER_PAGE;
			phy_write(phydev, AR8031_CCR, val);
		}

		phydev_warn(phydev, "Ar8031 CCR : %08x ...\n", phy_read(phydev, AR8031_CCR));
	
	} 
	// else {

		// ret = genphy_config_init(phydev);
		// if (ret < 0)
			// return ret;
	// }

	/* The RX and TX delay default is:
	 *   after HW reset: RX delay enabled and TX delay disabled
	 *   after SW reset: RX delay enabled, while TX delay retains the
	 *   value before reset.
	 */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
		ret = at803x_enable_rx_delay(phydev);
	else
		ret = at803x_disable_rx_delay(phydev);
	if (ret < 0)
		return ret;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		ret = at803x_enable_tx_delay(phydev);
	else
		ret = at803x_disable_tx_delay(phydev);
	if (ret < 0)
		return ret;

	ret = at803x_clk_out_config(phydev);
	if (ret < 0)
		return ret;

	if (at803x_match_phy_id(phydev, ATH8031_PHY_ID)) {
		ret = at8031_pll_config(phydev);
		if (ret < 0)
			return ret;
	}

	return 0;
}

...
    
//RAY.Wang: 2026-01-04，增加8031/8033光纤模式状态读取函数
int ar8031_fiber_read_status(struct phy_device *phydev)
{
	int err, ssr, speed, lpa;
	
	err = genphy_update_link(phydev);
	if(err)
		return err;
	
	ssr = phy_read(phydev, AR8031_PHY_SSR);
	
	if(ssr & (1 << 13))
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;
	
	speed = (ssr >> 14) & 0x3;
	if (speed == 2)
		phydev->speed = SPEED_1000;
	else if (speed == 1)
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	//phydev->lp_advertising |= ADVERTISED_1000baseT_Full;

	lpa = phy_read(phydev, MII_LPA);
	if(lpa < 0){
		phydev_err(phydev, "%s read MII_LPA error!\n", __func__);
		return lpa;
	}

	//phydev->lp_advertising |= mii_lpa_to_ethtool_lpa_t(lpa);

	// linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
		// phydev->lp_advertising, lpa | mii_lpa_to_ethtool_lpa_t(lpa));

	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseX_Full_BIT,
	 	phydev->lp_advertising, lpa & LPA_1000XFULL);

	phydev->autoneg = AUTONEG_DISABLE;
		
	phydev->pause = 0;
	phydev->asym_pause = 0;
	
	// phydev->state = PHY_FORCING;
	// phydev->link_timeout = PHY_FORCE_TIMEOUT;
	
	return 0;
}

static int at803x_copper_read_status(struct phy_device *phydev)
{
	int ss, err, old_link = phydev->link;

	/* Update the link, but return if there was an error */
	err = genphy_update_link(phydev);
	if (err)
		return err;

	/* why bother the PHY if nothing can have changed */
	if (phydev->autoneg == AUTONEG_ENABLE && old_link && phydev->link)
		return 0;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	err = genphy_read_lpa(phydev);
	if (err < 0)
		return err;

	/* Read the AT8035 PHY-Specific Status register, which indicates the
	 * speed and duplex that the PHY is actually using, irrespective of
	 * whether we are in autoneg mode or not.
	 */
	ss = phy_read(phydev, AT803X_SPECIFIC_STATUS);
	if (ss < 0)
		return ss;

	if (ss & AT803X_SS_SPEED_DUPLEX_RESOLVED) {
		int sfc;

		sfc = phy_read(phydev, AT803X_SPECIFIC_FUNCTION_CONTROL);
		if (sfc < 0)
			return sfc;

		switch (ss & AT803X_SS_SPEED_MASK) {
		case AT803X_SS_SPEED_10:
			phydev->speed = SPEED_10;
			break;
		case AT803X_SS_SPEED_100:
			phydev->speed = SPEED_100;
			break;
		case AT803X_SS_SPEED_1000:
			phydev->speed = SPEED_1000;
			break;
		}
		if (ss & AT803X_SS_DUPLEX)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;

		if (ss & AT803X_SS_MDIX)
			phydev->mdix = ETH_TP_MDI_X;
		else
			phydev->mdix = ETH_TP_MDI;

		switch (FIELD_GET(AT803X_SFC_MDI_CROSSOVER_MODE_M, sfc)) {
		case AT803X_SFC_MANUAL_MDI:
			phydev->mdix_ctrl = ETH_TP_MDI;
			break;
		case AT803X_SFC_MANUAL_MDIX:
			phydev->mdix_ctrl = ETH_TP_MDI_X;
			break;
		case AT803X_SFC_AUTOMATIC_CROSSOVER:
			phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
			break;
		}
	}

	if (phydev->autoneg == AUTONEG_ENABLE && phydev->autoneg_complete)
		phy_resolve_aneg_pause(phydev);

	return 0;
}

static int at803x_read_status(struct phy_device *phydev)
{
	int cbsr;
	
	cbsr = phy_read(phydev, AR8031_CFSR);
	if(cbsr & AR8031_FIBER){
		return ar8031_fiber_read_status(phydev);
	} else {
		return at803x_copper_read_status(phydev);
	}
}
```

经过上面的修改后，重新编译内核，启动系统，就可以支持光口的千兆以太网模式了。

### 2、光口的百兆以太网模式：

但是上面的修改，并不能支持光口的百兆以太网模式。经过研读AR8033的数据手册：

[/home/jason/BaiduSyncdisk/硬件资料/以太网相关/PHY/Qualcomm/Qualcomm-AR8033.pdf](/home/jason/BaiduSyncdisk/硬件资料/以太网相关/PHY/Qualcomm/Qualcomm-AR8033.pdf)

5.2.35 Chip configure register这一节的内容，可以设置光口的通讯模式：

寄存器：Offset: 0x1F

![image-20260105135639106](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/全志T536开发相关/昇伟电子/image-20260105135639106.png)

也就是说：

> [!important]
>
> **将寄存器偏移地址：0x1F的低4位设置为：0110（0110 = FX100_RGMII_50），即RGMII接口，FX100模式，阻抗50Ω，就可以按照百兆光口模式通讯了。**

---

> [!tip]
>
> 其实，原来我们已经在AM335x的MINI+的板子上做过这个工作，并且我当时还做了一个脚本文件：`rc.fiber_100FX`。
>
> 脚本内容如下：
>
> ```sh
> #!/bin/sh
> 
> #----------------------------------------------------------------------------
> #	Get macro define from /usr/local/sanway/ini/system.ini
> #----------------------------------------------------------------------------
> CONF_DIR=/usr/local
> CONF_MAC=$CONF_DIR/sanway/mac_addr.ini
> CONF=$CONF_DIR/sanway/ini/system.ini
> SECTION_APP=APP_SET
> SECTION_ETH=ETH_SET
> SECTION_MAC=MAC_SET
> SECTION_TCU=TCU_SET
> 
> ETH0=eth0
> ETH1=eth1
> 
> INFO=`cat $CONF \
>  | grep -v ^$ \
>  | sed -n "s/\s\+//;/\[${SECTION_APP}\]/,/^\[/p" \
>  | grep -v ^'\[' ` && eval "$INFO"
> 
> 
> INFO=`cat $CONF \
>  | grep -v ^$ \
>  | sed -n "s/\s\+//;/\[${SECTION_ETH}\]/,/^\[/p" \
>  | grep -v ^'\[' ` && eval "$INFO"
>  
> INFO=`cat $CONF_MAC \
>  | grep -v ^$ \
>  | sed -n "s/\s\+//;/\[${SECTION_MAC}\]/,/^\[/p" \
>  | grep -v ^'\[' ` && eval "$INFO"
> 
> #----------------------------------------------------------------------------
> #	Shell start here
> #----------------------------------------------------------------------------
> echo Setting $ETH0 to FIBER 100FX mode...
> $CONF_DIR/sanway/mdio $ETH0 0x1f 6
> 
> echo Setting $ETH1 to FIBER 100FX mode...
> $CONF_DIR/sanway/mdio $ETH1 0x1f 6
> 
> ```
>
> 该脚本就是将寄存器：0x1f设置为：6，即可按照百兆光口通讯了。

---

## 2026-01-04：

> [!warning]
>
> <font color=red>**测试发现一个问题，gmac1那一路网卡，千兆模式下可以ping通，但是百兆和十兆模式下无法ping通。**</font>

研究全志开发文档：

[/mnt/hgfs/H_DRIVE/全志T113i和T536官方资料_20250103/T536资料/20250821/Software软件类文档/SDK模块开发指南/Linux_EMAC_开发指南.pdf](/mnt/hgfs/H_DRIVE/全志T113i和T536官方资料_20250103/T536资料/20250821/Software软件类文档/SDK模块开发指南/Linux_EMAC_开发指南.pdf)

在8.4节：delay参数配置中，可以调节`tx-delay`和`rx-delay`这两个参数，如下所示：

![image-20260104120302232](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/全志T536开发相关/昇伟电子/image-20260104120302232.png)

![image-20260104120401451](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/全志T536开发相关/昇伟电子/image-20260104120401451.png)

按照文档中的描述，我们可以调节`tx-delay`和`rx-delay`参数：

```sh
cd /sys/devices/platform//soc@3000000/4510000.ethernet/
echo 15 > tx-delay
echo 11 > rx-delay
```

> [!note]
>
> **我的测试方法：先固定`tx-delay`为15，然后调节`rx-delay`，发现设置为20时不通，而原来设备树里默认设置为3，因此窗口为：[3, 19]，取中间值：11。**

---

再修改overlay设备树文件：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/dtbo/sanway/swm61850_mini_plus_v5.0.dts](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/dtbo/sanway/swm61850_mini_plus_v5.0.dts)

```c
//gmac0: AR8033, RGMII接口
ethernet@4500000 {
	compatible = "allwinner,sunxi-gmac-220", "snps,dwmac-5.10a";
	local-mac-address = [86 23 97 ba af 1a];
	//rx-delay = <0x00000003>;
	//tx-delay = <0x00000005>;
	//phy-mode = "rmii";
	rx-delay = <11>;
	tx-delay = <15>;
	phy-mode = "rgmii";
	mdio0: mdio0@0 {
		compatible = "snps,dwmac-mdio";
		#address-cells = <1>;
		#size-cells = <0>;
		gmac0_phy0: ethernet-phy@1 {
			compatible = "ethernet-phy-id004d.d074", "ethernet-phy-ieee802.3-c22";
			reg = <0x1>;
			max-speed = <1000>;  /* Max speed capability */
			reset-gpios = <&pio PA 3 GPIO_ACTIVE_LOW>;
			/* PHY datasheet rst time */
			reset-assert-us = <10000>;
			reset-deassert-us = <150000>;
		};
	};
};
//gmac1
ethernet@4510000 {
	compatible = "allwinner,sunxi-gmac-220", "snps,dwmac-5.10a";
	local-mac-address = [4a 57 15 91 df 31];
	rx-delay = <11>;
	tx-delay = <15>;
	phy-mode = "rgmii";
	mdio1: mdio1@0 {
		compatible = "snps,dwmac-mdio";
		#address-cells = <1>;
		#size-cells = <0>;
		gmac1_phy0: ethernet-phy@1 {
			compatible = "ethernet-phy-id004d.d074", "ethernet-phy-ieee802.3-c22";
			reg = <0x2>;
			max-speed = <1000>;  /* Max speed capability */
			reset-gpios = <&pio PA 4 GPIO_ACTIVE_LOW>;
			/* PHY datasheet rst time */
			reset-assert-us = <10000>;
			reset-deassert-us = <150000>;
		};
	};
};
```

然后将gmac1设置为百兆模式，再进行测试：

```sh
ethtool -s eth1 speed 100 duplex full
ping 192.168.2.177
```

**此时就可以ping通了。**

---

## 2025-12-30：

### 1、bootloader上电打印出错信息，卡死现象：

SWM61850_MINI+_V5.0硬件上焊接了1片1GB的LPDDR4x内存，是采用晶晨的芯片，结果用TF卡启动时，在SPL启动阶段报错，内存初始化失败。

经过和全志原厂杨廷瑞沟通，他给了一个新的DDR初始化参数，如下：

```ini
;*****************************************************************************
;sdram configuration
;
;*****************************************************************************
[dram_para]
dram_clk = 480
dram_type = 8
dram_dx_odt = 0x07070707
dram_dx_dri = 0x0e0e0e0e
dram_ca_dri = 0x0e0e
dram_para0 = 0x68686868
dram_para1 = 0x30fa
dram_para2 = 0x00000000
dram_mr0 = 0x0
dram_mr1 = 0x34
dram_mr2 = 0x00
dram_mr3 = 0x33
dram_mr4 = 0x3
dram_mr5 = 0x0
dram_mr6 = 0x0
dram_mr11 = 0x23
dram_mr12 = 0x72
dram_mr13 = 0x0
dram_mr14 = 0x0000000a
dram_mr16 = 0x0
dram_mr17 = 0x0
dram_mr22 = 0x06
dram_tpr0 = 0x7880
dram_tpr1 = 0x06060606
dram_tpr2 = 0x1f070503
dram_tpr3 = 0x6e000000
dram_tpr6 = 0x38000000
dram_tpr10 = 0x862f0000
dram_tpr11 = 0xc0c3c0c3
dram_tpr12 = 0x2e312d2e
dram_tpr13 = 0x00000C60
dram_tpr14 = 0x48484848
```

文件位置：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/configs/swa536/sys_config.fex](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/configs/swa536/sys_config.fex)

重新编译bootloader，并pack后，烧写`boot0_sdcard.fex`，再次启动，系统可以启动了。

> [!tip]
>
> **后续杨廷瑞又让我们将参数：`dram_clk = 480`改为：`dram_clk = 1200`，再次测试，也可以正常启动。**
>
> 按照杨廷瑞的说法，不需要设置DDR内存的容量，只要配置好参数，bootloader应能够自动获取到正确的内存容量大小。

---

### 2、千兆以太网调试：

#### 2.1. 修改设备树：

仍旧采用overlay dtb模式，在目录：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/dtbo/sanway/](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/dtbo/sanway/)

下新建文件：`swm61850_mini_plus_v5.0.dts`，修改千兆以太网设备配置如下：

```c
//采用AR8033千兆以太网，RGMII接口
gmac0_pins_default: gmac0@0 {
	pins = "PJ0", "PJ1", "PJ2", "PJ3",
		"PJ4", "PJ5", "PJ6", "PJ7",
		"PJ8", "PJ9", "PJ10", "PJ11",
		"PJ12", "PJ13", "PJ14", "PJ15";
	function = "rgmii0";
	drive-strength = <40>;
	bias-pull-up;
};
gmac0_pins_sleep: gmac0@1 {
	pins = "PJ0", "PJ1", "PJ2", "PJ3",
		"PJ4", "PJ5", "PJ6", "PJ7",
		"PJ8", "PJ9", "PJ10", "PJ11",
		"PJ12", "PJ13", "PJ14", "PJ15";
	function = "gpio_in";
};
gmac1_pins_default: gmac1@0 {
	pins = "PJ16", "PJ17", "PJ18", "PJ19",
		"PJ20", "PJ21", "PJ22", "PJ23",
		"PJ24", "PJ25", "PJ26", "PJ27",
		"PJ28", "PJ29", "PJ30", "PJ31";
	function = "rgmii1";
	drive-strength = <40>;
	bias-pull-up;
};
gmac1_pins_sleep: gmac1@1 {
	pins = "PJ16", "PJ17", "PJ18", "PJ19",
		"PJ20", "PJ21", "PJ22", "PJ23",
		"PJ24", "PJ25", "PJ26", "PJ27",
		"PJ28", "PJ29", "PJ30", "PJ31";
	function = "gpio_in";
};

...

//gmac0: AR8033, RGMII接口
ethernet@4500000 {
	compatible = "allwinner,sunxi-gmac-220", "snps,dwmac-5.10a";
	local-mac-address = [86 23 97 ba af 1a];
	rx-delay = <0x00000003>;
	tx-delay = <0x00000005>;
	//phy-mode = "rmii";
	phy-mode = "rgmii";
	mdio0: mdio0@0 {
		compatible = "snps,dwmac-mdio";
		#address-cells = <1>;
		#size-cells = <0>;
		gmac0_phy0: ethernet-phy@1 {
			compatible = "ethernet-phy-id004d.d074", "ethernet-phy-ieee802.3-c22";
			reg = <0x1>;
			max-speed = <1000>;  /* Max speed capability */
			reset-gpios = <&pio PA 3 GPIO_ACTIVE_LOW>;
			/* PHY datasheet rst time */
			reset-assert-us = <10000>;
			reset-deassert-us = <150000>;
		};
	};
};

//gmac1
ethernet@4510000 {
	compatible = "allwinner,sunxi-gmac-220", "snps,dwmac-5.10a";
	local-mac-address = [4a 57 15 91 df 31];
	rx-delay = <0x00000003>;
	tx-delay = <0x00000005>;
	phy-mode = "rgmii";
	mdio1: mdio1@0 {
		compatible = "snps,dwmac-mdio";
		#address-cells = <1>;
		#size-cells = <0>;
		gmac1_phy0: ethernet-phy@1 {
			compatible = "ethernet-phy-id004d.d074", "ethernet-phy-ieee802.3-c22";
			reg = <0x2>;
			max-speed = <1000>;  /* Max speed capability */
			reset-gpios = <&pio PA 4 GPIO_ACTIVE_LOW>;
			/* PHY datasheet rst time */
			reset-assert-us = <10000>;
			reset-deassert-us = <150000>;
		};
	};
};
```

经过这样的修改后，烧写带板子上，重启系统，可以找到设备，插入网线，也可以正常显示link up/down信息，但是无法ping通。

---

#### 2.2. 研究官方这一块设置的文档，发现在设备树中有两个属性很重要：

```c
aw,soc-phy25m;
aw,rgmii-clk-ext;
```

在soc级设备树源码：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/configs/linux-5.10-rt/sun55iw6p1.dtsi](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/configs/linux-5.10-rt/sun55iw6p1.dtsi)

默认配置了属性：

```c
aw,rgmii-clk-ext;
```

在board板级设备树源码：

[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/configs/swa536/board.dts](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/device/config/chips/t536/configs/swa536/board.dts)

默认配置属性：

```c
aw,soc-phy25m;
```

这两个属性的说明（可以参照驱动代码：[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/drivers/stmmac/dwmac-ethercat-sunxi.c](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/drivers/stmmac/dwmac-ethercat-sunxi.c)）：

```c
chip->soc_phy_clk_en = of_property_read_bool(np, "aw,soc-phy-clk-en") ||
						of_property_read_bool(np, "aw,soc-phy25m");
if (chip->soc_phy_clk_en) {
	chip->phy_clk = devm_clk_get(dev, "phy");
	if (IS_ERR(chip->phy_clk)) {
		chip->phy_clk = devm_clk_get(dev, "phy25m");
		if (IS_ERR(chip->phy_clk)) {
			sunxi_err(dev, "Get phy25m clk failed\n");
			return -EINVAL;
		}
	}
	sunxi_info(dev, "Phy use soc fanout\n");
} else
	sunxi_info(dev, "Phy use ext osc\n");
```

> aw,soc-phy25m：
>
> 如果设置了该属性，表示由T536 SOC芯片对外提供25MHz时钟。
>
> 如果没有设置该属性，表示外部PHY芯片自带25MHz晶振时钟。

而我们的硬件设计，是采用PHY自带25MHz晶振，因此，需要在设备树中去掉`aw,soc-phy25m`属性的定义。

---

对于属性：

```c
aw,rgmii-clk-ext;
```

阅读驱动代码：

```c
chip->rgmii_clk_ext	= of_property_read_bool(np, "aw,rgmii-clk-ext");
...
static int sunxi_dwmac200_set_syscon(struct sunxi_dwmac *chip)
{
	u32 reg_val = 0;

	/* Clear interface mode bits */
	reg_val &= ~(SUNXI_DWMAC200_SYSCON_ETCS | SUNXI_DWMAC200_SYSCON_EPIT);
	if (chip->variant->interface & PHY_INTERFACE_MODE_RMII)
		reg_val &= ~SUNXI_DWMAC200_SYSCON_RMII_EN;

	switch (chip->interface) {
	case PHY_INTERFACE_MODE_MII:
		/* default */
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		reg_val |= SUNXI_DWMAC200_SYSCON_EPIT;
		reg_val |= FIELD_PREP(SUNXI_DWMAC200_SYSCON_ETCS,
					chip->rgmii_clk_ext ? SUNXI_DWMAC_ETCS_EXT_GMII : SUNXI_DWMAC_ETCS_INT_GMII);
		if (chip->rgmii_clk_ext)
			sunxi_info(chip->dev, "RGMII use external transmit clock\n");
		else
			sunxi_info(chip->dev, "RGMII use internal transmit clock\n");
		break;
	case PHY_INTERFACE_MODE_RMII:
		reg_val |= SUNXI_DWMAC200_SYSCON_RMII_EN;
		reg_val &= ~SUNXI_DWMAC200_SYSCON_ETCS;
		break;
	default:
		sunxi_err(chip->dev, "Unsupported interface mode: %s", phy_modes(chip->interface));
		return -EINVAL;
	}

	writel(reg_val, chip->syscfg_base + SUNXI_DWMAC200_SYSCON_REG);
	return 0;
}
```

> 可知，当配置了属性时，变量：`chip->rgmii_clk_ext`使能，表示采用外部PHY提供的发送时钟；
>
> 如果没有配置`aw,rgmii-clk-ext`属性，此时表示采用T536 SOC内部的发送时钟；

而我们的硬件设计，PHY没有提供发送时钟，而是需要由CPU来输出发送时钟的，因此，这个属性也要去掉。不过采用RTL8201的PHY芯片的硬件设计，是提供了发送时钟的。

> [!important]
>
> **将这两个属性从设备树中去掉后，就可以ping通了。并且，经过iperf3的测试，传输速度可以达到千兆。**