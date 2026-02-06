# 全志Uboot加载dtbo

在全志（Allwinner）的U-Boot中，Device Tree Blob（DTB）和Device Tree Overlay（DTBO）是两种用于配置硬件的设备树文件。它们通常用于嵌入式系统中，特别是基于ARM架构的系统，如全志芯片组。

### DTB (Device Tree Blob)

**DTB** 是一个二进制文件，包含了设备的配置信息。它是在U-Boot启动过程中加载的，用于告诉操作系统如何配置硬件。例如，它可能包含关于CPU、内存、外设（如GPIO、I2C、SPI、UART等）的配置信息。

### DTBO (Device Tree Overlay)

**DTBO** 是一个可选的、较小的二进制文件，用于在不修改主DTB文件的情况下，动态地添加或修改设备的配置。这对于在不重新编译整个内核的情况下，对特定硬件进行微调非常有用。例如，如果你只需要为一块特定的板卡添加一些额外的GPIO配置或调整某些外设的参数，你可以创建一个DTBO文件来实现这一点。

### 在U-Boot中加载DTBO

在U-Boot中加载DTBO通常涉及到以下几个步骤：

1. **编译DTBO文件**：首先，你需要有一个描述额外硬件配置的.dts文件。然后使用dtc工具（Device Tree Compiler）将其编译成.dtbo文件。

```bash
dtc -I dts -O dtb -o myoverlay.dtbo myoverlay.dts
```

2. **在U-Boot中挂载文件系统**：确保你的U-Boot配置支持从存储设备（如SD卡、eMMC等）启动，并且已经正确挂载了文件系统。

3. **加载DTBO到内存**：你可以使用U-Boot的`load`命令将DTBO文件从存储设备加载到内存中。例如：

```bash
load mmc 0:5 ${fdtaddr} /home/sanway/update_linux.sh
```

这里`${fdtaddr}`是一个环境变量，通常指向一个内存地址，用于存放DTB或DTBO。`mmc 0:5`表示从SD卡的第5个分区加载文件。

4. **应用DTBO**：加载完DTBO到内存后，你可以通过U-Boot的命令行接口使用`fdt addr`和`fdt apply`命令来应用这个overlay。例如：

```bash
fdt addr ${fdtaddr}
fdt apply ${fdtaddr}
```

这些命令会将内存中的DTBO应用到当前的设备树上。

### 注意事项

- 确保你的U-Boot版本支持DTBO。一些较旧的U-Boot版本可能不支持此功能。
- 路径和设备标识符（如`mmc 0:1`）需要根据你的具体硬件配置进行调整。
- 在应用DTBO之前，确保已经正确加载了主DTB文件，因为DTBO是基于当前的DTB来修改的。

通过上述步骤，你可以在全志的U-Boot环境中灵活地管理和应用设备树覆盖（DTBO）文件，以适应不同的硬件需求和配置。