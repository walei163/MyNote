# 基于FPGA从机的QSPI通信的实现及应用

用 FPGA 作为 QSPI 通信从机时，涉及到的一些要点，简单记录一下。

## 一、QSPI 的通信机制

---

QSPI 通信的具体原理不再详述，可以参考博文：

![基于STM32主机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200910_01_qspi_function_block-480x300.png)

### [基于STM32主机的QSPI通信的实现及应用](https://www.sunev.cn/embedded/705.html)

仅贴出来两张 QSPI 4 线数据通信时的时序。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_01_qspi_read.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_01_qspi_read.png)

图 1 QSPI 读时序

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_02_qspi_write.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_02_qspi_write.png)

图 2 QSPI 写时序

根据 QSPI 的读写时序可以看出，读写一帧 QSPI 数据，由命令+地址+空指令（读）+数据组成，这里可以结合 MCU 自定义各个部分的位宽，例如设计 6 位命令+10 位地址+8 位空指令（读）+32 位数据的形式。下图是 FPGA 处理 QSPI 的框图。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_03_qspi_salve_mode_struture.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_03_qspi_salve_mode_struture.png)

图 3 基于 FPGA 的 QSPI 通信从机的设计

处理流程大致如下：FPGA 接收到 QSPI 数据帧后解析出命令、地址和数据，即转成并口通信，然后再根据自定义的命令和地址读写相应的控制寄存器和状态寄存器，以及数据缓存 FIFO。

## 二、仿真分析

从上面的 QSPI 读写波形可以看出，一帧完整的 QSPI 读时序由指令+地址+空指令周期+数据组成，而一帧完整的 QSPI 写时序由指令+地址+数据组成。

结合 QSPI 的读写时序和 FPGA 端处理流程，分别写出读写时序的状态机以及 Test Bench 文件。下面是 FPGA 作为从机实现的 QSPI 读写仿真波形。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_04_qspi_simulation.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_04_qspi_simulation.png)

图 4 FPGA 从机 QSPI 读写操作

仿真波形分别完成了一次主机写操作和读操作。主机 QSPI 写操作的详细仿真波形：

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_05_qspi_fpga_receive.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_05_qspi_fpga_receive.png)

图 5 FPGA 从机接收 QSPI 数据

写操作是主机向地址 10’h130 写数据 32’haabbccdd，可以看出在写操作结束，状态机能够正确的发出写命令 master_wr 高电平、地址以及数据。

