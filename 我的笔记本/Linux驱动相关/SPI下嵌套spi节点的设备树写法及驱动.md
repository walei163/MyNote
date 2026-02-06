# SPI下嵌套SPU节点的设备树写法及驱动设计（来自于copilot）

你可以在 `spi_cpld0` 节点下添加一个子节点，描述与 `spi3` 相关的配置信息。常见做法是添加一个 `spi3` 的 phandle（引用），或者直接添加一个子节点用于描述 spi3 的相关属性。这样，驱动代码可以通过解析 `spi_cpld0` 节点下的 `spi3` 子节点，获取并操作 `spi3`。

**示例：在 `spi_cpld0` 下添加 spi3 节点**

````dts
&spi2 {
	status = "okay";
	pinctrl-names = "default", "high_speed";
	pinctrl-0 = <&spi2m0_cs0 &spi2m0_cs1 &spi2m0_pins>;
	pinctrl-1 = <&spi2m0_cs0 &spi2m0_cs1 &spi2m0_pins_hs>;

	spi_cpld0: spi_cpld@0 {
		compatible = "sanway,spi_cpld";
		spi-max-frequency = <10000000>;
		reg = <0>;
		interrupt-parent = <&gpio3>;
		interrupts = <RK_PA6 IRQ_TYPE_EDGE_RISING>;

		// 新增spi3节点
		spi3 {
			compatible = "sanway,spi3";
			spi-controller = <&spi3>;
			spi-max-frequency = <10000000>;
			reg = <0>;
			// 你可以根据需要添加更多属性
		};
	};

	spidev1: spidev@1 {
		compatible = "rohm,dh2228fv";
		spi-max-frequency = <10000000>;
		reg = <1>;
	};
};
````

**说明：**
- `spi3` 子节点可以自定义命名（如 `spi3` 或 `spi3_dev`），`compatible` 字符串建议与驱动匹配。
- `spi-controller = <&spi3>;` 表示引用 SPI3 控制器节点，便于驱动中通过 phandle 获取。
- 你可以根据实际需求添加更多属性（如片选、速率等）。
- 驱动中可通过 `of_get_child_by_name()` 或 `of_get_next_child()` 获取 `spi3` 子节点，并解析相关属性。

如需更详细的驱动解析方法或设备树写法，可继续提问。

---

你可以在驱动中通过 `spi_new_device()` 创建并注册自己的 `spi_device`，这样你就可以直接用标准 SPI 接口（如 `spi_sync`、`spi_write_then_read` 等）访问两路 SPI 总线上的设备。

### 1. 设备树推荐写法

建议在根节点下新建一个“多 SPI 设备”节点，引用两个 SPI 控制器：

````dts
multi_spi_dev {
    compatible = "sanway,multi-spi-dev";
    spi2-controller = <&spi2>;
    spi3-controller = <&spi3>;
    status = "okay";
};
````

---

### 2. 驱动代码核心示例

```c
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>

struct multi_spi_dev {
	struct spi_device *spi2_dev;
	struct spi_device *spi3_dev;
};

static struct spi_device *create_spi_device(struct spi_master *master, u8 chip_select)
{
	struct spi_board_info spi_device_info = {
		.modalias = "spidev", // 或你的设备名
		.max_speed_hz = 10000000,
		.bus_num = 0, // 稍后赋值
		.chip_select = chip_select,
		.mode = SPI_MODE_0,
	};
	struct spi_device *spi;
	spi_device_info.bus_num = master->bus_num;
	spi = spi_new_device(master, &spi_device_info);
	return spi;
}

static int multi_spi_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *spi2_np, *spi3_np;
	struct spi_master *spi2_master, *spi3_master;
	struct multi_spi_dev *msd;

	msd = devm_kzalloc(&pdev->dev, sizeof(*msd), GFP_KERNEL);
	if (!msd)
		return -ENOMEM;

	spi2_np = of_parse_phandle(np, "spi2-controller", 0);
	spi3_np = of_parse_phandle(np, "spi3-controller", 0);
	if (!spi2_np || !spi3_np)
		return -ENODEV;

	spi2_master = of_find_spi_master_by_node(spi2_np);
	spi3_master = of_find_spi_master_by_node(spi3_np);
	of_node_put(spi2_np);
	of_node_put(spi3_np);
	if (!spi2_master || !spi3_master)
		return -ENODEV;

	msd->spi2_dev = create_spi_device(spi2_master, 0); // 片选0
	msd->spi3_dev = create_spi_device(spi3_master, 0); // 片选0

	// 现在你可以通过 msd->spi2_dev 和 msd->spi3_dev 进行 SPI 通信
	// 例如：spi_write_then_read(msd->spi2_dev, ...);

	return 0;
}

static const struct of_device_id multi_spi_of_match[] = {
	{ .compatible = "sanway,multi-spi-dev" },
	{ }
};
MODULE_DEVICE_TABLE(of, multi_spi_of_match);

static struct platform_driver multi_spi_driver = {
	.probe = multi_spi_probe,
	.driver = {
		.name = "multi_spi_dev",
		.of_match_table = multi_spi_of_match,
	},
};
module_platform_driver(multi_spi_driver);

MODULE_LICENSE("GPL");
```

---

### 3. 重点说明

- `create_spi_device()` 用于在指定的 `spi_master` 上创建 `spi_device`，你可以指定片选号、速率等参数。
- 你可以通过 `spi_write_then_read()`、`spi_sync()` 等接口直接操作 `msd->spi2_dev` 和 `msd->spi3_dev`。
- 设备树只需提供 SPI 控制器的 phandle，不需要在 SPI 控制器节点下声明 spidev 子节点。

如需更详细的 SPI 通信代码或有其他需求，请继续提问。