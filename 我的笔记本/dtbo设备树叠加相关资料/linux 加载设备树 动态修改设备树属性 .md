# [linux 加载设备树 动态修改设备树属性 ](https://www.cnblogs.com/jiejunwang/p/18715868)

在Linux 系统中，设备树（Device Tree）是描述硬件配置的数据结构，通常在启动时由 Bootloader（如 U-Boot）加载并传递给内核。然而，Linux 内核也支持在运行时动态修改设备树属性。以下是动态修改设备树属性的方法：

------

### 1. **通过 `/sys/firmware/devicetree` 修改属性**

Linux 内核将设备树信息映射到 `/sys/firmware/devicetree` 文件系统中。可以通过该文件系统查看和修改设备树属性。

#### 查看设备树属性

设备树节点和属性以目录和文件的形式存在于 `/sys/firmware/devicetree` 中。例如，查看某个节点的 `status` 属性：

```sh
cat /sys/firmware/devicetree/base/soc/i2c@12340000/status
```

　　

#### 修改设备树属性

设备树属性在 `/sys/firmware/devicetree` 中是只读的，无法直接修改。如果需要动态修改属性，需要通过其他方法（如内核模块或设备树覆盖）。

---

### 2. **使用设备树覆盖（Device Tree Overlay）**

设备树覆盖是一种动态修改设备树的方法。它允许在运行时加载额外的设备树片段，覆盖或添加现有设备树的节点和属性。

#### 步骤：

- **1.编写设备树覆盖文件**

　　创建一个 `.dts` 文件，描述需要修改的节点和属性。例如，修改 `/soc/i2c@12340000` 的 `status` 属性：

```c
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target-path = "/soc/i2c@12340000";
        __overlay__ {
            status = "okay";
        };
    };
};
```

- **2.编译设备树覆盖文件**

　　使用设备树编译器（`dtc`）将 `.dts` 文件编译为 `.dtbo` 文件：

```sh
dtc -I dts -O dtb -o my_overlay.dtbo my_overlay.dts
```

- 3.**加载设备树覆盖**

　　将 `.dtbo` 文件加载到内核中：

```sh
echo my_overlay.dtbo > /sys/kernel/config/device-tree/overlays/0/path
```

- 4.**验证修改**

　　检查设备树属性是否已修改：

```sh
cat /sys/firmware/devicetree/base/soc/i2c@12340000/status
```

- 5.**卸载设备树覆盖**

　　如果需要卸载覆盖，可以执行以下命令：

```sh
rmdir /sys/kernel/config/device-tree/overlays/0
```

 

### **3.通过内核模块修改设备树**

 可以通过编写内核模块，直接修改设备树中的属性。

#### 示例代码：

以下是一个简单的内核模块，用于修改设备树节点的 `status` 属性：

```c
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
static int __init my_module_init(void)
{
    struct device_node *node;
    // 查找设备树节点
    node = of_find_node_by_path("/soc/i2c@12340000");
    if(!node) {
        pr_err("Device tree node not found!\n");
        return -ENODEV;
    }
    
    // 修改 status 属性
    of_property_write_string(node, "status", "okay");
    pr_info("Device tree node status updated to 'okay'\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    pr_info("Module unloaded\n");
}
module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Dynamic Device Tree Property Modification");
```

#### 编译和加载模块：

- 1.编写 Makefile：

```sh
obj-m += my_module.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

- 2.编译模块：

```sh
make
```

- 3.加载模块：

```sh
insmod my_module.ko
```

- 4.验证修改：

```sh
cat /sys/firmware/devicetree/base/soc/i2c@12340000/status
```

- 5.卸载模块：

```sh
rmmod my_module
```

### 4. **通过 U-Boot 修改设备树**

在启动阶段，可以通过 U-Boot 修改设备树属性，然后将修改后的设备树传递给内核。具体方法请参考 U-Boot 的相关文档。

------

### 5. **注意事项**

- - **权限问题**：修改设备树属性可能需要 root 权限。
  - **内核支持**：确保内核支持设备树覆盖和动态修改功能。
  - **硬件影响**：修改设备树属性可能会影响硬件行为，请谨慎操作。