<iframe id="aswift_4" name="aswift_4" style="left: 0px; top: 0px; border: 0px; width: 820px; height: 0px;" sandbox="allow-forms allow-popups allow-popups-to-escape-sandbox allow-same-origin allow-scripts allow-top-navigation-by-user-activation" width="820" height="0" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" scrolling="no" src="https://googleads.g.doubleclick.net/pagead/ads?client=ca-pub-1538751101685194&amp;output=html&amp;h=280&amp;adk=186289618&amp;adf=2162686003&amp;w=820&amp;abgtt=6&amp;fwrn=4&amp;fwrnh=100&amp;lmt=1719804077&amp;num_ads=1&amp;rafmt=1&amp;armr=3&amp;sem=mc&amp;pwprc=2743427909&amp;ad_type=text_image&amp;format=820x280&amp;url=https%3A%2F%2Fwww.sunev.cn%2Fembedded%2F700.html&amp;fwr=0&amp;pra=3&amp;rh=200&amp;rw=820&amp;rpe=1&amp;resp_fmts=3&amp;wgl=1&amp;fa=27&amp;dt=1719804077643&amp;bpp=1&amp;bdt=4519&amp;idt=-M&amp;shv=r20240625&amp;mjsv=m202406250101&amp;ptt=9&amp;saldr=aa&amp;abxe=1&amp;cookie=ID%3Debd5679fc6bd9edd%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_MbTMIa9kmVgJwjXOjLH9JCYfYkNmg&amp;gpic=UID%3D00000e6d5f78c6a6%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_Ma4zyUL_itqbfJhyMInIHV2e3Y2gA&amp;eo_id_str=ID%3D74e57ca6521a2cfc%3AT%3D1719804077%3ART%3D1719804077%3AS%3DAA-AfjaWG3Zb9PJvDrd_qTboFZuo&amp;prev_fmts=0x0%2C820x200%2C300x600%2C820x280&amp;nras=3&amp;correlator=6217691678519&amp;frm=20&amp;pv=1&amp;ga_vid=1775381088.1719804077&amp;ga_sid=1719804077&amp;ga_hid=1002184768&amp;ga_fc=0&amp;u_tz=480&amp;u_his=1&amp;u_h=1080&amp;u_w=1920&amp;u_ah=1032&amp;u_aw=1920&amp;u_cd=24&amp;u_sd=1&amp;adx=390&amp;ady=2497&amp;biw=1920&amp;bih=890&amp;scr_x=0&amp;scr_y=378&amp;eid=44759842%2C31084864%2C44798934%2C95334511%2C95334526%2C95334572%2C95334580%2C95334830%2C95335897%2C95335246%2C95335292&amp;oid=2&amp;pvsid=1488729488837113&amp;tmod=1672601438&amp;uas=0&amp;nvt=1&amp;ref=https%3A%2F%2Fwww.baidu.com%2Flink%3Furl%3DP1Yp3gWIKEwjIs3zJawd14aU-YiPAfBpSJTC3MkNTp0edjuPAjmMGB4C0NtosxUC%26wd%3D%26eqid%3Dc0b1aea00373955a000000066682185f&amp;fc=1408&amp;brdim=-8%2C-8%2C-8%2C-8%2C1920%2C0%2C1936%2C1048%2C1920%2C890&amp;vis=1&amp;rsz=%7C%7Cs%7C&amp;abl=NS&amp;fu=128&amp;bc=31&amp;bz=1.01&amp;ifi=5&amp;uci=a!5&amp;btvi=3&amp;fsb=1&amp;dtd=18" data-google-container-id="a!5" tabindex="0" title="Advertisement" aria-label="Advertisement" data-load-complete="true" data-google-query-id="CMDCkLnxhIcDFf_MTAIdYYQExg"></iframe>

主机 QSPI 读操作的详细仿真波形：

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_06_qspi_fpga_send.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_06_qspi_fpga_send.png)

图 6 FPGA 从机发送 QSPI 数据

读操作是主机读取地址 10’h141 的值，读操作相较于写操作而言，地址和数据之间多了 8 个周期的空指令，待 FPGA 准备数据。在读操作结束，主机能够正确的读取到 10’h141 地址的数据 32’hccddeeff。

## 三、设计要点

### 3.1 QSPI 空指令周期

在 QSPI 总线读操作时，如果 FPGA 给出的数据足够快，一般小于半个 QSPI 的时钟周期，那么不需要空指令周期；反之，如果不能在半个 QSPI 的时钟周期内准备好数据，则需要根据实际情况，增加 QSPI 总线的空指令周期。

### 3.2 读写使能周期

QSPI 总线转成并行总线时，读写使能信号应满足只有一个系统时钟周期的有效电平，否则会出现数据连续读写的情况。

