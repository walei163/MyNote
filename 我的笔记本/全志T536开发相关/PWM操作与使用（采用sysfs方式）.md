# PWM操作与使用（采用sysfs方式）

## 一、内核配置PWM：

在SWA536-H2核心板上，定义了4路PWM输出，如下图：

![image-20250506105627800](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/全志T536开发相关/images/image-20250506105627800.png)

对应到CPU的GPIO：PD0~PD3。

内核的DTB配置如下（sun55iw6p1）：

```c
pwm0: pwm@2090000 {
	#pwm-cells = <0x3>;
	compatible = "allwinner,sunxi-pwm-v203";
	reg = <0x0 0x02090000 0x0 0x400>;
	clocks = <&ccu CLK_PWM0>;
	interrupts = <GIC_SPI 19 IRQ_TYPE_LEVEL_HIGH>;
	resets = <&ccu RST_BUS_PWM0>;
	pwm-number = <10>;
	pwm-base = <0x0>;
	sunxi-pwms = <&pwm0_0>, <&pwm0_1>, <&pwm0_2>, <&pwm0_3>, <&pwm0_4>,
	<&pwm0_5>, <&pwm0_6>, <&pwm0_7>, <&pwm0_8>, <&pwm0_9>;
	status = "okay";
};
pwm0_0: pwm@2090010 {
	compatible = "allwinner,sunxi-pwm0";
	reg = <0x0 0x02090010 0x0 0x4>;
	reg_base = <0x02090000>;
	status = "disabled";
};
pwm0_1: pwm@2090011 {
	compatible = "allwinner,sunxi-pwm1";
	reg = <0x0 0x02090011 0x0 0x4>;
	reg_base = <0x02090000>;
	status = "disabled";
};
pwm0_2: pwm@2090012 {
	compatible = "allwinner,sunxi-pwm2";
	reg = <0x0 0x02090012 0x0 0x4>;
	reg_base = <0x02090000>;
	status = "disabled";
};
pwm0_3: pwm0@2090013 {
	compatible = "allwinner,sunxi-pwm3";
	reg = <0x0 0x02090013 0x0 0x4>;
	reg_base = <0x02090000>;
	status = "disabled";
};
```

板级配置如下（board.dts）：

```c
&pwm0 {
	status = "okay";
};

&pwm0_0 {
	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&pwm0_0_pins_active>;
	pinctrl-1 = <&pwm0_0_pins_sleep>;
	status = "okay";
};

&pwm0_1 {
	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&pwm0_1_pins_active>;
	pinctrl-1 = <&pwm0_1_pins_sleep>;
	status = "okay";
};

&pwm0_2 {
	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&pwm0_2_pins_active>;
	pinctrl-1 = <&pwm0_2_pins_sleep>;
	status = "okay";
};

&pwm0_3 {
	pinctrl-names = "active", "sleep";
	pinctrl-0 = <&pwm0_3_pins_active>;
	pinctrl-1 = <&pwm0_3_pins_sleep>;
	status = "okay";
};

...

&pio {
    pwm0_0_pins_active: pwm0@0 {
        pins = "PD0";
        function = "pwm0_0";
    };
    pwm0_0_pins_sleep: pwm0@1 {
        pins = "PD0";
        function = "gpio_in";
        bias-pull-down;
    };
    pwm0_1_pins_active: pwm0_1@0 {
        pins = "PD1";
        function = "pwm0_1";
    };

    pwm0_1_pins_sleep: pwm0_1@1 {
        pins = "PD1";
        function = "gpio_in";
        bias-pull-down;
    };
    pwm0_2_pins_active: pwm0_2@0 {
        pins = "PD2";
        function = "pwm0_2";
    };
    pwm0_2_pins_sleep: pwm0_2@1 {
        pins = "PD2";
        function = "gpio_in";
        bias-pull-down;
    };
    pwm0_3_pins_active: pwm0_3@0 {
        pins = "PD3";
        function = "pwm0_3";
    };
    pwm0_3_pins_sleep: pwm0_3@1 {
        pins = "PD3";
        function = "gpio_in";
        bias-pull-down;
    };
}
```

