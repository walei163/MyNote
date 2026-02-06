### OTG device tree configuration
 
Contents↑

    1 Article purpose
    2 DT bindings documentation
    3 DT configuration
        3.1 DT configuration (STM32 level)
        3.2 DT configuration (board level)
            3.2.1 DT configuration using full-speed USB PHY
            3.2.2 DT configuration using high-speed USB PHY
        3.3 DT configuration examples
            3.3.1 DT configuration example as full-speed OTG
            3.3.2 DT configuration example as high speed OTG
    4 How to configure the DT using STM32CubeMX
    5 References

**1 Article purpose↑**

---

This article explains how to configure the OTG internal peripheral when it is assigned to the Linux® OS. In that case, it is controlled by the USB framework.

The configuration is performed using the device tree mechanism.

It is used by OTG Linux driver[1] which registers the relevant information in the USB framework.

**2 DT bindings documentation↑**

---

The Platform DesignWare HS OTG USB 2.0 controller device tree bindings[2] document represents the OTG (DRD) controller.

The Generic USB device tree bindings[3] document represents generic USB properties, proposed by USB framework such as maximum speed, dr_mode...

**3 DT configuration↑**

---
This hardware description is a combination of the STM32 microprocessor device tree files (.dtsi extension) and board device tree files (.dts extension). See the Device tree for an explanation of the device tree file split.

**STM32CubeMX** can be used to generate the board device tree. Refer to <font face="Arial" color=blue size=3>How to configure the DT using STM32CubeMX</font> for more details.

**3.1 DT configuration (STM32 level)↑**

---

The **usbotg_hs** DT node is declared in stm32mp157c.dtsi[4].

It is composed of a set of properties, used to describe the <font face="Arial" color=blue size=3>OTG controller</font>: registers address, clocks, resets, interrupts...

```
usbotg_hs: usb-otg@49000000 {
	compatible = "snps,dwc2";
	reg = <0x49000000 0x10000>;
	clocks = <&rcc USBO_K>;
	clock-names = "otg";
	resets = <&rcc USBO_R>;
	reset-names = "dwc2";
	interrupts = <GIC_SPI 98 IRQ_TYPE_LEVEL_HIGH>;
	dr_mode = "otg"; 	 	 	 	/* dr_mode[3] can be overwritten at board level to set a particular mode */
	status = "disabled";
};
```



|  ![40px-Warning](_v_images/20200408222536099_13582.png)   This device tree part is related to STM32 microprocessors. It must be kept as is, without being modified by the end-user.  |
| :-: |

**3.2 DT configuration (board level)↑**

---
Follow the sequences described in the below chapters to configure and enable the OTG on your board.

OTG supports two PHY interfaces that can be statically selected via DT:

+   full-speed PHY, embedded with the OTG controller
+    high-speed USBPHYC HS PHY that can be assigned to either the USBH or the OTG controller.

When operating in "otg" or "host" mode, an external charge pump, e.g. 5V regulator must be specified.


| ![40px-Info](_v_images/20200408222152642_18060.png) 	Please refer to Regulator overview for additional information on regulators configuration. |
| :----------------------------------------------------------------------------------------------------------------------------------------------: |

**3.2.1 DT configuration using full-speed USB PHY↑**

---

+ Enable the OTG by setting status = "okay".
+    Use embedded full-speed PHY by setting compatible = "st,stm32mp1-fsotg", "snps,dwc2"
+    Configure full-speed PHY pins (OTG ID, DM, DP) as analog via pinctrl, through pinctrl-0 and pinctrl-names.
+    Optionally set dual-role mode through dr_mode = "peripheral", "host" or "otg" (default to "otg" in case it is not set)
+    Optionally set vbus voltage regulator for otg and host modes, through vbus-supply = <&your_regulator>

**3.2.2 DT configuration using high-speed USB PHY↑**

---
+ Enable the OTG by setting status = "okay".
+ Select USBPHYC port#2 by setting phys = <&usbphyc_port1 0>; and phy-names = "usb2-phy";
+ Optionally configure OTG ID pin as analog via pinctrl, through pinctrl-0 and pinctrl-names.
+ Optionally set dual-role mode through dr_mode = "peripheral", "host" or "otg" (default to "otg" in case it is not set)
+ Optionally set vbus voltage regulator for otg and host modes, through vbus-supply = <&your_regulator>