<iframe id="aswift_5" name="aswift_5" style="left: 0px; top: 0px; border: 0px; width: 820px; height: 0px;" sandbox="allow-forms allow-popups allow-popups-to-escape-sandbox allow-same-origin allow-scripts allow-top-navigation-by-user-activation" width="820" height="0" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" scrolling="no" src="https://googleads.g.doubleclick.net/pagead/ads?client=ca-pub-1538751101685194&amp;output=html&amp;h=280&amp;adk=186289618&amp;adf=94843132&amp;w=820&amp;abgtt=6&amp;fwrn=4&amp;fwrnh=100&amp;lmt=1719804077&amp;num_ads=1&amp;rafmt=1&amp;armr=3&amp;sem=mc&amp;pwprc=2743427909&amp;ad_type=text_image&amp;format=820x280&amp;url=https%3A%2F%2Fwww.sunev.cn%2Fembedded%2F700.html&amp;fwr=0&amp;pra=3&amp;rh=200&amp;rw=820&amp;rpe=1&amp;resp_fmts=3&amp;wgl=1&amp;fa=27&amp;dt=1719804077643&amp;bpp=1&amp;bdt=4518&amp;idt=1&amp;shv=r20240625&amp;mjsv=m202406250101&amp;ptt=9&amp;saldr=aa&amp;abxe=1&amp;cookie=ID%3Debd5679fc6bd9edd%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_MbTMIa9kmVgJwjXOjLH9JCYfYkNmg&amp;gpic=UID%3D00000e6d5f78c6a6%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_Ma4zyUL_itqbfJhyMInIHV2e3Y2gA&amp;eo_id_str=ID%3D74e57ca6521a2cfc%3AT%3D1719804077%3ART%3D1719804077%3AS%3DAA-AfjaWG3Zb9PJvDrd_qTboFZuo&amp;prev_fmts=0x0%2C820x200%2C300x600%2C820x280%2C820x280&amp;nras=4&amp;correlator=6217691678519&amp;frm=20&amp;pv=1&amp;ga_vid=1775381088.1719804077&amp;ga_sid=1719804077&amp;ga_hid=1002184768&amp;ga_fc=0&amp;u_tz=480&amp;u_his=1&amp;u_h=1080&amp;u_w=1920&amp;u_ah=1032&amp;u_aw=1920&amp;u_cd=24&amp;u_sd=1&amp;adx=390&amp;ady=3320&amp;biw=1920&amp;bih=890&amp;scr_x=0&amp;scr_y=378&amp;eid=44759842%2C31084864%2C44798934%2C95334511%2C95334526%2C95334572%2C95334580%2C95334830%2C95335897%2C95335246%2C95335292&amp;oid=2&amp;pvsid=1488729488837113&amp;tmod=1672601438&amp;uas=0&amp;nvt=1&amp;ref=https%3A%2F%2Fwww.baidu.com%2Flink%3Furl%3DP1Yp3gWIKEwjIs3zJawd14aU-YiPAfBpSJTC3MkNTp0edjuPAjmMGB4C0NtosxUC%26wd%3D%26eqid%3Dc0b1aea00373955a000000066682185f&amp;fc=1408&amp;brdim=-8%2C-8%2C-8%2C-8%2C1920%2C0%2C1936%2C1048%2C1920%2C890&amp;vis=1&amp;rsz=%7C%7Cs%7C&amp;abl=NS&amp;fu=128&amp;bc=31&amp;bz=1.01&amp;ifi=6&amp;uci=a!6&amp;btvi=4&amp;fsb=1&amp;dtd=23" data-google-container-id="a!6" tabindex="0" title="Advertisement" aria-label="Advertisement" data-load-complete="true" data-google-query-id="CJTmkLnxhIcDFVILewcdt9IPfA"></iframe>

## 四、调试记录

在调试过程中，曾遇到一些问题，尤其是经常会出现电平异常的情况，如图 7 所示。这样的情况均是 FPGA 处理 QSPI 的状态机出现异常导致。结合 QSPI 读写时序和仿真波形可以解决。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_07_qspi_fpga_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_07_qspi_fpga_wave.png)

图 7 QSPI 波形电平异常

最后遇到一个蜜汁困惑的问题：先烧写 MCU 程序，再烧写 FPGA 程序，数据输出正确；反之，错误，但是如果将示波器表笔加在 qspi_clk 上，则输出又正确。正确的 QSPI 传输波形如图 8 图 9 所示，只测了 QSPI_CLK 和 QSPI_IO0。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_08_qspi_fpga_send_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_08_qspi_fpga_send_wave.png)

