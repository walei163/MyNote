# Paho-MQTT C(嵌入式版)接入示例

1. 一般在嵌入式Linux下，需要采用`Mosquitto`来实现MQTT的服务器端和客户端。采用`paho`可以实现MQTT客户端。
不过现在实际的MQTT开发方式可以按照如下步骤来实现：

* 交叉编译`Mosquitto`的动态库实现MQTT服务器端。客户端一般不采用`Mosquitto`的库。
* 交叉编译`paho`的动态库实现MQTT客户端。
* 参照paho提供的例程，实现自己的MQTT的客户端代码。

2. 以下是我自己在开发过程中整理的开发文档。可以参照这些文档中的描述来实现：

* Mosquitto安装与使用（<mark>后续可以将其作为服务器端使用</mark>）：
[Mosquitto安装与使用](https://walei163.notion.site/Mosquitto-160133c208da45bf9f1a09282f915b6b?pvs=4)    	

 * 关于paho开发客户端的说明与实际示例（<mark>重要，后续开发需要采用paho的库来实现MQTT客户端应用</mark>）：
 [温湿度监控系统——学习paho mqtt的基本操作](https://walei163.notion.site/paho-mqtt-3eaf3516827b45cc9d41c6bc9083e83e?pvs=4)

* 采用Mosquito开发客户端的例程（<mark>这个可以不需要，因为一般采用paho来开发客户端应用</mark>）：
[采用Mosquito开发客户端的例程](https://walei163.notion.site/Mosquito-fad7c576a6084cefa1ea2f3441e0c710?pvs=4)

3.  压缩包内容：

* Mosquitto的源代码。包括已经编译好的基于armhf的动态库（32位，64位需要重新编译），和Windows下的安装包。
* paho的源代码。包括已经编译好的基于armhf的动态库。
* 我写的一个简单的paho客户端的测试程序示例。
* MQTT客户端可视化测试工具。Windows下安装，可以提供一个图形化的测试MQTT的客户端工具界面，需要先安装JAVA运行时库。

4. 使用方法：

* 在PC机上安装Mosquitto服务器端程序，可以安装Windows下的安装包。Linux下需要自己编译Mosquitto源码，可以编译成x86平台的，也可以交叉编译成ARM平台的。运行服务器程序时，需要配置好`mosquitto.conf`中的配置。

* 在PC机上安装MQTT客户端可视化工具，可以安装Windows下的安装包（`org.eclipse.paho.ui.app-1.0.2-win32.win32.x86_64.zip`）。也可以安装Linux发行版下的安装包（`org.eclipse.paho.ui.app-1.0.2-linux.gtk.x86_64.tar.gz`）。

> [!note]
>
> 注意：
> 1）Window下的安装包在安装之前，需要先安装JAVA运行时库。
> 2）客户端可以和服务器端安装到同一台PC机上，也可以安装到不同的PC机。注意PC机的IP地址，需要在同一个局域网内。
> 3）安装客户端只是为了测试方便，以便于验证我们自己写的客户端程序是否能够正确发布和订阅信息，接收MQTT协议报文，后续正式量产不需要这个环节。

* 将编译好的基于ARM系统的paho客户端动态库复制到嵌入式系统中，同时加上我们自己编写的基于paho的客户端程序也复制到嵌入式系统中，如果设置好了发布/订阅的内容，就可以正确的运行了。
