# 嵌入式linux 打开网络TSN功能及测试

# 嵌入式 Linux 网络 TSN 功能配置与测试指南

目录

---

[TOC]



---

## 1. TSN 功能概述

**TSN（Time-Sensitive Networking）** 是 IEEE 802.1 标准定义的一组协议，旨在为以太网提供确定性低延迟和高可靠性的数据传输能力，主要应用于工业自动化、汽车网络和音视频传输等领域。核心功能包括：

- **时间同步（IEEE 802.1AS）**：全网设备纳秒级时钟同步。
- **流量调度（IEEE 802.1Qbv）**：时间感知调度器（TAPRIO）实现时间分片。
- **帧抢占（IEEE 802.1Qbu）**：高优先级流量中断低优先级帧传输。
- **流过滤与监管（IEEE 802.1Qci）**：过滤非法流量并限制带宽。

## 2. 硬件与软件要求

### 硬件要求

- 嵌入式平台：需支持 TSN 的 SoC 或外设，例如：
  - NXP LS1028A（集成 TSN 交换机）
  - TI AM64x（支持 PRU-ICSS 工业通信）
  - Intel i210-T1 网卡
- 网络拓扑：支持 TSN 的交换机或直连设备。

### 软件要求

- Linux 内核：**≥ 5.10（完整 TSN 支持需打实时补丁如 PREEMPT_RT）**。
- 工具链：
  - iproute2（配置网络策略）
  - linuxptp（PTP 时间同步）
  - ethtool（帧抢占控制）

## 3. 内核配置与编译

#### 关键内核配置选项

```sh
# 在内核源码目录执行
make menuconfig
```

#### 启用以下选项：

```sh
# Networking support → Networking options
[*] IEEE 802.1Q VLAN Support
[*] IEEE 802.1 Audio Video Bridging (AVB)
[*] Time Sensitive Networking (TSN) support
[*] IEEE 802.1Qbv Time Aware Scheduler (TAPRIO)
[*] IEEE 802.1Qbu Frame Preemption
[*] IEEE 802.1Qci Per-Stream Filtering and Policing
[*] IEEE 802.1AS Time Synchronization
```

#### 内核编译

```sh
make -j$(nproc) 
```

## 4. 网络 TSN 功能配置

### 4.1. 时间同步（IEEE 802.1AS）

**启动 PTP 主时钟**

```sh
ptp4l -i eth0 -m -2 -s -S  # 主节点（-S 表示使用软件时间戳）
```

**同步系统时钟**：

```sh
phc2sys -s eth0 -c CLOCK_REALTIME -m -O 0  # -O 0 表示偏移校准为0
```

### 4.2 流量调度（TAPRIO/Qbv）

配置时间感知调度器：

```sh
tc qdisc replace dev eth0 parent root taprio \
    num_tc 3 \
    map 0 1 2 0 0 0 0 0 \          # 优先级到流量类别映射
    queues 1@0 1@1 1@2 \           # 每个流量类的队列数量
    base-time 0 \                  # 基准时间（从0开始）
    sched-entry S 01 300000 \      # 时间槽1：允许优先级0流量，持续300ms
    sched-entry S 02 200000 \      # 时间槽2：允许优先级1流量，持续200ms
    clockid CLOCK_TAI              # 使用TAI时钟源
```

### 4.3. 帧抢占（Qbu）

启用帧抢占功能：

```sh
ethtool --set-priv-flags eth0 preemption on
```

## 5. TSN 功能测试方法

### 5.1 时间同步测试

**主从时钟同步误差**：

```sh
pmc -u -b 0 'GET TIME_STATUS_NP' | grep offset
# 预期输出：offset 绝对值 < 1μs
```

### 5.2 流量调度延迟测试

**生成测试流量**：

```sh
# 发送高优先级流量（VLAN PCP=6）
mz eth0 -q -d 100us -a rand -b rand -t ip "proto=0x11,dp=5000,vlan=6,pfc=6"
```

**抓包分析延迟**：

```sh
tshark -i eth0 -f "vlan.pcp == 6" -T fields -e frame.time_delta
# 输出相邻帧时间差，应接近100μs
```

### 5.3 帧抢占验证

**测试方法**：

1. 同时发送高低优先级流量。
2. 使用 `ethtool -S eth0` 查看 `mac.txPreempted` 计数器是否增加。

## 6. 验证与调试

### 关键调试命令

| 功能             | 命令                             |
| ---------------- | -------------------------------- |
| 查看 TAPRIO 配置 | `tc qdisc show dev eth0`         |
| 检查帧抢占状态   | `ethtool --show-priv-flags eth0` |
| 实时内核延迟     | `cyclictest -m -Sp90 -n -t1`     |

### 日志分析

```sh
dmesg | grep -E 'tsn|taprio|802.1Q'  # 查看内核级TSN事件
```

## 7. 应用场景与注意事项

### 典型应用场景

- 工业自动化：PLC 与机械臂的实时控制。
- 车载网络：ADAS 摄像头与ECU的同步数据传输。
- 专业音视频：4K/8K 视频流无卡顿传输。

### 注意事项

1. 硬件兼容性：确认网卡支持 `ethtool --features eth0 | grep preemption`。
2. 实时性优化：部署 PREEMPT_RT 补丁以减少内核抖动。
3. 网络规划：避免非TSN流量与关键流量的带宽竞争。

### 资源推荐

- TSN 工具包：[Linux TSN 项目](https://github.com/AVnu/OpenAvnu)
- 硬件参考设计：NXP LS1028A-RDB

---

通过本文档，开发者可系统性地在嵌入式 Linux 中实现 TSN 功能，并验证其确定性传输能力。实际部署时需结合具体硬件调整参数，并持续监控网络性能。

---

原文链接：https://blog.csdn.net/qq_35223473/article/details/147788581