图 8 正确的 QSPI 读数据波形

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_09_qspi_fpga_receive_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_09_qspi_fpga_receive_wave.png)

图 9 正确的 QSPI 写波形

对于出现问题的情况，先抓取第一个波形，指令段有部分异常波形，数据段成高阻态。如图 10 图 11 所示。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_10_qspi_fpga_1st_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_10_qspi_fpga_1st_wave.png)

图 10 异常的 QSPI 波形（第一次操作）

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_11_qspi_fpga_2nd_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_11_qspi_fpga_2nd_wave.png)

图 11 异常的 QSPI 波形（后续操作）

后续的波形，指令和地址阶段都正确，数据阶段不正确，有点像高阻态。猜测由于某种原因导致 FPGA 的 qspi 接口未转换成输出接口，而 MCU 的 qspi 接口转换成了输入接口。有一种办法可以验证，即 MCU 发送 qspi 读命令之前增加一个 FPGA 复位命令，看数据输出是否正常。通过实测，数据仍是错误。

<iframe id="aswift_6" name="aswift_6" style="left: 0px; top: 0px; border: 0px; width: 820px; height: 0px;" sandbox="allow-forms allow-popups allow-popups-to-escape-sandbox allow-same-origin allow-scripts allow-top-navigation-by-user-activation" width="820" height="0" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" scrolling="no" src="https://googleads.g.doubleclick.net/pagead/ads?client=ca-pub-1538751101685194&amp;output=html&amp;h=280&amp;adk=186289618&amp;adf=1488068516&amp;w=820&amp;abgtt=6&amp;fwrn=4&amp;fwrnh=100&amp;lmt=1719804077&amp;num_ads=1&amp;rafmt=1&amp;armr=3&amp;sem=mc&amp;pwprc=2743427909&amp;ad_type=text_image&amp;format=820x280&amp;url=https%3A%2F%2Fwww.sunev.cn%2Fembedded%2F700.html&amp;fwr=0&amp;pra=3&amp;rh=200&amp;rw=820&amp;rpe=1&amp;resp_fmts=3&amp;wgl=1&amp;fa=27&amp;dt=1719804077647&amp;bpp=1&amp;bdt=4522&amp;idt=0&amp;shv=r20240625&amp;mjsv=m202406250101&amp;ptt=9&amp;saldr=aa&amp;abxe=1&amp;cookie=ID%3Debd5679fc6bd9edd%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_MbTMIa9kmVgJwjXOjLH9JCYfYkNmg&amp;gpic=UID%3D00000e6d5f78c6a6%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_Ma4zyUL_itqbfJhyMInIHV2e3Y2gA&amp;eo_id_str=ID%3D74e57ca6521a2cfc%3AT%3D1719804077%3ART%3D1719804077%3AS%3DAA-AfjaWG3Zb9PJvDrd_qTboFZuo&amp;prev_fmts=0x0%2C820x200%2C300x600%2C820x280%2C820x280%2C820x280%2C300x600&amp;nras=6&amp;correlator=6217691678519&amp;frm=20&amp;pv=1&amp;ga_vid=1775381088.1719804077&amp;ga_sid=1719804077&amp;ga_hid=1002184768&amp;ga_fc=0&amp;u_tz=480&amp;u_his=1&amp;u_h=1080&amp;u_w=1920&amp;u_ah=1032&amp;u_aw=1920&amp;u_cd=24&amp;u_sd=1&amp;adx=390&amp;ady=3865&amp;biw=1920&amp;bih=890&amp;scr_x=0&amp;scr_y=378&amp;eid=44759842%2C31084864%2C44798934%2C95334511%2C95334526%2C95334572%2C95334580%2C95334830%2C95335897%2C95335246%2C95335292&amp;oid=2&amp;pvsid=1488729488837113&amp;tmod=1672601438&amp;uas=0&amp;nvt=1&amp;ref=https%3A%2F%2Fwww.baidu.com%2Flink%3Furl%3DP1Yp3gWIKEwjIs3zJawd14aU-YiPAfBpSJTC3MkNTp0edjuPAjmMGB4C0NtosxUC%26wd%3D%26eqid%3Dc0b1aea00373955a000000066682185f&amp;fc=1408&amp;brdim=-8%2C-8%2C-8%2C-8%2C1920%2C0%2C1936%2C1048%2C1920%2C890&amp;vis=1&amp;rsz=%7C%7Cs%7C&amp;abl=NS&amp;fu=128&amp;bc=31&amp;bz=1.01&amp;ifi=7&amp;uci=a!7&amp;btvi=6&amp;fsb=1&amp;dtd=344" data-google-container-id="a!7" tabindex="0" title="Advertisement" aria-label="Advertisement" data-load-complete="true" data-google-query-id="CNfcpLnxhIcDFVjPTAIdOVIPbA"></iframe>

