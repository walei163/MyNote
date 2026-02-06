# libmodbus库相关资料

Created: March 15, 2024 2:13 PM
Tags: C语言学习相关
Property: Jason Wang

原文链接：

[8. RS485 — 快速使用手册—基于LubanCat-RK356x系列板卡  文档](https://doc.embedfire.com/linux/rk356x/quick_start/zh/latest/quick_start/rs485/rs485.html)

libmodbus库源代码压缩包：
[libmodbus-3.1.10.tar.gz](file:///media/sf_D_DRIVE/BaiduNetdiskWorkspace/Linux_work/libmodbus库相关资料/libmodbus-3.1.10.tar.gz)

## 8.5. libmodbus简介

libmodbus是一个与使用modbus协议的设备进行数据发送/接收的库，它包含各种后端（backends）通过不同网络进行通信。
（例如，RTU模式下的串口、485总线或TCP / IPv6中的以太网）。
libmodbus还提供了较低通信层的抽象，并在所有支持的平台上提供相同的API。

libmodbus是开源的，它遵循 LGPL v2.1 开源协议，这个协议没有GPL协议那么严格，
简单来说，只要你不修改libmodbus库里面的东西（只调用、链接该库），你是可以闭源你的代码的，也可以用于商业用途，这一点非常好的。

官方代码仓库位于：[https://github.com/stephane/libmodbus](https://github.com/stephane/libmodbus)

提示

> 如果使用驱动层换流只需对官方代码进行小修改，将串口改为实际串口即可，如果使用应用层换流需要配置rts引脚，野火也提供了示例测试，可进行参考。
> 

### 8.5.1. 使用驱动层换流

### 8.5.1.1. 前期准备

根据前面介绍的使能RS485对应的 **RS485设备树插件** 并且正确将RS485-1和RS485-2相连接后，可进行后续步骤。

开发准备，在开发板系统上安装libmodbus-dev和一些编译工具。

---

```bash
sudo apt update
sudo apt install gcc make git autoconf libtool automake
```

---

1、拉取libmodbus源码并修改

---

```bash
git clone https://github.com/stephane/libmodbus.git
```

---

拉取下来后看到本地有libmodbus文件夹，我们进入 libmodbus/tests目录下，该目录下存放了很多测试程序，此处我们使用unit-test-server.c、unit-test-client.c进行测试。

修改为使用使用的接口，以鲁班猫2金手指/btb底板为例：

打开libmodbus/tests/unit-test-server.c，将代码第57行和第76行的/dev/ttyUSB0修改为实际使用的串口，鲁班猫2金手指/btb底板rs485-1对应/dev/ttyS3

---

```bash
#将libmodbus/tests/unit-test-server.c的第57行和第76行定义的接口修改为实际的接口
...
printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyS3\n\n");
...

...
ip_or_device = "/dev/ttyS3";
...
```

---

打开libmodbus/tests/unit-test-client.c，将代码第93行和第112行的/dev/ttyUSB1修改为实际使用的串口，鲁班猫2金手指/btb底板rs485-2对应/dev/ttyS4

---

```bash
#将libmodbus/tests/unit-test-client.c的第93行和第112行定义的接口修改为实际的接口
...
printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyS4\n\n");
...

...
ip_or_device = "/dev/ttyS4";
...
```

---

2、编译程序

进入 **libmodbus最上层目录** 下，运行它提供的脚本，它主要是自动生成一些用于配置的文件。

---

```bash
./autogen.sh
```

---

打印消息如下

---

```bash
configure.ac:33: installing 'build-aux/compile'
configure.ac:56: installing 'build-aux/config.guess'
configure.ac:56: installing 'build-aux/config.sub'
configure.ac:32: installing 'build-aux/install-sh'
configure.ac:32: installing 'build-aux/missing'
src/Makefile.am: installing 'build-aux/depcomp'
parallel-tests: installing 'build-aux/test-driver'

------------------------------------------------------
Initialized build system. You can now run ./configure
------------------------------------------------------
```

---

运行完毕后，接着运行 configure 去配置编译相关的信息。

---

```bash
./configure
```

---

最后打印信息如下

---

```bash
config.status: executing depfiles commands
config.status: executing libtool commands

      libmodbus 3.1.10
      ===============

      prefix:                 /usr/local
      sysconfdir:             ${prefix}/etc
      libdir:                 ${exec_prefix}/lib
      includedir:             ${prefix}/include

      compiler:               gcc
      cflags:                 -g -O2
      ldflags:

      tests:                  yes
```

---

运行上一步之后，在当前目录下将产生 Makefile 文件，使用 make 命令编译即可：

---

```bash
make
```

---

在编译完成后，在 tests 目录下你会发现有很多可执行的文件，比如我们稍后要运行的程序
unit-test-server、unit-test-client：

---

```bash
root@lubancat:~/libmodbus/tests# ls
LICENSE                     bandwidth-server-one    unit-test-client.c
Makefile                    bandwidth-server-one.c  unit-test-client.o
Makefile.am                 bandwidth-server-one.o  unit-test-server
Makefile.in                 random-test-client      unit-test-server.c
README.md                   random-test-client.c    unit-test-server.o
bandwidth-client            random-test-client.o    unit-test.h
bandwidth-client.c          random-test-server      unit-test.h.in
bandwidth-client.o          random-test-server.c    unit-tests.sh
bandwidth-server-many-up    random-test-server.o    version
bandwidth-server-many-up.c  stamp-h2                version.c
bandwidth-server-many-up.o  unit-test-client        version.o
```

---

3、运行程序

打开两个终端，一个用于运行服务端一个用于运行客户端。如下所示

在终端1中执行：

---

```bash
./unit-test-server rtu
```

---

在终端2中执行：

---

```bash
./unit-test-client rtu
```

---

在 client 终端中最终打印

---

```bash
[11][0F][01][60][00][00][00][02][2B][00][01][00][64][59][63]
* try function 0xF: write 0 values: Waiting for a confirmation...
<11><8F><03><05><F4>
OK
[11][0F][01][60][07][B0][06][02][2B][00][01][00][64][12][27]
* try function 0xF: write 1968 values: Waiting for a confirmation...
<11><8F><03><05><F4>
OK
[11][42][00][00][00][00][7B][55]
Waiting for a confirmation...
<11><C2><01><B1><65>
Return an exception on unknown function code: OK

TEST INVALID INITIALIZATION:
The device string is empty
OK
The baud rate value must not be zero
OK

ALL TESTS PASS WITH SUCCESS.
```

---

以上只是测试使用，关于具体用法请读者参考官方说明自行研究。

### 8.5.2. 使用应用层层换流

### 8.5.2.1. 前期准备

根据前面介绍的使能RS485对应的 **串口设备树插件** 并且正确将RS485-1和RS485-2相连接后，可进行后续步骤。

开发准备，在开发板系统上安装libmodbus-dev和一些编译工具。

---

```bash
sudo apt update
sudo apt install gcc git libmodbus-dev pkg-config
```

---

### 8.5.2.2. 程序

以鲁班猫2金手指/btb底板为例，其他底板需要自行修改流控引脚编号以及实际的串口接口。

1.服务端程序：lubancat_rk_code_storage/quick_start/rs485/test-server.c

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_GPIO_INDEX   "90"  //485-1的换流引脚
#define SERVER_ID         17
const uint16_t UT_REGISTERS_TAB[] = { 0x0A, 0x0E, 0x0A, 0x1B,0x0A};

static int _server_ioctl_init(void)
{
   int fd;
   //index config
   fd = open("/sys/class/gpio/export", O_WRONLY);
   if(fd < 0)
         return 1;

   write(fd, SERVER_GPIO_INDEX, strlen(SERVER_GPIO_INDEX));
   close(fd);

   //direction config
   fd = open("/sys/class/gpio/gpio" SERVER_GPIO_INDEX "/direction", O_WRONLY);
   if(fd < 0)
         return 2;

   write(fd, "out", strlen("out"));
   close(fd);

   return 0;
}

static int _server_ioctl_on(void)
{
   int fd;

   fd = open("/sys/class/gpio/gpio" SERVER_GPIO_INDEX "/value", O_WRONLY);
   if(fd < 0)
         return 1;

   write(fd, "1", 1);
   close(fd);
   return 0;
}

static int _server_ioctl_off(void)
{
   int fd;

   fd = open("/sys/class/gpio/gpio" SERVER_GPIO_INDEX "/value", O_WRONLY);
   if(fd < 0)
         return 1;

   write(fd, "0", 1);
   close(fd);
   return 0;
}

static void _modbus_rtu_server_ioctl(modbus_t *ctx, int on)
{
   if (on) {
         _server_ioctl_on();
   } else {
         _server_ioctl_off();
   }
}

int main(int argc, char*argv[])
{
   modbus_t *ctx;
   modbus_mapping_t *mb_mapping;
   int rc;
   int i;
   uint8_t *query;
   /*设置串口信息*/
   ctx = modbus_new_rtu("/dev/ttyS3", 9600, 'N', 8, 1);
   _server_ioctl_init();

   /*设置从机地址，设置485模式*/
   modbus_set_slave(ctx, SERVER_ID);
   modbus_rtu_set_custom_rts(ctx, _modbus_rtu_server_ioctl);
   modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);
   modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

   query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
   /*开启调试*/
   modbus_set_debug(ctx, TRUE);

   mb_mapping = modbus_mapping_new_start_address(0,0,0,0,0,5,0,0);
   if (mb_mapping == NULL) {
         fprintf(stderr, "Failed to allocate the mapping: %s\n",
               modbus_strerror(errno));
         modbus_free(ctx);
         return -1;
   }

   /* 初始化值 */
   for (i=0; i < 5; i++) {
         mb_mapping->tab_registers[i] = UT_REGISTERS_TAB[i];
   }

   rc = modbus_connect(ctx);
   if (rc == -1) {
         fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
         modbus_free(ctx);
         return -1;
   }
   modbus_flush(ctx);

   for (;;) {
         do {
            rc = modbus_receive(ctx, query);
         } while (rc == 0);

         rc = modbus_reply(ctx, query, rc, mb_mapping);
         if (rc == -1) {
            break;
         }
   }

   modbus_mapping_free(mb_mapping);
   free(query);

   modbus_close(ctx);
   modbus_free(ctx);
   return 0;
}
```

2.客户端程序：lubancat_rk_code_storage/quick_start/rs485/test-client.c

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CLIENT_GPIO_INDEX   "91"  //485-2的换流引脚
#define SERVER_ID         17

static int _client_ioctl_init(void)
{
   int fd;
   //index config
   fd = open("/sys/class/gpio/export", O_WRONLY);
   if(fd < 0)
         return 1 ;

   write(fd, CLIENT_GPIO_INDEX, strlen(CLIENT_GPIO_INDEX));
   close(fd);

   //direction config
   fd = open("/sys/class/gpio/gpio" CLIENT_GPIO_INDEX "/direction", O_WRONLY);
   if(fd < 0)
         return 2;

   write(fd, "out", strlen("out"));
   close(fd);

   return 0;
}

static int _client_ioctl_on(void)
{
   int fd;

   fd = open("/sys/class/gpio/gpio" CLIENT_GPIO_INDEX "/value", O_WRONLY);
   if(fd < 0)
         return 1;

   write(fd, "0", 1);
   close(fd);

   return 0;
}

static int _client_ioctl_off(void)
{
   int fd;

   fd = open("/sys/class/gpio/gpio" CLIENT_GPIO_INDEX "/value", O_WRONLY);
   if(fd < 0)
         return 1;

   write(fd, "1", 1);
   close(fd);

   return 0;
}

static void _modbus_rtu_client_ioctl(modbus_t *ctx, int on)
{
   if (on) {
         _client_ioctl_on();
   } else {
         _client_ioctl_off();
   }
}

int main(int argc, char *argv[])
{
   modbus_t *ctx = NULL;
   int i,rc;
   uint16_t tab_rp_registers[5] = {0}; //定义存放数据的数组

   /*创建一个RTU类型的变量*/
   /*设置要打开的串口设备  波特率 奇偶校验 数据位 停止位*/
   ctx = modbus_new_rtu("/dev/ttyS4", 9600, 'N', 8, 1);
   if (ctx == NULL) {
         fprintf(stderr, "Unable to allocate libmodbus context\n");
         return -1;
   }

   /*设置485模式*/
   _client_ioctl_init();
   modbus_rtu_set_custom_rts(ctx, _modbus_rtu_client_ioctl);
   modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_DOWN);
   modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

   /*设置debug模式*/
   modbus_set_debug(ctx, TRUE);

   /*设置从机地址*/
   modbus_set_slave(ctx, SERVER_ID);

   /*RTU模式 打开串口*/
   if (modbus_connect(ctx) == -1) {
         fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
         modbus_free(ctx);
         return -1;
   }

   //读取多个连续寄存器
   rc = modbus_read_registers(ctx, 0, 5, tab_rp_registers);
   if (rc == -1)
   {
            fprintf(stderr,"%s\n", modbus_strerror(errno));
            return -1;
   }
   for (i=0; i<5; i++)
   {
         //打印读取的数据
         printf("reg[%d] = %d(0x%x)\n", i, tab_rp_registers[i], tab_rp_registers[i]);
   }

   modbus_close(ctx);
   modbus_free(ctx);
   return 0;
}

```

### 8.5.2.3. 编译

将两个程序的源文件传输到开发板系统，然后使用下面命令编译：

---

```bash
gcc test-server.c -o test-server `pkg-config --cflags --libs libmodbus`
gcc test-client.c -o test-client `pkg-config --cflags --libs libmodbus`
```

---

打开两个终端，一个用于运行服务端一个用于运行客户端（ssh终端和串口终端）。如下所示：

服务终端

---

```bash
root@lubancat:~$./test-server
```

---

客户终端

---

```bash
 root@lubancat:~$./test-client
```

---

在客户终端中最终打印

客户终端

---

```bash
Opening /dev/ttyS4 at 9600 bauds (N, 8, 1)
[0A][03][00][00][00][05][84][B2]
Sending request using RTS signal
Waiting for a confirmation...
<0A><03><0A><00><0A><00><0E><00><0A><00><1B><00><0A><C7><5C>
reg[0] = 10(0xa)
reg[1] = 14(0xe)
reg[2] = 10(0xa)
reg[3] = 27(0x1b)
reg[4] = 10(0xa)
```

---

在服务终端中最终打印

服务终端

---

```bash
 Opening /dev/ttyS3 at 9600 bauds (N, 8, 1)
 Bytes flushed (0)
 Waiting for a indication...
 <0A><03><00><00><00><05><84><B2>
 [0A][03][0A][00][0A][00][0E][00][0A][00][1B][00][0A][C7][5C]
 Sending request using RTS signal
 Waiting for a indication...
```

---

以上代码均做测试用，关于具体用法请读者自行研究。
参考资料：
[https://github.com/stephane/libmodbus](https://github.com/stephane/libmodbus)