| ![40px-Info](_v_images/20200408222152642_18060.png)	Please refer to USBPHYC device tree configuration for additional information on USBPHYC port#2 configuration |
| :--------------------------------------------------------------------------------------------------------------------------------------------------------------: |

**3.3 DT configuration examples↑**

---
**3.3.1 DT configuration example as full-speed OTG↑**

---

The example below shows how to configure full-speed OTG, with the ID pin to detect role (peripheral, host):

+ OTG ID and data (DM, DP) pins: use Pinctrl device tree configuration to configure PA10, PA11 and PA12 as analog input.
+ Use integrated full-speed USB PHY by setting compatible
+ Dual-role (dr_mode) is "otg" (e.g. the default as unspecified)
+ Use vbus voltage regulator

<br/>

```
# part of pin-controller dt node
usbotg_hs_pins_a: usbotg_hs-0 {
	pins {
		pinmux = <STM32_PINMUX('A', 10, ANALOG)>; /* OTG_ID */            /* configure 'PA10' as ANALOG */

	};
};
```

```
usbotg_fs_dp_dm_pins_a: usbotg-fs-dp-dm-0 {
	pins {
		pinmux = <STM32_PINMUX('A', 11, ANALOG)>, /* OTG_FS_DM */
			 <STM32_PINMUX('A', 12, ANALOG)>; /* OTG_FS_DP */
	};
};
```

```
&usbotg_hs {
	compatible = "st,stm32mp1-fsotg", "snps,dwc2";                            /* Use full-speed integrated PHY */
	pinctrl-names = "default";
	pinctrl-0 = <&usbotg_hs_pins_a &usbotg_fs_dp_dm_pins_a>;                  /* configure OTG ID and full-speed data pins */
	vbus-supply = <&vbus_otg>;                                                /* voltage regulator to supply Vbus */
	status = "okay";
};
```

**3.3.2 DT configuration example as high speed OTG↑**

---
The example below shows how to configure high-speed OTG, with the ID pin to detect role (peripheral, host):

+ OTG ID pin: use Pinctrl device tree configuration to configure PA10 as analog input.
+ Use USB HS PHY port#2, with the UTMI switch that selects the OTG controller
+ Dual-role mode (dr_mode) is "otg" (e.g. the default as unspecified)
+ Use vbus voltage regulator

<br/>

```
# part of pin-controller dt node
usbotg_hs_pins_a: usbotg_hs-0 {
	pins {
		pinmux = <STM32_PINMUX('A', 10, ANALOG)>; /* OTG_ID */            /* configure 'PA10' as ANALOG */

	};
};
```

```
&usbotg_hs {
	compatible = "st,stm32mp1-hsotg", "snps,dwc2";
	pinctrl-names = "default";
	pinctrl-0 = <&usbotg_hs_pins_a>;                                          /* configure OTG_ID pin */
	phys = <&usbphyc_port1 0>;                                                /* 0: UTMI switch selects the OTG controller */
	phy-names = "usb2-phy";
	vbus-supply = <&vbus_otg>;                                                /* voltage regulator to supply Vbus */
	status = "okay";                                                          /* enable OTG */
};
```

**4 How to configure the DT using STM32CubeMX↑**

---
The STM32CubeMX tool can be used to configure the STM32MPU device and get the corresponding platform configuration device tree files.
The STM32CubeMX may not support all the properties described in the above DT bindings documentation paragraph. If so, the tool inserts user sections in the generated device tree. These sections can then be edited to add some properties and they are preserved from one generation to another. Refer to STM32CubeMX user manual for further information.

**5 References↑**

---
Please refer to the following links for additional information:

drivers/usb/dwc2/ , DesignWare HS OTG Controller driver
Documentation/devicetree/bindings/usb/dwc2.txt , Platform DesignWare HS OTG USB 2.0 controller device tree bindings
Documentation/devicetree/bindings/usb/generic.txt , Generic USB device tree bindings
arch/arm/boot/dts/stm32mp157c.dtsi , STM32MP157C device tree file