FPGA 不下载程序，只下载 MCU 的程序，观察波形，发现输出正常；FPGA 下载程序的情况下，波形如上段文字分析，说明 FPGA 的时序影响到了 MCU 发出的波形。

进一步排查 FPGA 收发 qspi 的代码，屏蔽 FPGA 发送端的代码，FPGA 接收数据正常，波形正常。再进一步定位到 qspi_rd => 1；出现的问题，同时将该信号输出到管脚，发现下载 MCU 程序的时候出现了高电平。由于表笔夹在 qspi_clk 管脚会改善结果，所以只测了 qspi_rd 波形，如图 12 所示。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_12_qspi_rd_wave.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_12_qspi_rd_wave.png)

图 12 MCU 下载程序期间 qspi_rd 被拉高

然后，在 SignalTap 中同时观察 qspi_clk 和 qspi_rd 波形，当 MCU 下载程序的时候，qspi_clk 会出现高低电平跳变，这样 FPGA 状态机就会乱掉，进一步导致 qspi_rd 为高。如图 13 所示。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_13_qspi_rd_SignalTap.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_13_qspi_rd_SignalTap.png)

图 13 SignalTap 采集波形，MCU 下载程序期间 qspi_rd 被拉高

根本原因：qspi_clk 引脚未加下拉电阻造成 MCU 下载程序时波形不稳定，这也解释了为什么 qspi_clk 放置示波器表笔，数据传输正常。