如果设置无误，则系统启动后的信息：

```sh
# dmesg |grep pwm
[    0.750784] sunxi:pwm-2090000.pwm:[INFO]: start probe
[    0.752114] sunxi:pwm-2090000.pwm:[INFO]: pwmchip probe success
```

表明PWM已经正确加载了，接下来操作PWM。

## 二、采用sysfs方式操作PWM：

### 1、PWM的系统目录：

```sh
/sys/class/pwm
```

如果系统加载正确，在该目录下会出现设备名称：

```sh
# ls
pwmchip0
```

继续进入到pwmchip0目录下：

```sh
# ll
total 0      
lrwxrwxrwx    1 root     root           0 Jan  1 00:02 device -> ../../../2090000.pwm
--w-------    1 root     root        4.0K Jan  1 00:14 export
-r--r--r--    1 root     root        4.0K Jan  1 00:02 npwm
drwxr-xr-x    2 root     root           0 Jan  1 00:02 power
lrwxrwxrwx    1 root     root           0 Jan  1 00:02 subsystem -> ../../../../../../class/pwm
-rw-r--r--    1 root     root        4.0K Jan  1 00:02 uevent
--w-------    1 root     root        4.0K Jan  1 00:02 unexport
```

此时可以通过export方式，导出PWM设备：

```sh
echo 0 > export
echo 1 > export
echo 2 > export
echo 3 > export
```

出现pwm0~pwm3这4路pwm设备：

```sh
# ll
total 0      
lrwxrwxrwx    1 root     root           0 Jan  1 00:02 device -> ../../../2090000.pwm
--w-------    1 root     root        4.0K Jan  1 00:14 export
-r--r--r--    1 root     root        4.0K Jan  1 00:02 npwm
drwxr-xr-x    2 root     root           0 Jan  1 00:02 power
drwxr-xr-x    3 root     root           0 Jan  1 00:12 pwm0
drwxr-xr-x    3 root     root           0 Jan  1 00:13 pwm1
drwxr-xr-x    3 root     root           0 Jan  1 00:13 pwm2
drwxr-xr-x    3 root     root           0 Jan  1 00:13 pwm3
lrwxrwxrwx    1 root     root           0 Jan  1 00:02 subsystem -> ../../../../../../class/pwm
-rw-r--r--    1 root     root        4.0K Jan  1 00:02 uevent
--w-------    1 root     root        4.0K Jan  1 00:02 unexport
```

### 2、操作pwm设备（以PWM0为例）

```sh
# cd pwm0
# ll
total 0      
-r--r--r--    1 root     root        4.0K Jan  1 00:12 capture
-rw-r--r--    1 root     root        4.0K Jan  1 00:17 duty_cycle
-rw-r--r--    1 root     root        4.0K Jan  1 00:17 enable
-rw-r--r--    1 root     root        4.0K Jan  1 00:16 period
-rw-r--r--    1 root     root        4.0K Jan  1 00:17 polarity
drwxr-xr-x    2 root     root           0 Jan  1 00:12 power
-rw-r--r--    1 root     root        4.0K Jan  1 00:12 uevent
```

period：设置PWM输出波形的周期值，单位为ns。

duty_cycle：设置PWM输出波形的占空比（高电平的时间，单位也为ns）。

polarity：设置极性。可以为：normal或者inversed

enable：使能并启动PWM。1 - 使能；0 - 禁止。

例如：

```sh
echo 20000000 >period //设置周期
echo 2000000 >duty_cycle //设置高电平的时间。
echo normal > polarity //设置极性,有 normal 或 inversed 两个参数选择
echo 1 > enable //使能pwm
echo 0 > enable //失能pwm
```

