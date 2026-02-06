# 关于内核SPI的cs-gpios配置说明

在Linux内核下，SPI总线除了自带的硬件cs片选之外，还可以采用GPIO来增加片选。其在Linux的设备树中的配置说明如下：

```yaml
cs-gpios:
    description: |
      GPIOs used as chip selects.
      If that property is used, the number of chip selects will be
      increased automatically with max(cs-gpios, hardware chip selects).

      So if, for example, the controller has 4 CS lines, and the
      cs-gpios looks like this
        cs-gpios = <&gpio1 0 0>, <0>, <&gpio1 1 0>, <&gpio1 2 0>;

      Then it should be configured so that num_chipselect = 4, with
      the following mapping
        cs0 : &gpio1 0 0
        cs1 : native
        cs2 : &gpio1 1 0
        cs3 : &gpio1 2 0
```

也就是说，可以将GPIO模式cs片选与SPI自身硬件cs片选（又称native模式）混合使用。

举例说明（T536的SPI0配置）：

```c
spi@4025000 {
	//pinctrl-0 = <&spi0_pins_default &spi0_pins_cs>;
	pinctrl-0 = <&spi0_pins_default>;
	pinctrl-1 = <&spi0_pins_sleep>;
	pinctrl-names = "default", "sleep";
	sunxi,spi-bus-mode = <SUNXI_SPI_BUS_MASTER>;	/*SUNXI_SPI_BUS_MASTER*/
	sunxi,spi-cs-mode = <SUNXI_SPI_CS_SOFT>;	/*SUNXI_SPI_CS_AUTO*/
	// spi-cpol;
    	// spi-cpha;
	cs-gpios = <&pio PC 3 GPIO_ACTIVE_LOW>; /* PC3 */
	status = "okay";
	spidev@0 {
		compatible = "rohm,dh2228fv";
		reg = <0x0>;
		spi-max-frequency = <10000000>;
		spi-rx-bus-width = <1>;
		spi-tx-bus-width = <1>;
		status = "disabled";
	};
	spiadc@0 {
		compatible = "sanway,spiadc";
		reg = <0>;
		spi-max-frequency = <50000000>;
		spi-rx-bus-width = <1>;
		spi-tx-bus-width = <1>;
		interrupt-parent = <&pio>;
		interrupts = <PC 7 IRQ_TYPE_EDGE_FALLING>;
		reset-gpios = <&pio PH 8 GPIO_ACTIVE_LOW>;
		//convst-gpios = <&pio PD 23 GPIO_ACTIVE_HIGH>;
		pwms = <&pwm0 0 1000000000 1>;		//PWM默认驱动频率100us，采用PWM0-0
		status = "okay";
	};
};
```

多个cs-gpios配置举例：

```c
spi0: spi@fffc8000 {
	cs-gpios = <0>, <&pioC 11 0>, <0>, <0>;
	mtd_dataflash@1 {
		compatible = "atmel,at45", "atmel,dataflash";
		spi-max-frequency = <50000000>;
		reg = <1>;
	};
};
```

