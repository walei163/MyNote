# LBC总线相关

大伏羲板卡资料目录：[/media/sf_H_DRIVE/全志T113i和T536官方资料_20250103/吴伟桦给的资料/3-软件资料/可用lbc/lbc/](/media/sf_H_DRIVE/全志T113i和T536官方资料_20250103/吴伟桦给的资料/3-软件资料/可用lbc/lbc/)

---

## 一、大伏羲核心板的设备树中，对于LBC这块的配置如下：

1. ### `sun55iw6p1.dtsi`中的配置：

```c
lbc: lbc-controller@2810000 {
			compatible = "allwinner,sunxi-lbc";
			clock-frequency = <33000000>;
			transfer_mode = <2>;
			transfer_width = <1>;
			chain_delay = <30>;
			burst_mode = <0>;
			reg = <0x0 0x02810000 0x0 0x1000>,
				  <0x0 0x10000000 0x0 0x8000000>;
			reg-names = "reg_addr", "data_addr";
			interrupts = <GIC_SPI 48 IRQ_TYPE_LEVEL_HIGH>;
			clocks = <&ccu CLK_PLL_PERI0_400M>, <&ccu CLK_PLL_PERI0_200M>,
					 <&ccu CLK_LBC_NSI_AHB>, <&ccu CLK_BUS_LBC>, <&ccu CLK_LBC>;
			clock-names = "lbc_pll", "lbc_nsi_pll", "lbc_nsi_ahb", "bus_lbc", "lbc";
			resets = <&ccu RST_BUS_LBC>;
			reset-names = "lbc_rst";
 			reset-fpga-gpios = <&pio PA 7 GPIO_ACTIVE_LOW>;
			convst-adc-gpios = <&pio PB 6 GPIO_ACTIVE_LOW>;
			reset-adc-gpios = <&pio PD 0 GPIO_ACTIVE_LOW>;
 			direct-gpios = <&pio PA 8 GPIO_ACTIVE_LOW>;
 			// adc-cs-gpios = <&pio PE 0 GPIO_ACTIVE_HIGH>;
 			status = "okay";
		};
```

其中：

- `clock-frequency`被配置成了33MHz；
- `burst_mode`配置为：0（不启用突发模式）

2. ### `board.dts`中的与LBC相关的配置：

```c
lbc_pins_a: lbc_pins_a@0 {
		pins = "PE4", "PE5", "PE6", "PE7",
			"PE8", "PE9", "PE10", "PE11",
			"PE12", "PE13", "PE14", "PE15",
			 "PE17",
			"PK0", "PK1", "PK2", "PK3",
			"PK4", "PK5", "PK6", "PK7",
			"PK8", "PK9", "PK10", "PK11",
			"PK12", "PK13", "PK14", "PK15";
		function = "lbus";
		drive-strength = <30>;
		bias-pull-up;
	};

	lbc_pins_b: lbc_pins_b@0 {
		pins = "PE4", "PE5", "PE6", "PE7",
			"PE8", "PE9", "PE10", "PE11",
			"PE12", "PE13", "PE14", "PE15",
			 "PE17",
			"PK0", "PK1", "PK2", "PK3",
			"PK4", "PK5", "PK6", "PK7",
			"PK8", "PK9", "PK10", "PK11",
			"PK12", "PK13", "PK14", "PK15";
		function = "gpio_in";
	};

...
    
&lbc {
	lbc-io-supply = <&reg_bldo3>;
	lbc-io-vol = <1800000>;
	pinctrl-names = "default", "sleep";
	pinctrl-0 = <&lbc_pins_a>;
	pinctrl-1 = <&lbc_pins_b>;
	status = "okay";
};
```

3. ### `pinctrl-sun55iw6.c`管脚配置源码：

   经过对比和代码树：[/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/drivers/pinctrl/](/media/jason/btrfs_disk_M/Develop/allwinner/t536/V1.1/bsp/drivers/pinctrl/)下的同名文件内容完全一致。