# CAN学习之--不使用收发器进行通讯测试

# CAN不使用外置PHY芯片简单组网直接通信的方法

当排查问题时，可以使用如下方法跳过PHY芯片，使用单片机的RXD、TXD组网直接通信。
![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/硬件设计相关/images/df9a88f9927a4adca967efa3c42e0fbc.png)
仅需要二极管和上拉电阻。

---

在实际调试或者学习CAN通讯过程中，在需要进行CAN调试的时候，但是手头有只有MCU的评估板，没有CAN的收发器，比如ATA6561、MCP2518这类芯片的时候，该怎么办呢？
因为我们知道，CAN收发器只是做信号的转换，我们的MCU作为CAN 的MAC，才是真实处理数据的，要是我们能直接使用MCU的CAN MAC进行相互通讯，那就很方便了，同时也有利于理解CAN的物理层级。

---

## CAN 控制器的接口：

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/硬件设计相关/images/b6049cab23764c19bc67c2735f87d600.png)

观察PIC18MCU的ECAN模块内部框图，可以看到对外实际就是TX和RX，看起来和串口一样。

但实际CAN是差分的总线，所以我们需要吧TX和RX合并起来，变成单总线。

---

PIC18F45K80的CAN TX是RB2，RX是RB3

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/硬件设计相关/images/6c7bf23198f9473a8cb9008a2eac9a59.png)

我们对外部电路做一些改动：

![在这里插入图片描述](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/硬件设计相关/images/b7311c47fcb34642ad5af7ffdd8f874e.png)

这里的二极管最好选择正向压降低一点的型号。
不难看出电路原理，CAN_RX对外连接到"CAN"总线上面就可以通讯了。或者CAN_TX要是能够开漏输出，也可以直接把TX和RX进行短接。

经过测试，不同型号的MCU之间通过搭建这样的电路，正确的进行了CAN通讯。在一些时候还是很方便的。￼
仅需要二极管和上拉电阻。
————————————————
版权声明：本文为CSDN博主「havc_W」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/havc_W/article/details/147758184



---

# CAN 为什么需要收发器

在RTL代码中集成了两个CAN node，打算直接连接将两个node的Rx和Tx对接，

发现两个CAN Node无法通信，询问技术支持后才知道必须要收发器。

那为什么一定需要收发器呢？

除了转换单端的CAN信号用于不同的传输，收发器也会将CANTX loop back 回 CAN RX,

因为CAN node 需要监视它自己的传输：

①CAN协议规定的ACK要求，当一个节点在CAN总线上发送一个frame帧时，它希望网络上的其它节点至少给一个ACK。

对于ACK响应阶段，发送器输出一个1并希望读回一个0。

②在仲裁阶段，一个带有高优先级的节点需要能够用0覆盖1，这里 发射机需要再次都会发送的数据，当节点发送1并读回0时，

则失去仲裁。

参考来自 TI MCAN

![img](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/硬件设计相关/images/5ff5bcc30ecbd34f7ccd3e013a3ab807.png)