<iframe id="aswift_7" name="aswift_7" style="left: 0px; top: 0px; border: 0px; width: 820px; height: 0px;" sandbox="allow-forms allow-popups allow-popups-to-escape-sandbox allow-same-origin allow-scripts allow-top-navigation-by-user-activation" width="820" height="0" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" scrolling="no" src="https://googleads.g.doubleclick.net/pagead/ads?client=ca-pub-1538751101685194&amp;output=html&amp;h=280&amp;adk=186289618&amp;adf=2769895359&amp;w=820&amp;abgtt=6&amp;fwrn=4&amp;fwrnh=100&amp;lmt=1719804132&amp;num_ads=1&amp;rafmt=1&amp;armr=3&amp;sem=mc&amp;pwprc=2743427909&amp;ad_type=text_image&amp;format=820x280&amp;url=https%3A%2F%2Fwww.sunev.cn%2Fembedded%2F700.html&amp;fwr=0&amp;pra=3&amp;rh=200&amp;rw=820&amp;rpe=1&amp;resp_fmts=3&amp;wgl=1&amp;fa=27&amp;dt=1719804077650&amp;bpp=1&amp;bdt=4526&amp;idt=0&amp;shv=r20240625&amp;mjsv=m202406250101&amp;ptt=9&amp;saldr=aa&amp;abxe=1&amp;cookie=ID%3Debd5679fc6bd9edd%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_MbTMIa9kmVgJwjXOjLH9JCYfYkNmg&amp;gpic=UID%3D00000e6d5f78c6a6%3AT%3D1719804077%3ART%3D1719804077%3AS%3DALNI_Ma4zyUL_itqbfJhyMInIHV2e3Y2gA&amp;eo_id_str=ID%3D74e57ca6521a2cfc%3AT%3D1719804077%3ART%3D1719804077%3AS%3DAA-AfjaWG3Zb9PJvDrd_qTboFZuo&amp;prev_fmts=0x0%2C820x200%2C300x600%2C820x280%2C820x280%2C820x280%2C300x600%2C820x280%2C1920x890%2C1005x124&amp;nras=9&amp;correlator=6217691678519&amp;frm=20&amp;pv=1&amp;ga_vid=1775381088.1719804077&amp;ga_sid=1719804077&amp;ga_hid=1002184768&amp;ga_fc=0&amp;u_tz=480&amp;u_his=1&amp;u_h=1080&amp;u_w=1920&amp;u_ah=1032&amp;u_aw=1920&amp;u_cd=24&amp;u_sd=1&amp;adx=390&amp;ady=5286&amp;biw=1920&amp;bih=890&amp;scr_x=0&amp;scr_y=1764&amp;eid=44759842%2C31084864%2C44798934%2C95334511%2C95334526%2C95334572%2C95334580%2C95334830%2C95335897%2C95335246%2C95335292&amp;oid=2&amp;psts=AOrYGsn7HoHc_tA3x1mHFk3HO3KfDk-qwTX0tBrcbaqF-YiMgHH2vvTbQ9abp0rf32NvcNRkkVGNvWjuJZnyuIjM5bU0XQ%2CAOrYGsnfHq7zAxDMISsz-4IyTjYymuN-yPg29P137tDu_Y7GTbuIDyJjwpQAdqL5bUI7TFmXg9HYdE5oal52kF06n5i6rlcm-YEFZp1JwCYN1nrW598&amp;pvsid=1488729488837113&amp;tmod=1672601438&amp;uas=0&amp;nvt=1&amp;ref=https%3A%2F%2Fwww.baidu.com%2Flink%3Furl%3DP1Yp3gWIKEwjIs3zJawd14aU-YiPAfBpSJTC3MkNTp0edjuPAjmMGB4C0NtosxUC%26wd%3D%26eqid%3Dc0b1aea00373955a000000066682185f&amp;fc=1408&amp;brdim=-8%2C-8%2C-8%2C-8%2C1920%2C0%2C1936%2C1048%2C1920%2C890&amp;vis=1&amp;rsz=%7C%7Cs%7C&amp;abl=NS&amp;fu=128&amp;bc=31&amp;bz=1.01&amp;ifi=8&amp;uci=a!8&amp;btvi=8&amp;fsb=1&amp;dtd=55231" data-google-container-id="a!8" tabindex="0" title="Advertisement" aria-label="Advertisement" data-google-query-id="CNqSu9PxhIcDFQcYewcdx6wIvg" data-load-complete="true"></iframe>

解决办法：1）qspi_clk 加下拉电阻，qspi_cs 加上拉电阻（虽然 SignalTap 采集到的数据是稳定的高电平；另外，主机采用的是 STM32，QSPI 部分参考的官方原理图）；2）FPGA 收发 qspi 数据的状态机引入 qspi_cs 低电平判断来运行状态机，最初设计的状态机是直接通过 qspi_clk 的跳变沿实现接收数据和发送数据，如果 qspi_clk 有扰动，整个状态机就会乱掉，所以加上 qspi_cs 低电平判断 qspi 数据传输的开始。

附件仅博主测试所用，暂不提供下载

## 20210315 更新

在 MCU 连续多次操作 QSPI 写命令时，发现 FPGA 收到的命令会有遗漏。通过 SignalTap 连续抓取 FPGA 波形，MCU 单步调试，发现确实会有命令遗漏的情况，如下图所示。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_14_SignalTap.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_14_SignalTap.png)

图 14 SignalTap 抓取波形，地址 0x165 命令丢失

图中，遗漏了向地址 0x165 写数据的命令，这样就是 FPGA 接收 QSPI 命令的时序出现了问题。

由于设计的 QSPI 接口时钟频率大概几十 MB（MCU 为 QSPI 主机），考虑到后续处理数据可以采用更高的频率，如 100Mhz，所以 QSPI 命令转并口时，采用了系统时钟频率，只不过这里的处理有些草率，导致出现亚稳态的情况。原始的处理代码：

```verilog
reg [2:0] state;
always@(posedge sys_clk or negedge rst_n)
  if(!rst_n)
    begin
      state <= 3'd0;
      qspi_frame_cmd_in <= 16'h0;
      qspi_frame_cmd_in_valid_1 <= 0;
      qspi_frame_cmd_in_valid_2 <= 0;
    end
  else case(state)
    3'd0:
    begin
      state <= state + 1;
//      qspi_frame_cmd_in <= 16'h0;
      qspi_frame_cmd_in_valid_1 <= 0;
      qspi_frame_cmd_in_valid_2 <= 0;
    end
    3'd1:
    begin
      if(cnt == 3'd1)
        begin
          state <= state + 1;
          qspi_frame_cmd_in[15:8] <= data_in;
          qspi_frame_cmd_in_valid_1 <= 0;
          qspi_frame_cmd_in_valid_2 <= 0;          
        end
      else
        state <= 3'd1;
    end
    3'd2:
    begin
      if(cnt == 3'd2)
        begin
          state <= state + 1;
          qspi_frame_cmd_in[7:0] <= data_in;
          qspi_frame_cmd_in_valid_1 <= 1;
          qspi_frame_cmd_in_valid_2 <= 0;
        end
      else
        state <= 3'd2;
    end
    3'd3:
    begin
        state <= state + 1;
        qspi_frame_cmd_in <= qspi_frame_cmd_in;
        qspi_frame_cmd_in_valid_1 <= 0;
        qspi_frame_cmd_in_valid_2 <= 0;        
    end      
    3'd4:
    begin
      if(cnt == 3'd3)
        begin
          state <= 0;
          qspi_frame_cmd_in <= qspi_frame_cmd_in;
          qspi_frame_cmd_in_valid_1 <= 0;
          qspi_frame_cmd_in_valid_2 <= 1;
        end
      else
        state <= 3'd4;
    end
    default:
      state <= 0;
  endcase
```

在系统时钟下，通过判断 cnt 是否满足条件来使一帧 QSPI 数据中的命令和数据标志位置 1，而 cnt 是由采样 QSPI 数据的状态机来改变的，所以导致命令和数据标志置 1 会出现亚稳态。

修改为：将命令和数据标志置 1 放在采样 QSPI 数据的状态机下，再通过跨时钟域单比特信号处理，将低时钟频率的信号转换成高时钟频率的信号，也就是将 QSPI 时钟频率下的命令和数据转换成系统时钟频率下的命令和数据。仿真波形如下：

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_15_Simulation.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_15_Simulation.png)

图 15 优化后的代码仿真波形

qspi_frame_cmd_in_valid_1_pos 和 qspi_frame_cmd_in_valid_1_pos 分别是跨时钟域处理后的信号，进一步转换并口的读写信号。

[![基于FPGA从机的QSPI通信的实现及应用](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_16_SignalTap.png)](https://www.sunev.cn/blog/wp-content/uploads/2020/09/20200903_16_SignalTap.png)

图 16 优化后的 SignalTap 抓取波形

再通过 SignalTap 抓取波形，经多次测试，并未发现命令遗漏的情况了。