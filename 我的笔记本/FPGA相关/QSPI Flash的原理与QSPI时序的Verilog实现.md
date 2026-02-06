## QSPI Flash的原理与QSPI时序的Verilog实现

## 一、 软件平台与硬件平台

　　软件平台：

　　　　1、操作系统：Windows-8.1

　　　　2、开发套件：ISE14.7

　　　　3、仿真工具：ModelSim-10.4-SE

　　　　4、Matlab版本：Matlab2014b/Matlab2016a

　　硬件平台：

　　　　1、 FPGA型号：Xilinx公司的XC6SLX45-2CSG324

　　　　2、 Flash型号：WinBond公司的W25Q128BV  Quad SPI Flash存储器

　　提示：如果图片不清晰，请把图片在浏览器的新建标签页打开或保存到本地打开。

## 二、 原理介绍

　　上一篇博客《SPI总线的原理与FPGA实现》中已经有关于标准SPI协议的原理与时序的介绍，这里不再赘述。本节主要是讨论QSPI(Quad SPI，四线SPI总线)的相关内容。我的开发板上有一片型号是W25Q128BV的Quad SPI  Flash存储器，本文将以它为例子来说明QSPI操作的一些内容。

　　W25Q128BV的Quad SPI Flash存储器的Top View如下图所示

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915152901227-2019694658.png)

 

　　这块芯片一共有8个有用的管脚，其每个管脚的功能定义如下图所示

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915152927908-1863935977.png)

 

　　由上图可知2号管脚DO(IO1)，3号管脚  /WP(IO2)，5号管脚DI(IO0)以及7号管脚/HOLD(IO3)均为双向IO口，所以在编写Verilog代码的时候要把它们定义为inout类型，inout类型的信号既可以做输出也可以作为输入，具体在代码里面如何处理后文会有介绍。

　　QSPI Flash每个引脚的详细描述如下：

　　1、Chip Select(/CS)

   　　片选信号Chip  Select(/CS)的作用是使能或者不使能设备的操作，当CS为高时，表示设备未被选中，串行数据输出线(DO或IO0，IO1，IO2，IO3)均处于高阻态，当CS为低时，表示设备被选中，FPGA可以给QSPI Flash发送数据或从QSPI Flash接收数据。

　　2、串行数据输入信号DI以及串行输出信号DO

   　　W25Q128BV支持标准SPI协议，双线SPI(Dual SPI)协议与四线SPI(Quad  SPI)协议。标准的SPI协议在串行时钟信号(SCLK)的上升沿把串行输入信号DI上的数据存入QSPI  Flash中，在串行时钟信号(SCLK)的下降沿把QSPI Flash中的数据串行化通过单向的DO引脚输出。而在Dual SPI与Quad  SPI中，DI与DO均为双向信号(既可以作为输入，也可以作为输出)。

　　3、Write Project(/WP)

   　　写保护信号的作用是防止QSPI Flash的状态寄存器被写入错误的数据，WP信号低电平有效，但是当状态寄存器2的QE位被置1时，WP信号失去写保护功能，它变成Quad SPI的一个双向数据传输信号。

　　4、HOLD(/HOLD)

   　 HOLD信号的作用是暂停QSPI  Flash的操作。当HOLD信号为低，并且CS也为低时，串行输出信号DO将处于高阻态，串行输入信号DI与串行时钟信号SCLK将被QSPI  Flash忽略。当HOLD拉高以后，QSPI  Flash的读写操作能继续进行。当多个SPI设备共享同一组SPI总线相同的信号的时候，可以通过HOLD来切换信号的流向。和WP信号一样，当当状态寄存器2的QE位被置1时，HOLD信号失去保持功能，它也变成Quad SPI的一个双向数据传输信号。

　　5、串行时钟线

   　 串行时钟线用来提供串行输入输出操作的时钟。

　　W25Q128BV的内部结构框图如下图所示：

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915153059258-1627550893.png)

 

　　更多详细的内容请阅读W25Q128BV的芯片手册。由于本文要进行4线SPI的操作，但QSPI Flash默认的操作模式是标准单线SPI模式，所以在每次进行4线SPI操作的时候一定要先把状态寄存器2的QE位(倒数第2位)置1，然后才能进行QSPI操作。

　　最后介绍一下我的开发板上QSPI Flash硬件原理图如下图所示：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915153136548-2035591525.png)

## 三、 目标任务

　　1、编写标准SPI 协议 Verilog代码来操作QSPI Flash，并用ChipScope抓出各个指令的时序与芯片手册提供的时序进行对比

　　2、在标准SPI协议的基础上增加Quad SPI的功能，并用ChipScope抓出Quad SPI的读写数据的时序

　　3、对比标准SPI与Quad SPI读写W25Q128BV的ChipScope时序，感受二者的效率差距

## 四、 设计思路与Verilog代码编写

### 4.1、 命令类型的定义

　　W25Q128BV一共有35条命令，这里不可能把所有命令的逻辑都写出来，所以截取了一部分常用的命令作为示例来说明QSPI Flash的操作方法。由于命令数目很多，所以在这个部分先对各个命令类型做一个初步定义，下文的代码就是按照这个定义来编写的。

| 命令编号 | 命令类型(自定义) | 命令码(芯片手册定义) | 命令功能             |
| -------- | ---------------- | -------------------- | -------------------- |
| 1        | 5’b0XXXX         | 8’h00                | 无                   |
| 2        | 5’b10000         | 8’h90                | 读设备ID             |
| 3        | 5’b10001         | 8’h06                | 写使能               |
| 4        | 5’b10010         | 8’h20                | 扇区擦除             |
| 5        | 5’b10011         | 8’h05/8’h35          | 读状态寄存器1/2      |
| 6        | 5’b10100         | 8’h04                | 关闭写使能           |
| 7        | 5’b10101         | 8’h02                | 写数据操作(单线模式) |
| 8        | 5’b10110         | 8’h01                | 写状态寄存器         |
| 9        | 5’b10111         | 8’h03                | 读数据操作(单线模式) |
| 10       | 5’b11000         | 8’h32                | 写数据操作(四线模式) |
| 11       | 5’b11001         | 8’h6b                | 读数据操作(四线模式) |

> [!note]　　
>
> **说明：**
>
> 　　**1、命令类型是我自己随便定义的，可以随便修改。命令码是芯片手册上定义好，不能修改，更详细的内容请参考W25Q128芯片手册。**
>
> 　　**2、命令类型的最高位是使能位，只有当最高位为1时，命令才有效(在代码里面写的就是只有当最高位为1时才能进入SPI操作的状态机)。**
>
> 　　**3、进行四线读写操作之前，一定要把四线读写功能的使能位打开，方法是通过写状态寄存器命令把状态寄存器2的QE位(倒数第二位)置1。**
>

### 4.2、 如何用Matlab产生存放在ROM中的.coe文件格式的数据

　　上一节设计了一个把存放在ROM中的数据用SPI总线发出来的例子，ROM里面只存放了10个数据，所以可以直接把这10个数据填写到.coe文件就可以了，由于QSPI Flash的页编程(写数据)指令最大支持256字节的写操作，所以下面的例子的功能是把ROM中存放的256个字节(8-bit)数据先写入QSPI  Flash中，然后在读出来。由于数据太多(256个),所以一个一个填写肯定不现实，所以可以利用Matlab来直接产生.coe文件，Matlab的完整代码如下：

```matlab
width=8;   %rom中数据的宽度
depth=256; %rom的深度
y=0:255;   
y=fliplr(y); %产生要发送的数据，255,254,253，...... ，2,1,0
fid = fopen('test_data.coe', 'w'); % 打开一个.coe文件

% 存放在ROM中的.coe文件第一行必须是这个字符串，16表示16进制，可以改成其他进制
fprintf(fid,'memory_initialization_radix=16;\n'); 

% 存放在ROM中的.coe文件第二行必须是这个字符串
fprintf(fid,'memory_initialization_vector=\n'); 

% 把前255个数据写入.coe文件中，并用逗号隔开，为了方便知道数据的个数，每行只写一个数据
fprintf(fid,'%x,\n',y(1:end-1));
 
% 把最后一个数据写入.coe文件中，并用分号结尾
fprintf(fid,'%x;\n',y(end)); 
fclose(fid);  % 关闭文件指针
```

用Matlab2014b运行上面的代码以后会在与这个.m文件相同的目录下产生一个.coe文件，这个.coe文件可以导入到ROM中。

### 4.3、 标准SPI总线操作QSPI Flash思路与代码编写

　　上一篇博客《SPI总线的原理与FPGA实现》已经介绍过用spi_module这个模块去读取QSPI  Flash的Manufacturer/Device  ID，事实上除了上篇博客提供的那种方法以外，还可以直接在时钟信号的下降沿发送数据，时钟信号的上升沿来接受数据来完成读ID的操作，当FPGA在时钟的下降沿发送数据的时候，那么时钟的上升沿刚好在数据的正中间，QSPI  Flash刚好可以在这个上升沿把数据读进来，读操作则正好相反。但是有很多有经验的人告诉我在设计中如非必要最好不要使用时钟下降沿触发的设计方法，可能是因为大多数FPGA里面的Flip Flops资源都是上升沿触发的，如果在Verilog代码采用下降沿触发的话  ，综合的时候会在CLK输入信号前面综合出一个反相器，这个反相器可能会对时钟信号的质量有影响，具体的原因等我再Google上继续搜索一段时间在说。这个例子由于状态机相较前几篇博客来说相对复杂，所以接下来写代码我还是采用下降沿发送数据，上升沿接收数据的方式来描述这个状态机。

　　接下来的任务就是抽象出一个状态机。上一篇博客仅仅读一个ID就用了6个状态，所以采用上一篇博客的设计思路显然不太现实，但对于初学者而言，上一篇博客仍然有一个基本的指引作用。通过阅读QSPI Flash的芯片手册，可以发现，所有的命令其实至多由以下三个部分组成：

　　　　1、发送8-bit的命令码

　　　　2、发送24-bit的地址码

　　　　3、发送数据或接收数据

　　所有命令的状态跳变图可由下图描述

![img](https://img2018.cnblogs.com/blog/1426240/201811/1426240-20181123085739594-648045779.png)

　　所以按照这个思路来思考的话抽象出来的状态机的状态并不多。单线模式的状态为以下几个：

　　　　1、空闲状态：用来初始化各个寄存器的值

　　　　2、发送命令状态：用来发送8-bit的命令码

　　　　3、发送地址状态：用来发送24-bit的地址码

　　　　4、读等待状态：当读数据操作正在进行的时候进入此状态等待读数据完毕

　　　　5、写数据状态(单线模式)：在这个状态FPGA往QSPI Flash里面写数据

　　　　6、结束状态：一条指令操作结束，并给出一个结束标志

　　完整的代码如下：

```verilog
`timescale 1ns / 1ps

module qspi_driver
(
output                  O_qspi_clk          , // SPI总线串行时钟线
output reg              O_qspi_cs           , // SPI总线片选信号
output reg              O_qspi_mosi         , // SPI总线输出信号线，也是QSPI Flash的输入信号线
input                   I_qspi_miso         , // SPI总线输入信号线，也是QSPI Flash的输出信号线
                                            
input                   I_rst_n             , // 复位信号

input                   I_clk_25M           , // 25MHz时钟信号
input       [4:0]       I_cmd_type          , // 命令类型
input       [7:0]       I_cmd_code          , // 命令码
input       [23:0]      I_qspi_addr         , // QSPI Flash地址

output reg              O_done_sig          , // 指令执行结束标志
output reg  [7:0]       O_read_data         , // 从QSPI Flash读出的数据
output reg              O_read_byte_valid   , // 读一个字节完成的标志
output reg  [3:0]       O_qspi_state          // 状态机，用于在顶层调试用
);


parameter   C_IDLE            =   4'b0000  ; // 空闲状态
parameter   C_SEND_CMD        =   4'b0001  ; // 发送命令码
parameter   C_SEND_ADDR       =   4'b0010  ; // 发送地址码
parameter   C_READ_WAIT       =   4'b0011  ; // 读等待
parameter   C_WRITE_DATA      =   4'b0101  ; // 写数据
parameter   C_FINISH_DONE     =   4'b0110  ; // 一条指令执行结束

reg         [7:0]   R_read_data_reg     ; // 从Flash中读出的数据用这个变量进行缓存，等读完了在把这个变量的值给输出
reg                 R_qspi_clk_en       ; // 串行时钟使能信号
reg                 R_data_come_single  ; // 单线操作读数据使能信号，当这个信号为高时
            
reg         [7:0]   R_cmd_reg           ; // 命令码寄存器
reg         [23:0]  R_address_reg       ; // 地址码寄存器 
reg         [7:0]   R_write_bits_cnt    ; // 写bit计数器，写数据之前把它初始化为7，发送一个bit就减1
reg         [8:0]   R_write_bytes_cnt   ; // 写字节计数器，发送一个字节数据就把它加1
reg         [7:0]   R_read_bits_cnt     ; // 写bit计数器，接收一个bit就加1
reg         [8:0]   R_read_bytes_cnt    ; // 读字节计数器，接收一个字节数据就把它加1
reg         [8:0]   R_read_bytes_num    ; // 要接收的数据总数
reg                 R_read_finish       ; // 读数据结束标志位

wire        [7:0]   W_rom_addr          ;  
wire        [7:0]   W_rom_out           ;  

assign O_qspi_clk = R_qspi_clk_en ? I_clk_25M : 0   ; // 产生串行时钟信号
assign W_rom_addr = R_write_bytes_cnt               ;

////////////////////////////////////////////////////////////////////////////////////////////
// 功能：用时钟的下降沿发送数据
////////////////////////////////////////////////////////////////////////////////////////////
always @(negedge I_clk_25M)
begin
    if(!I_rst_n)
        begin
            O_qspi_cs           <=  1'b1   ;        
            O_qspi_state        <=  C_IDLE ;
            R_cmd_reg           <=  0      ;
            R_address_reg       <=  0      ;
            R_qspi_clk_en       <=  1'b0   ;  //SPI clock输出不使能
            R_write_bits_cnt    <=  0      ;
            R_write_bytes_cnt   <=  0      ;
            R_read_bytes_num    <=  0      ;    
            R_address_reg       <=  0      ;
            O_done_sig          <=  1'b0   ;
            R_data_come_single  <=  1'b0   ;           
        end
    else
        begin
            case(O_qspi_state)
                C_IDLE:  // 初始化各个寄存器，当检测到命令类型有效(命令类型的最高位位1)以后,进入发送命令码状态
                    begin                              
                        R_qspi_clk_en  <=   1'b0         ;
                        O_qspi_cs      <=   1'b1         ;
                        O_qspi_mosi    <=   1'b0         ;    
                        R_cmd_reg      <=   I_cmd_code   ;
                        R_address_reg  <=   I_qspi_addr  ;
                        O_done_sig     <=   1'b0         ;            
                        if(I_cmd_type[4] == 1'b1) 
                            begin                //如果flash操作命令请求
                                O_qspi_state        <=  C_SEND_CMD  ;
                                R_write_bits_cnt    <=  7           ;        
                                R_write_bytes_cnt   <=  0           ;
                                R_read_bytes_num    <=  0           ;                    
                            end
                    end
                C_SEND_CMD: // 发送8-bit命令码状态 
                    begin
                        R_qspi_clk_en       <=  1'b1    ; // 打开SPI串行时钟SCLK的使能开关
                        O_qspi_cs           <=  1'b0    ; // 拉低片选信号CS
                        if(R_write_bits_cnt > 0) 
                            begin                           //如果R_cmd_reg还没有发送完
                                O_qspi_mosi        <=  R_cmd_reg[R_write_bits_cnt] ;         //发送bit7~bit1位
                                R_write_bits_cnt   <=  R_write_bits_cnt-1'b1       ;
                            end                            
                        else 
                            begin                                 //发送bit0
                                O_qspi_mosi <=  R_cmd_reg[0]    ;
                                if ((I_cmd_type[3:0] == 4'b0001) | (I_cmd_type[3:0] == 4'b0100)) 
                                    begin    //如果是写使能指令(Write Enable)或者写不使能指令(Write Disable)
                                        O_qspi_state    <=  C_FINISH_DONE   ;
                                    end                          
                                else if (I_cmd_type[3:0] == 4'b0011) 
                                    begin    //如果是读状态寄存器指令(Read Register)
                                        O_qspi_state        <=  C_READ_WAIT ;
                                        R_write_bits_cnt    <=  7           ;
                                        R_read_bytes_num    <=  1           ;//读状态寄存器指令需要接收一个数据 
                                    end                             
                                else if( (I_cmd_type[3:0] == 4'b0010) || (I_cmd_type[3:0] == 4'b0101) || (I_cmd_type[3:0] == 4'b0111) || (I_cmd_type[3:0] == 4'b0000) ) 
                                    begin // 如果是扇区擦除(Sector Erase),页编程指令(Page Program),读数据指令(Read Data),读设备ID指令(Read Device ID)                          
                                        O_qspi_state        <=  C_SEND_ADDR ;
                                        R_write_bits_cnt    <=  23          ; // 这几条指令后面都需要跟一个24-bit的地址码
                                    end
                            end
                    end
                C_SEND_ADDR: // 发送地址状态
                    begin
                        if(R_write_bits_cnt > 0)  //如果R_cmd_reg还没有发送完
                            begin                                 
                                O_qspi_mosi        <=  R_address_reg[R_write_bits_cnt] ; //发送bit23~bit1位
                                R_write_bits_cnt   <=  R_write_bits_cnt    -   1       ;    
                            end                                 
                        else 
                            begin 
                                O_qspi_mosi <=  R_address_reg[0]    ;   //发送bit0
                                if(I_cmd_type[3:0] == 4'b0010) // 扇区擦除(Sector Erase)指令
                                    begin  //扇区擦除(Sector Erase)指令发完24-bit地址码就执行结束了，所以直接跳到结束状态
                                        O_qspi_state <= C_FINISH_DONE   ;    
                                    end
                                else if (I_cmd_type[3:0] == 4'b0101) // 页编程(Page Program)指令
                                    begin                              
                                        O_qspi_state        <=  C_WRITE_DATA    ;
                                        R_write_bits_cnt    <=  7               ;                       
                                    end
                                else if (I_cmd_type[3:0] == 4'b0000) // 读Device ID指令
                                    begin             
                                        O_qspi_state        <=  C_READ_WAIT     ;
                                        R_read_bytes_num    <=  2               ; //接收2个数据的Device ID
                                    end                                                         
                                else if (I_cmd_type[3:0] == 4'b0111) // 读数据(Read Data)指令
                                    begin
                                        O_qspi_state        <=  C_READ_WAIT     ;
                                        R_read_bytes_num    <=  256             ;   //接收256个数据        
                                    end                                        
                            end
                    end                  
                C_READ_WAIT: // 读等待状态
                    begin
                        if(R_read_finish)  
                            begin
                                O_qspi_state        <=  C_FINISH_DONE   ;
                                R_data_come_single  <=  1'b0            ;
                            end
                        else
                            begin
                                R_data_come_single  <=  1'b1            ; // 单线模式下读数据标志信号，此信号为高标志正在接收数据
                            end
                    end
                C_WRITE_DATA: // 写数据状态
                    begin
                        if(R_write_bytes_cnt < 256) // 往QSPI Flash中写入 256个数据
                            begin                       
                                if(R_write_bits_cnt > 0) //如果数据还没有发送完
                                    begin                           
                                        O_qspi_mosi         <=  W_rom_out[R_write_bits_cnt] ; //发送bit7~bit1位
                                        R_write_bits_cnt    <=  R_write_bits_cnt  - 1'b1    ;                        
                                    end                 
                                else 
                                    begin                                 
                                        O_qspi_mosi         <=  W_rom_out[0]                ; //发送bit0
                                        R_write_bits_cnt    <=  7                           ;
                                        R_write_bytes_cnt   <=  R_write_bytes_cnt + 1'b1    ;
                                    end
                            end
                        else 
                            begin
                                O_qspi_state    <=  C_FINISH_DONE   ;
                                R_qspi_clk_en   <=  1'b0            ;
                            end
                    end
                C_FINISH_DONE:
                    begin
                        O_qspi_cs           <=  1'b1    ;
                        O_qspi_mosi         <=  1'b0    ;
                        R_qspi_clk_en       <=  1'b0    ;
                        O_done_sig          <=  1'b1    ;
                        R_data_come_single  <=  1'b0    ;
                        O_qspi_state        <=  C_IDLE  ;
                    end
                default:O_qspi_state    <=  C_IDLE      ;
            endcase         
        end
end

//////////////////////////////////////////////////////////////////////////////
// 功能：接收QSPI Flash发送过来的数据    
//////////////////////////////////////////////////////////////////////////////
always @(posedge I_clk_25M)
begin
    if(!I_rst_n)
        begin
            R_read_bytes_cnt    <=  0       ;
            R_read_bits_cnt     <=  0       ;
            R_read_finish       <=  1'b0    ;
            O_read_byte_valid   <=  1'b0    ;
            R_read_data_reg     <=  0       ;
            O_read_data         <=  0       ;
        end
    else if(R_data_come_single)   // 此信号为高表示接收数据从QSPI Flash发过来的数据
        begin
            if(R_read_bytes_cnt < R_read_bytes_num) 
                begin            
                    if(R_read_bits_cnt < 7)  //接收一个Byte的bit0~bit6    
                        begin                         
                            O_read_byte_valid   <=  1'b0                               ;
                            R_read_data_reg     <=  {R_read_data_reg[6:0],I_qspi_miso} ;
                            R_read_bits_cnt     <=  R_read_bits_cnt +   1'b1           ;
                        end
                    else  
                        begin
                            O_read_byte_valid   <=  1'b1                               ;  //一个byte数据有效
                            O_read_data         <=  {R_read_data_reg[6:0],I_qspi_miso} ;  //接收bit7
                            R_read_bits_cnt     <=  0                                  ;
                            R_read_bytes_cnt    <=  R_read_bytes_cnt    +   1'b1       ;
                        end
                end                               
            else 
                begin 
                    R_read_bytes_cnt    <=  0       ;
                    R_read_finish       <=  1'b1    ;
                    O_read_byte_valid   <=  1'b0    ;
                end
        end                               
    else 
        begin
            R_read_bytes_cnt    <=  0       ;
            R_read_bits_cnt     <=  0       ;
            R_read_finish       <=  1'b0    ;
            O_read_byte_valid   <=  1'b0    ;
            R_read_data_reg     <=  0       ;
        end
end         

rom_data rom_data_inst (
  .clka(I_clk_25M), // input clka
  .addra(W_rom_addr), // input [7 : 0] addra
  .douta(W_rom_out) // output [7 : 0] douta
);

endmodule
```

接下来就是写一个测试代码对这个单线模式SPI驱动，为了保证把上面的所有指令都测试一遍，测试代码如下：

```verilog
module qspi_top
(
    input         I_clk         ,
    input         I_rst_n       ,

    output        O_qspi_clk    , // SPI总线串行时钟线
    output        O_qspi_cs     , // SPI总线片选信号
    output        O_qspi_mosi   , // SPI总线输出信号线，也是QSPI Flash的输入信号线
    input         I_qspi_miso     // SPI总线输入信号线，也是QSPI Flash的输出信号线
     
);
     
reg [3:0]   R_state             ;
reg [7:0]   R_flash_cmd         ;
reg [23:0]  R_flash_addr        ;
reg         R_clk_25M           ;
reg [4:0]   R_cmd_type          ;
                                
wire        W_done_sig          ;
wire [7:0]  W_read_data         ;
wire        W_read_byte_valid   ;
wire [2:0]  R_qspi_state        ;


////////////////////////////////////////////////////////////////////          
//功能：二分频逻辑          
////////////////////////////////////////////////////////////////////          
always @(posedge I_clk or negedge I_rst_n)
begin
    if(!I_rst_n) 
        R_clk_25M   <=  1'b0        ;
    else 
        R_clk_25M   <=  ~R_clk_25M  ;
end
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//功能：测试状态机
////////////////////////////////////////////////////////////////////
always @(posedge R_clk_25M or negedge I_rst_n)
begin
    if(!I_rst_n) 
        begin
            R_state         <=  4'd0        ;
            R_flash_addr    <=  24'd0       ;
            R_flash_cmd     <=  8'h00       ;
            R_cmd_type      <=  5'b0_0000   ;
        end
     else 
        begin
            case(R_state)           
                4'd0://读Device ID指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd  <= 8'h90           ; 
                                R_flash_addr <= 24'd0           ; 
                                R_cmd_type   <= 5'b1_0000       ; 
                            end     
                    end 
                4'd1://写Write disable instruction
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ;
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h04            ; 
                                R_cmd_type  <= 5'b1_0100        ; 
                            end     
                    end                
                4'd2://写使能(Write Enable)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin
                                R_flash_cmd <= 8'h06            ; 
                                R_cmd_type  <= 5'b1_0001        ; 
                            end
                    end         
                4'd3:// 扇区擦除(Sector Erase)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h20            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_0010        ; 
                            end
                    end
            
                4'd4://读状态寄存器1, 当Busy位(状态寄存器1的最低位)为0时表示擦除操作完成
                    begin
                        if(W_done_sig) 
                            begin 
                                if(W_read_data[0]==1'b0) 
                                    begin 
                                        R_flash_cmd <= 8'h00            ; 
                                        R_state     <= R_state + 1'b1   ;
                                        R_cmd_type  <= 5'b0_0000        ; 
                                    end
                                else 
                                    begin 
                                        R_flash_cmd <= 8'h05        ; 
                                        R_cmd_type  <= 5'b1_0011    ; 
                                    end
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h05        ; 
                                R_cmd_type  <= 5'b1_0011    ; 
                            end
                    end
                4'd5://写使能(Write Enable)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin
                                R_flash_cmd <= 8'h06            ; 
                                R_cmd_type  <= 5'b1_0001        ; 
                            end
                    end             
                4'd6: //页编程操作(Page Program): 把存放在ROM中的数据写入QSPI Flash中
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h02            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_0101        ; 
                            end
                    end
                4'd7://读状态寄存器1, 当Busy位(状态寄存器1的最低位)为0时表示写操作完成
                    begin
                        if(W_done_sig) 
                            begin 
                                if(W_read_data[0]==1'b0) 
                                    begin 
                                        R_flash_cmd <= 8'h00            ; 
                                        R_state     <= R_state + 1'b1   ;
                                        R_cmd_type  <= 5'b0_0000        ; 
                                    end
                                else 
                                    begin 
                                        R_flash_cmd <= 8'h05        ; 
                                        R_cmd_type  <= 5'b1_0011    ; 
                                    end
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h05        ; 
                                R_cmd_type  <= 5'b1_0011    ; 
                            end
                    end           
                4'd8://读256 Bytes
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ;
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h03            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_0111        ; 
                            end
                    end
            
                4'd9:// 空闲状态
                    begin
                        R_flash_cmd <= 8'h00            ; 
                        R_state     <= 4'd9             ;
                        R_cmd_type  <= 5'b0_0000        ; 
                    end
                default :   R_state     <= 4'd0         ;
            endcase
        end           
end 
qspi_driver U_qspi_driver
(
.O_qspi_clk          (O_qspi_clk        ), // SPI总线串行时钟线
.O_qspi_cs           (O_qspi_cs         ), // SPI总线片选信号
.O_qspi_mosi         (O_qspi_mosi       ), // SPI总线输出信号线，也是QSPI Flash的输入信号线
.I_qspi_miso         (I_qspi_miso       ), // SPI总线输入信号线，也是QSPI Flash的输出信号线
                   
.I_rst_n             (I_rst_n           ), // 复位信号

.I_clk_25M           (R_clk_25M         ), // 25MHz时钟信号
.I_cmd_type          (R_cmd_type        ), // 命令类型
.I_cmd_code          (R_flash_cmd       ), // 命令码
.I_qspi_addr         (R_flash_addr      ), // QSPI Flash地址

.O_done_sig          (W_done_sig        ), // 指令执行结束标志
.O_read_data         (W_read_data       ), // 从QSPI Flash读出的数据
.O_read_byte_valid   (W_read_byte_valid ), // 读一个字节完成的标志
.O_qspi_state        (R_qspi_state      )  // 状态机，用于在顶层调试用
);
     
wire [35:0]     CONTROL0    ;
wire [69:0]     TRIG0       ;
icon icon_inst (
    .CONTROL0(CONTROL0) // INOUT BUS [35:0]
);

ila ila_inst (
    .CONTROL(CONTROL0)  , // INOUT BUS [35:0]
    .CLK(I_clk)           ,      // IN
    .TRIG0(TRIG0)      // IN BUS [255:0]
);                                                     

assign  TRIG0[7:0]      =   W_read_data         ;                                               
assign  TRIG0[8]        =   W_read_byte_valid   ;   
assign  TRIG0[12:9]     =   R_state             ;        
assign  TRIG0[16:13]    =   R_qspi_state        ;   
assign  TRIG0[17]       =   W_done_sig          ;    
assign  TRIG0[18]       =   I_qspi_miso         ;  
assign  TRIG0[19]       =   O_qspi_mosi         ;  
assign  TRIG0[20]       =   O_qspi_cs           ;  
assign  TRIG0[21]       =   O_qspi_clk          ; 
assign  TRIG0[26:22]    =   R_cmd_type          ; 
assign  TRIG0[34:27]    =   R_flash_cmd         ; 
assign  TRIG0[58:35]    =   R_flash_addr        ; 
assign  TRIG0[59]       =   I_rst_n             ; 


endmodule
```

接下来主要看看用ChipScope抓出来的时序图和芯片手册规定的时序图是否完全一致。

　　1、读ID指令

​    芯片手册指令的读ID指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915153900493-529226718.png)

　　ChipScope抓出来的时序图

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915153926753-1202639708.png)

 

　　2、写不使能(Write Disable)指令

​    芯片手册指令的写不使能时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915153948758-1539983891.png)

​    ChipScope抓出来的写不使能时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154008514-1233136443.png)

　　3、写使能(Write Enable)指令

​    芯片手册指令的写使能时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154034258-940277774.png)

​     ChipScope抓出来的写使能时序图

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154046993-196365382.png)

　　3、扇区擦除(Sector Erase)指令

​    芯片手册的扇区擦除指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154109903-225214572.png)

​    芯片手册的扇区擦除指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154119518-979765546.png)

　　4、读状态寄存器(Read Status Register)指令

​    芯片手册的读状态寄存器指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154136668-1924229584.png)

​    ChipScope抓回来的读状态寄存器指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154148968-1907901612.png)

　　由于在擦除操作和写操作(Page Program)操作指令发送完毕以后，芯片内部会自己执行相关的操作并把状态寄存器1中的最低位Busy位拉高，状态寄存器1的各位如下：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154248078-1330622951.png)

　　所以读状态寄存器指令的目的是为了检测Busy是否为高，Busy为高的话说明擦除操作或写操作(Page  Program)操作指令并未完成，这种情况应该等待不发送其他指令，等到Busy位拉低以后说明擦除操作或写操作(Page  Program)操作指令已经完成，这时才可以执行后续的操作。值得注意的是，在上图中读到8个bit数据(上图读到的数据是0000_0011(8’h03))以后，O_qpsi_cs和O_qspi_clk信号仍然持续了2个周期的时间才变为空闲状态，实际上，回过头去看代码的话很容易解释这种情况，由于代码里面是用时钟的下降沿发送数据，而用时钟的上升沿来接收数据，发送数据与接收数据的状态切换是通过R_data_coming_signal和R_read_finish信号来转换的，所以导致了这种情况的发生。但是读数据相关的指令并不要求完全8-bit对齐，如果在读数据的过程中只要O_qpsi_cs一直为低，那么会一直重复读下去，直至O_qpsi_cs拉高为止才停止读数据，所以上面多出来的两个时钟并不会影响结果的正确性。

　　5、写数据(Page Program)指令(单线模式)

​    芯片手册的写数据指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154321638-344267680.png)

　　ChipScope抓回来的时序图

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154343488-256879760.png)

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154358068-1908970268.png)

 

　　6、读数据(Read Data)指令(单线模式)

​    芯片手册的读数据指令时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154430668-272376497.png)

　　ChipScope抓回来的时序图：

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154457208-1310970211.png)

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154512218-167866786.png)

 

　　通过上面用ChipScope抓回来的时序图与芯片手册的时序图进行对比可以很清晰的理解整个QSPI Flash的操作流程与时序细节。

### 4.4、 如何处理双向信号(Verilog中用关键词inout定义的信号都是双向信号)

　　四线操作中IO0，IO1，IO2和IO3全部都可以用来发送数据以及接收数据，所以在编写代码的时候要把它们全部定义成双向类型的信号，即inout类型。所以在编写四线操作的代码之前有必要提前熟悉一下inout信号的处理方法。

> [!note]　　
>
> **总的来说，inout信号大致有以下4个特征：**
>
> 　　　　**1、inout端口不能被赋值为reg型，因此，不能用于always语句中。**
>
> 　　　　**2、对于inout端口的逻辑判断，要用到？：条件表达式，来控制高阻的赋值**
>
> 　　　　**3、inout信号需要有一个中转的寄存器，在always语句中才可以将输入的信号赋给输出(用inout代替纯output)**
>
> 　　　　**4、高阻态不要用于芯片内部，应该用逻辑引到引脚处，然后用高阻来实现。**

实际上，FPGA内部的双向信号是通过一个三态门来实现的，一个典型的三态门结构如下

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154703663-946056288.png)

　　描述这个三态门的Verilog代码如下：

```veri
module Test_inout
(
input   I_clk,
input   I_rst_n,
    .
    .
    .
inout   IO_data,
    .
    .
    . 
)

reg     R_data_out  ;
wire    I_data_in   ;
assign  IO_data = Control ? R_data_out : 1'bz ;
assign  I_data_in   =   IO_data ;

always @(posedge I_clk or negedge I_rst_n)
begin
    .
    .
    .
    ;
end

endmodule
```

上面的代码表达的意思是：

> [!important]
>
> **当Control为1是，IO_data信号作为输出，输出R_data_out的值，当Control为0时，IO_data信号作为输入，输入的值赋给I_data_in变量。**
>
> **值得注意的是，inout作为输出的时候不能直接给他赋值，而需要通过一个中间变量R_data_out来间接赋值，同时inout变量的输入输出属性必须通过一个控制信号Control来控制，控制性号为高时为inout信号作为输出，输出R_data_out的值，为低时inout处于高阻状态，这时才作为输入接收外部的数据。**

### 4.5、 四线SPI总线操作QSPI Flash思路与代码编写

　　上一小节已经完成了单线模式QSPI  Flash的读写以及其他相关操作，这一节将在上一节的基础上加上四线读写功能。

**通过进一步阅读W25Q128BV的芯片手册可知，在进行四线读写操作之前一定要把QE(Quad Enable)位置为1**，而QE位在状态寄存器2的倒数第二位，见下图

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915154833823-207198662.png)

　　除此以外，四线读操作还需要在读数据之前等待8个dummy  clock用来加快读数据的速度(详细内容请阅读芯片手册)。

**所以，四线操作相对于单线操作而言除了要增加四线模式读写数据的状态以外还要增加一个写状态寄存器的功能用来开启QE(Quad Enable)位以及一个dummy用来等待8个clock。**

　　综上所述，四线模式的状态为以下几个：

　　　　1、空闲状态：用来初始化各个寄存器的值

　　　　2、发送命令状态：用来发送8-bit的命令码

　　　　3、发送地址状态：用来发送24-bit的地址码

　　　　4、读等待状态(单线模式)：当读数据操作正在进行的时候进入此状态等待读数据完毕

　　　　5、写数据状态(单线模式)：在这个状态FPGA往QSPI Flash里面写数据

　　　　6、写状态寄存器状态：用来把状态寄存器的QE(Quad Enable)置1

　　　　7、Dummy Clock状态：四线读数据之前需要等待8个dummy clock

　　　　8、写数据状态(四线模式)：在这个状态FPGA往QSPI Flash里面通过四线模式写数据

　　　　9、读等待状态(四线模式)：在这个状态等待FPGA从QSPI Flash里面通过四线模式读数据完成

　　　　10、结束状态：一条指令操作结束，并给出一个结束标志

　　其中标红的状态是四线模式的代码在单线模式代码的基础上增加的四个状态。

　　四线模式的完整代码如下所示：

```verilog
`timescale 1ns / 1ps

module qspi_driver
(
output                  O_qspi_clk          , // QSPI Flash Quad SPI(QPI)总线串行时钟线
output reg              O_qspi_cs           , // QPI总线片选信号
inout                   IO_qspi_io0         , // QPI总线输入/输出信号线
inout                   IO_qspi_io1         , // QPI总线输入/输出信号线
inout                   IO_qspi_io2         , // QPI总线输入/输出信号线
inout                   IO_qspi_io3         , // QPI总线输入/输出信号线
                                            
input                   I_rst_n             , // 复位信号

input                   I_clk_25M           , // 25MHz时钟信号
input       [4:0]       I_cmd_type          , // 命令类型
input       [7:0]       I_cmd_code          , // 命令码
input       [23:0]      I_qspi_addr         , // QSPI Flash地址
input       [15:0]      I_status_reg        , // QSPI Flash状态寄存器的值

output reg              O_done_sig          , // 指令执行结束标志
output reg  [7:0]       O_read_data         , // 从QSPI Flash读出的数据
output reg              O_read_byte_valid   , // 读一个字节完成的标志
output reg  [3:0]       O_qspi_state          // 状态机，用于在顶层调试用
);


parameter   C_IDLE            =   4'b0000  ; // 空闲状态
parameter   C_SEND_CMD        =   4'b0001  ; // 发送命令码
parameter   C_SEND_ADDR       =   4'b0010  ; // 发送地址码
parameter   C_READ_WAIT       =   4'b0011  ; // 单线模式读等待
parameter   C_WRITE_DATA      =   4'b0101  ; // 单线模式写数据到QSPI Flash
parameter   C_FINISH_DONE     =   4'b0110  ; // 一条指令执行结束

parameter     C_WRITE_STATE_REG =   4'b0111  ; // 写状态寄存器
parameter     C_WRITE_DATA_QUAD =   4'b1000  ; // 四线模式写数据到QSPI Flash
parameter     C_DUMMY           =   4'b1001  ; // 四线模式读数据需要8个时钟周期的dummy clock，这可以加快读数据的速度
parameter     C_READ_WAIT_QUAD  =   4'b1010  ; // 四线模式读等待状态

// QSPI Flash IO输入输出状态控制寄存器
reg         R_qspi_io0          ;
reg         R_qspi_io1          ;
reg         R_qspi_io2          ;
reg         R_qspi_io3          ;          
reg         R_qspi_io0_out_en   ;
reg         R_qspi_io1_out_en   ;
reg         R_qspi_io2_out_en   ;
reg         R_qspi_io3_out_en   ;

reg         [7:0]   R_read_data_reg     ; // 从Flash中读出的数据用这个变量进行缓存，等读完了在把这个变量的值给输出
reg                 R_qspi_clk_en       ; // 串行时钟使能信号
reg                 R_data_come_single  ; // 单线操作读数据使能信号，当这个信号为高时
reg                 R_data_come_quad      ; // 单线操作读数据使能信号，当这个信号为高时
            
reg         [7:0]   R_cmd_reg           ; // 命令码寄存器
reg         [23:0]  R_address_reg       ; // 地址码寄存器 
reg         [15:0]  R_status_reg        ; // 状态寄存器

reg         [7:0]   R_write_bits_cnt    ; // 写bit计数器，写数据之前把它初始化为7，发送一个bit就减1
reg         [8:0]   R_write_bytes_cnt   ; // 写字节计数器，发送一个字节数据就把它加1
reg         [7:0]   R_read_bits_cnt     ; // 写bit计数器，接收一个bit就加1
reg         [8:0]   R_read_bytes_cnt    ; // 读字节计数器，接收一个字节数据就把它加1
reg         [8:0]   R_read_bytes_num    ; // 要接收的数据总数
reg                 R_read_finish       ; // 读数据结束标志位

wire        [7:0]   W_rom_addr          ;  
wire        [7:0]   W_rom_out           ;  

assign O_qspi_clk = R_qspi_clk_en ? I_clk_25M : 0   ; // 产生串行时钟信号
assign W_rom_addr = R_write_bytes_cnt               ;

// QSPI IO方向控制
assign IO_qspi_io0     =   R_qspi_io0_out_en ? R_qspi_io0 : 1'bz ;                
assign IO_qspi_io1     =   R_qspi_io1_out_en ? R_qspi_io1 : 1'bz ;                
assign IO_qspi_io2     =   R_qspi_io2_out_en ? R_qspi_io2 : 1'bz ;                
assign IO_qspi_io3     =   R_qspi_io3_out_en ? R_qspi_io3 : 1'bz ; 
////////////////////////////////////////////////////////////////////////////////////////////
// 功能：用时钟的下降沿发送数据
////////////////////////////////////////////////////////////////////////////////////////////
always @(negedge I_clk_25M)
begin
    if(!I_rst_n)
        begin
            O_qspi_cs           <=  1'b1   ;        
            O_qspi_state        <=  C_IDLE ;
            R_cmd_reg           <=  0      ;
            R_address_reg       <=  0      ;
            R_qspi_clk_en       <=  1'b0   ;  //QSPI clock输出不使能
            R_write_bits_cnt    <=  0      ;
            R_write_bytes_cnt   <=  0      ;
            R_read_bytes_num    <=  0      ;    
            R_address_reg       <=  0      ;
            O_done_sig          <=  1'b0   ;
            R_data_come_single  <=  1'b0   ;           
            R_data_come_quad      <=  1'b0   ;           
        end
    else
        begin
            case(O_qspi_state)
                C_IDLE:  // 初始化各个寄存器，当检测到命令类型有效(命令类型的最高位位1)以后,进入发送命令码状态
                    begin                              
                        R_qspi_clk_en          <=   1'b0         ;
                        O_qspi_cs              <=   1'b1         ;
                        R_qspi_io0             <=   1'b0         ;    
                        R_cmd_reg              <=   I_cmd_code   ;
                        R_address_reg          <=   I_qspi_addr  ;
                        R_status_reg           <=   I_status_reg ;
                        O_done_sig             <=   1'b0         ;
                        R_qspi_io3_out_en   <=   1'b0         ; // 设置IO_qspi_io3为高阻
                        R_qspi_io2_out_en   <=   1'b0         ; // 设置IO_qspi_io2为高阻
                        R_qspi_io1_out_en   <=   1'b0         ; // 设置IO_qspi_io1为高阻
                        R_qspi_io0_out_en   <=   1'b0         ; // 设置IO_qspi_io0为高阻
                        if(I_cmd_type[4] == 1'b1) 
                            begin                //如果flash操作命令请求
                                O_qspi_state        <=  C_SEND_CMD  ;
                                R_write_bits_cnt    <=  7           ;        
                                R_write_bytes_cnt   <=  0           ;
                                R_read_bytes_num    <=  0           ;                    
                            end
                    end
                C_SEND_CMD: // 发送8-bit命令码状态 
                    begin
                        R_qspi_io0_out_en   <=  1'b1    ; // 设置IO_qspi_io0为输出
                        R_qspi_clk_en       <=  1'b1    ; // 打开SPI串行时钟SCLK的使能开关
                        O_qspi_cs           <=  1'b0    ; // 拉低片选信号CS
                        if(R_write_bits_cnt > 0) 
                            begin                           //如果R_cmd_reg还没有发送完
                                R_qspi_io0            <=  R_cmd_reg[R_write_bits_cnt] ;         //发送bit7~bit1位
                                R_write_bits_cnt       <=  R_write_bits_cnt-1'b1       ;
                            end                            
                        else 
                            begin                                 //发送bit0
                                R_qspi_io0 <=  R_cmd_reg[0]    ;
                                if ((I_cmd_type[3:0] == 4'b0001) | (I_cmd_type[3:0] == 4'b0100)) 
                                    begin    //如果是写使能指令(Write Enable)或者写不使能指令(Write Disable)
                                        O_qspi_state    <=  C_FINISH_DONE   ;
                                    end                          
                                else if (I_cmd_type[3:0] == 4'b0011) 
                                    begin    //如果是读状态寄存器指令(Read Register)
                                        O_qspi_state        <=  C_READ_WAIT ;
                                        R_write_bits_cnt    <=  7           ;
                                        R_read_bytes_num    <=  1           ;//读状态寄存器指令需要接收一个数据 
                                    end                             
                                else if( (I_cmd_type[3:0] == 4'b0010) ||  // 如果是扇区擦除(Sector Erase)
                                         (I_cmd_type[3:0] == 4'b0101) ||  // 如果是页编程指令(Page Program)
                                         (I_cmd_type[3:0] == 4'b0111) ||  // 如果是读数据指令(Read Data)
                                         (I_cmd_type[3:0] == 4'b0000) ||  // 如果是读设备ID指令(Read Device ID)
                                         (I_cmd_type[3:0] == 4'b1000) ||  // 如果是四线模式页编程指令(Quad Page Program)
                                         (I_cmd_type[3:0] == 4'b1001)     // 如果是四线模式读数据指令(Quad Read Data)
                                        ) 
                                    begin                          
                                        O_qspi_state        <=  C_SEND_ADDR ;
                                        R_write_bits_cnt    <=  23          ; // 这几条指令后面都需要跟一个24-bit的地址码
                                    end
                                else if (I_cmd_type[3:0] == 4'b0110) 
                                    begin    //如果是Write Status Register
                                        O_qspi_state        <=  C_WRITE_STATE_REG   ;
                                        R_write_bits_cnt    <=  15                  ;
                                    end 
                            end
                    end
                C_WRITE_STATE_REG   :
                    begin
                        R_qspi_io0_out_en   <=  1'b1    ;   // 设置IO0为输出
                        if(R_write_bits_cnt > 0)  
                            begin                           //如果R_cmd_reg还没有发送完
                                R_qspi_io0         <=  R_status_reg[R_write_bits_cnt] ;   //发送bit15~bit1位
                                R_write_bits_cnt   <=  R_write_bits_cnt    -   1      ;    
                            end                                 
                        else 
                            begin                                        //发送bit0
                                R_qspi_io0      <=  R_status_reg[0]    ;   
                                O_qspi_state    <=  C_FINISH_DONE      ;                                          
                            end                            
                    end 
                C_SEND_ADDR: // 发送地址状态
                    begin
                        R_qspi_io0_out_en   <=  1'b1    ;
                        if(R_write_bits_cnt > 0)  //如果R_cmd_reg还没有发送完
                            begin                                 
                                R_qspi_io0            <=  R_address_reg[R_write_bits_cnt] ; //发送bit23~bit1位
                                R_write_bits_cnt       <=  R_write_bits_cnt    -   1       ;    
                            end                                 
                        else 
                            begin 
                                R_qspi_io0 <=  R_address_reg[0]    ;   //发送bit0
                                if(I_cmd_type[3:0] == 4'b0010) // 扇区擦除(Sector Erase)指令
                                    begin  //扇区擦除(Sector Erase)指令发完24-bit地址码就执行结束了，所以直接跳到结束状态
                                        O_qspi_state <= C_FINISH_DONE   ;    
                                    end
                                else if (I_cmd_type[3:0] == 4'b0101) // 页编程(Page Program)指令,页编程指令和写数据指令是一个意思
                                    begin                              
                                        O_qspi_state        <=  C_WRITE_DATA    ;
                                        R_write_bits_cnt    <=  7               ;                       
                                    end
                                else if (I_cmd_type[3:0] == 4'b0000) // 读Device ID指令
                                    begin             
                                        O_qspi_state        <=  C_READ_WAIT     ;
                                        R_read_bytes_num    <=  2               ; //接收2个数据的Device ID
                                    end                                                         
                                else if (I_cmd_type[3:0] == 4'b0111) // 读数据(Read Data)指令
                                    begin
                                        O_qspi_state        <=  C_READ_WAIT     ;
                                        R_read_bytes_num    <=  256             ;   //接收256个数据        
                                    end 
                                else if (I_cmd_type[3:0] == 4'b1000) 
                                    begin   //如果是四线模式页编程指令(Quad Page Program)                               
                                        O_qspi_state        <=  C_WRITE_DATA_QUAD   ;
                                        R_write_bits_cnt    <=  7                   ;                       
                                    end 
                                else if (I_cmd_type[3:0] == 4'b1001) 
                                    begin   //如果是四线读操作                               
                                        O_qspi_state        <=  C_DUMMY         ;
                                        R_read_bytes_num    <=  256             ; //接收256个数据    
                                        R_write_bits_cnt    <=  7               ;                      
                                    end 
                            end
                    end 
                C_DUMMY:  // 四线读操作之前需要等待8个dummy clock
                    begin  
                        R_qspi_io3_out_en   <=  1'b0            ; // 设置IO_qspi_io3为高阻
                        R_qspi_io2_out_en   <=  1'b0            ; // 设置IO_qspi_io2为高阻
                        R_qspi_io1_out_en   <=  1'b0            ; // 设置IO_qspi_io1为高阻
                        R_qspi_io0_out_en   <=  1'b0            ; // 设置IO_qspi_io0为高阻       
                        if(R_write_bits_cnt > 0)    
                            R_write_bits_cnt    <=  R_write_bits_cnt - 1 ;                                    
                        else 
                            O_qspi_state        <=  C_READ_WAIT_QUAD     ;                                          
                    end   
                C_READ_WAIT: // 单线模式读等待状态
                    begin
                        if(R_read_finish)  
                            begin
                                O_qspi_state        <=  C_FINISH_DONE   ;
                                R_data_come_single  <=  1'b0            ;
                            end
                        else
                            begin
                                R_data_come_single  <=  1'b1            ; // 单线模式下读数据标志信号，此信号为高标志正在接收数据
                                R_qspi_io1_out_en   <=  1'b0            ;
                            end
                    end
                C_READ_WAIT_QUAD: // 四线模式读等待状态
                    begin
                        if(R_read_finish)  
                            begin
                                O_qspi_state        <=  C_FINISH_DONE   ;
                                R_data_come_quad    <=  1'b0            ;
                            end
                        else
                            R_data_come_quad        <=  1'b1            ;
                    end
                C_WRITE_DATA: // 写数据状态
                    begin
                        if(R_write_bytes_cnt < 256) // 往QSPI Flash中写入 256个数据
                            begin                       
                                if(R_write_bits_cnt > 0) //如果数据还没有发送完
                                    begin                           
                                        R_qspi_io0             <=  W_rom_out[R_write_bits_cnt] ; //发送bit7~bit1位
                                        R_write_bits_cnt    <=  R_write_bits_cnt  - 1'b1    ;                        
                                    end                 
                                else 
                                    begin                                 
                                        R_qspi_io0             <=  W_rom_out[0]                ; //发送bit0
                                        R_write_bits_cnt    <=  7                           ;
                                        R_write_bytes_cnt   <=  R_write_bytes_cnt + 1'b1    ;
                                    end
                            end
                        else 
                            begin
                                O_qspi_state    <=  C_FINISH_DONE   ;
                                R_qspi_clk_en   <=  1'b0            ;
                            end
                    end
                C_WRITE_DATA_QUAD    :
                    begin
                        R_qspi_io0_out_en   <=  1'b1    ;   // 设置IO0为输出
                        R_qspi_io1_out_en   <=  1'b1    ;   // 设置IO1为输出
                        R_qspi_io2_out_en   <=  1'b1    ;   // 设置IO2为输出
                        R_qspi_io3_out_en   <=  1'b1    ;   // 设置IO3为输出                          
                        if(R_write_bytes_cnt == 9'd256)
                            begin
                                O_qspi_state   <=  C_FINISH_DONE    ;    
                                R_qspi_clk_en  <=  1'b0             ; 
                            end 
                        else
                            begin      
                                if(R_write_bits_cnt == 8'd3)
                                    begin
                                        R_write_bytes_cnt   <=  R_write_bytes_cnt + 1'b1         ;
                                        R_write_bits_cnt    <=  8'd7                             ;
                                        R_qspi_io3          <=  W_rom_out[3]                     ; // 分别发送bit3
                                        R_qspi_io2          <=  W_rom_out[2]                     ; // 分别发送bit2
                                        R_qspi_io1          <=  W_rom_out[1]                     ; // 分别发送bit1
                                        R_qspi_io0          <=  W_rom_out[0]                     ; // 分别发送bit0
                                    end
                                else
                                    begin
                                        R_write_bits_cnt    <=  R_write_bits_cnt - 4            ;
                                        R_qspi_io3          <=  W_rom_out[R_write_bits_cnt - 0] ; // 分别发送bit7
                                        R_qspi_io2          <=  W_rom_out[R_write_bits_cnt - 1] ; // 分别发送bit6
                                        R_qspi_io1          <=  W_rom_out[R_write_bits_cnt - 2] ; // 分别发送bit5
                                        R_qspi_io0          <=  W_rom_out[R_write_bits_cnt - 3] ; // 分别发送bit4
                                    end 
                            end                                            
                    end 
                C_FINISH_DONE:
                    begin
                        O_qspi_cs           <=  1'b1    ;
                        R_qspi_io0             <=  1'b0    ;
                        R_qspi_clk_en       <=  1'b0    ;
                        O_done_sig          <=  1'b1    ;
                        R_qspi_io3_out_en   <=  1'b0    ; // 设置IO_qspi_io3为高阻
                        R_qspi_io2_out_en   <=  1'b0    ; // 设置IO_qspi_io2为高阻
                        R_qspi_io1_out_en   <=  1'b0    ; // 设置IO_qspi_io1为高阻
                        R_qspi_io0_out_en   <=  1'b0    ; // 设置IO_qspi_io0为高阻
                        R_data_come_single  <=  1'b0    ;
                        R_data_come_quad    <=  1'b0    ;
                        O_qspi_state        <=  C_IDLE  ;
                    end
                default:O_qspi_state    <=  C_IDLE      ;
            endcase         
        end
end

//////////////////////////////////////////////////////////////////////////////
// 功能：接收QSPI Flash发送过来的数据    
//////////////////////////////////////////////////////////////////////////////
always @(posedge I_clk_25M)
begin
    if(!I_rst_n)
        begin
            R_read_bytes_cnt    <=  0       ;
            R_read_bits_cnt     <=  0       ;
            R_read_finish       <=  1'b0    ;
            O_read_byte_valid   <=  1'b0    ;
            R_read_data_reg     <=  0       ;
            O_read_data         <=  0       ;
        end
    else if(R_data_come_single)   // 此信号为高表示接收数据从QSPI Flash发过来的数据
        begin
            if(R_read_bytes_cnt < R_read_bytes_num) 
                begin            
                    if(R_read_bits_cnt < 7)  //接收一个Byte的bit0~bit6    
                        begin                         
                            O_read_byte_valid   <=  1'b0                               ;
                            R_read_data_reg     <=  {R_read_data_reg[6:0],IO_qspi_io1} ;
                            R_read_bits_cnt     <=  R_read_bits_cnt +   1'b1           ;
                        end
                    else  
                        begin
                            O_read_byte_valid   <=  1'b1                               ;  //一个byte数据有效
                            O_read_data         <=  {R_read_data_reg[6:0],IO_qspi_io1} ;  //接收bit7
                            R_read_bits_cnt     <=  0                                  ;
                            R_read_bytes_cnt    <=  R_read_bytes_cnt    +   1'b1       ;
                        end
                end                               
            else 
                begin 
                    R_read_bytes_cnt    <=  0       ;
                    R_read_finish       <=  1'b1    ;
                    O_read_byte_valid   <=  1'b0    ;
                end
        end 
    else if(R_data_come_quad)   
        begin
            if(R_read_bytes_cnt < R_read_bytes_num) 
                begin  //接收数据              
                    if(R_read_bits_cnt < 8'd1)
                        begin
                            O_read_byte_valid       <=  1'b0                    ;
                            R_read_data_reg         <=  {R_read_data_reg[3:0],IO_qspi_io3,IO_qspi_io2,IO_qspi_io1,IO_qspi_io0};//接收前四位
                            R_read_bits_cnt         <=  R_read_bits_cnt + 1     ; 
                        end
                    else    
                        begin
                            O_read_byte_valid       <=  1'b1                    ;
                            O_read_data             <=  {R_read_data_reg[3:0],IO_qspi_io3,IO_qspi_io2,IO_qspi_io1,IO_qspi_io0};  //接收后四位
                            R_read_bits_cnt         <=  0                       ;
                            R_read_bytes_cnt        <=  R_read_bytes_cnt + 1'b1 ;     
                        end
                end                               
            else 
                begin 
                    R_read_bytes_cnt    <=  0       ;
                    R_read_finish       <=  1'b1    ;
                    O_read_byte_valid   <=  1'b0    ;
                end
        end
    else 
        begin
            R_read_bytes_cnt    <=  0       ;
            R_read_bits_cnt     <=  0       ;
            R_read_finish       <=  1'b0    ;
            O_read_byte_valid   <=  1'b0    ;
            R_read_data_reg     <=  0       ;
        end
end         

rom_data rom_data_inst (
  .clka(I_clk_25M), // input clka
  .addra(W_rom_addr), // input [7 : 0] addra
  .douta(W_rom_out) // output [7 : 0] douta
);

endmodule
```

顶层测试状态机的完整代码如下：

```verilog
module qspi_top
(
    input         I_clk         ,
    input         I_rst_n       ,

    output        O_qspi_clk    , // QPI总线串行时钟线
    output        O_qspi_cs     , // QPI总线片选信号
    inout         IO_qspi_io0   , // QPI总线输入/输出信号线
    inout         IO_qspi_io1    , // QPI总线输入/输出信号线
    inout         IO_qspi_io2    , // QPI总线输入/输出信号线
    inout         IO_qspi_io3      // QPI总线输入/输出信号线  
);
     
reg [3:0]   R_state             ;
reg [7:0]   R_flash_cmd         ;
reg [23:0]  R_flash_addr        ;
reg         R_clk_25M           ;
reg [4:0]   R_cmd_type          ;
reg    [15:0]    R_status_reg        ;
                                
wire        W_done_sig          ;
wire [7:0]  W_read_data         ;
wire        W_read_byte_valid   ;
wire [2:0]  R_qspi_state        ;


////////////////////////////////////////////////////////////////////          
//功能：二分频逻辑          
////////////////////////////////////////////////////////////////////          
always @(posedge I_clk or negedge I_rst_n)
begin
    if(!I_rst_n) 
        R_clk_25M   <=  1'b0        ;
    else 
        R_clk_25M   <=  ~R_clk_25M  ;
end
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//功能：测试状态机
////////////////////////////////////////////////////////////////////
always @(posedge R_clk_25M or negedge I_rst_n)
begin
    if(!I_rst_n) 
        begin
            R_state         <=  4'd0        ;
            R_flash_addr    <=  24'd0       ;
            R_flash_cmd     <=  8'h00       ;
            R_cmd_type      <=  5'b0_0000   ;
        end
     else 
        begin
            case(R_state)           
                4'd0://读Device ID指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd  <= 8'h90           ; 
                                R_flash_addr <= 24'd0           ; 
                                R_cmd_type   <= 5'b1_0000       ; 
                            end     
                    end 
                4'd1://写不使能(Write disable)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ;
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h04            ; 
                                R_cmd_type  <= 5'b1_0100        ; 
                            end     
                    end                
                4'd2://写使能(Write Enable)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin
                                R_flash_cmd <= 8'h06            ; 
                                R_cmd_type  <= 5'b1_0001        ; 
                            end
                    end         
                4'd3:// 扇区擦除(Sector Erase)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h20            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_0010        ; 
                            end
                    end
            
                4'd4://读状态寄存器1, 当Busy位(状态寄存器1的最低位)为0时表示擦除操作完成
                    begin
                        if(W_done_sig) 
                            begin 
                                if(W_read_data[0]==1'b0) 
                                    begin 
                                        R_flash_cmd <= 8'h00            ; 
                                        R_state     <= R_state + 1'b1   ;
                                        R_cmd_type  <= 5'b0_0000        ; 
                                    end
                                else 
                                    begin 
                                        R_flash_cmd <= 8'h05        ; 
                                        R_cmd_type  <= 5'b1_0011    ; 
                                    end
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h05        ; 
                                R_cmd_type  <= 5'b1_0011    ; 
                            end
                    end
                4'd5://写状态寄存器2(Write Status Register2)指令，用来把QE(Quad Enable)位置1
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin
                                R_flash_cmd <= 8'h01            ; 
                                R_cmd_type  <= 5'b1_0110        ; 
                                R_status_reg<= 16'h0002            ;
                            end
                    end 
                4'd6://写使能(Write Enable)指令
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin
                                R_flash_cmd <= 8'h06            ; 
                                R_cmd_type  <= 5'b1_0001        ; 
                            end
                    end             
                4'd7: //四线模式页编程操作(Quad Page Program): 把存放在ROM中的数据写入QSPI Flash中
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ; 
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h32            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_1000        ; 
                            end
                    end
                4'd8://读状态寄存器1, 当Busy位(状态寄存器1的最低位)为0时表示写操作完成
                    begin
                        if(W_done_sig) 
                            begin 
                                if(W_read_data[0]==1'b0) 
                                    begin 
                                        R_flash_cmd <= 8'h00            ; 
                                        R_state     <= R_state + 1'b1   ;
                                        R_cmd_type  <= 5'b0_0000        ; 
                                    end
                                else 
                                    begin 
                                        R_flash_cmd <= 8'h05        ; 
                                        R_cmd_type  <= 5'b1_0011    ; 
                                    end
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h05        ; 
                                R_cmd_type  <= 5'b1_0011    ; 
                            end
                    end           
                4'd9://四线模式读256 Bytes数据
                    begin
                        if(W_done_sig) 
                            begin 
                                R_flash_cmd <= 8'h00            ; 
                                R_state     <= R_state + 1'b1   ;
                                R_cmd_type  <= 5'b0_0000        ; 
                            end
                        else 
                            begin 
                                R_flash_cmd <= 8'h6b            ; 
                                R_flash_addr<= 24'd0            ; 
                                R_cmd_type  <= 5'b1_1001        ; 
                            end
                    end
            
                4'd10:// 结束状态
                    begin
                        R_flash_cmd <= 8'h00            ; 
                        R_state     <= 4'd10            ;
                        R_cmd_type  <= 5'b0_0000        ; 
                    end
                default :   R_state     <= 4'd0         ;
            endcase
        end           
end 
qspi_driver U_qspi_driver
(
.O_qspi_clk          (O_qspi_clk        ), // QPI总线串行时钟线
.O_qspi_cs           (O_qspi_cs         ), // QPI总线片选信号
.IO_qspi_io0         (IO_qspi_io0       ), // QPI总线输入/输出信号线
.IO_qspi_io1         (IO_qspi_io1       ), // QPI总线输入/输出信号线
.IO_qspi_io2         (IO_qspi_io2       ), // QPI总线输入/输出信号线
.IO_qspi_io3         (IO_qspi_io3       ), // QPI总线输入/输出信号线
                   
.I_rst_n             (I_rst_n           ), // 复位信号

.I_clk_25M           (R_clk_25M         ), // 25MHz时钟信号
.I_cmd_type          (R_cmd_type        ), // 命令类型
.I_cmd_code          (R_flash_cmd       ), // 命令码
.I_qspi_addr         (R_flash_addr      ), // QSPI Flash地址
.I_status_reg         (R_status_reg        ), // QSPI Flash状态寄存器

.O_done_sig          (W_done_sig        ), // 指令执行结束标志
.O_read_data         (W_read_data       ), // 从QSPI Flash读出的数据
.O_read_byte_valid   (W_read_byte_valid ), // 读一个字节完成的标志
.O_qspi_state        (R_qspi_state      )  // 状态机，用于在顶层调试用
);
     
wire [35:0]     CONTROL0    ;
wire [99:0]     TRIG0       ;
icon icon_inst (
    .CONTROL0(CONTROL0) // INOUT BUS [35:0]
);

ila ila_inst (
    .CONTROL(CONTROL0)  , // INOUT BUS [35:0]
    .CLK(I_clk)           ,      // IN
    .TRIG0(TRIG0)      // IN BUS [255:0]
);                                                     

assign  TRIG0[7:0]      =   W_read_data         ;                                               
assign  TRIG0[8]        =   W_read_byte_valid   ;   
assign  TRIG0[12:9]     =   R_state             ;        
assign  TRIG0[16:13]    =   R_qspi_state        ;   
assign  TRIG0[17]       =   W_done_sig          ;    
assign  TRIG0[18]       =   IO_qspi_io0         ;  
assign  TRIG0[19]       =   IO_qspi_io1         ;  
assign  TRIG0[20]       =   IO_qspi_io2         ;  
assign  TRIG0[21]       =   IO_qspi_io3         ;  
assign  TRIG0[22]       =   O_qspi_cs           ;  
assign  TRIG0[23]       =   O_qspi_clk          ; 
assign  TRIG0[28:24]    =   R_cmd_type          ; 
assign  TRIG0[36:29]    =   R_flash_cmd         ; 
assign  TRIG0[60:37]    =   R_flash_addr        ; 
assign  TRIG0[61]       =   I_rst_n             ; 
assign  TRIG0[77:62]    =   R_status_reg        ; 


endmodule
```

接下来就对比一下各个指令的时序图，由于有一部分时序图在单线模式已经对比过了，这里只对比写状态寄存器，四线读操作，四线写操作三个指令的时序图

1、写写状态寄存器(Write Status Register)指令

​    芯片手册写状态寄存器时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155109823-996673963.png)

​    ChipScope抓出来的写状态寄存器时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155126558-778135305.png)

　　2、四线写数据(Quad Input Page Program)指令

​    芯片手册四线写数据时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155217033-1423108223.png)

​    ChipScope抓回来的四线写数据时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155242588-1172050356.png)

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155300958-654990319.png)

 

3、四线读数据(Quad Read Data)指令

​    芯片手册四线读数据(Fast Read Quad Output)时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155329409-1174109856.png)

ChipScope抓回来的四线读数据(Fast Read Quad Output)时序图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155350818-282114597.png)

 

![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155408793-138070872.png)

 

　　通过对比各个指令与芯片手册的指令，可以发现各个指令的时序与芯片手册完全吻合，至此四线SPI操作QSPI  Flash的代码全部测试通过。你可以把ROM里面的数据换成你自己的数据发给QSPI  Flash，ROM里面数据则可以通过4.2小节的Matlab自定义产生。

### 4.6、 四线模式与单线模式读写数据时的性能对比

由于QSPI  Flash四线模式中共有四根信号线，它们全部可以用做输入输出来传输数据，所以传输同样大小的数据块时候，四线模式的速度是单线模式的四倍。下面列出我在最初调试QSPI Flash的时候抓出来的两张图，分别是四线模式与单线模式写数据对比图与 四线模式与单线模式读数据对比图

　　四线模式与单线模式写数据对比图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155500799-2130984191.png)

　　四线模式与单线模式读数据对比图：

 ![img](https://img2018.cnblogs.com/blog/1426240/201809/1426240-20180915155523636-1191061247.png)

　　从上面可以很清晰的看出读写同一块数据的时候，四线模式相较于单线模式而言，速度有了4倍的提升。

## 五、 进一步思考

### 5.1、 用Verilog编写QSPI Flash驱动的意义何在？

　　事实上，Xilinx的FPGA的已经有专门的引脚用来与QSPI  Flash相连，当我们把.bin文件或者.mcs文件通过Jtag固化到QSPI  Flash中的时候，ISE软件中的iMPACT会自动把.bin文件或者.mcs文件以SPI协议的格式写入到QSPI Flash中，由于QSPI  Flash是一种非易失性存储器，所以断电后，数据并不会丢失，当FPGA断电重启以后，它会自动从QSPI Flash中加载事先固化到QSPI  Flash里面的程序到内部的RAM中执行，而不像.bit掉电程序就会丢失。既然FPGA已经支持QSPI Flash的协议，为什么还要写QSPI  Flash的驱动呢？原因是在实际的项目中，如果要远程对FPGA的代码进行在线升级的话就必须有一套QSPI  Flash的驱动来配合CPU进行升级操作。

　　比如，某厂家生产了几千台4G基站，而且已经发货到国外各个国家商用。而后期研发人员调试的时候发现FPGA部分有一个小Bug需要修复或者说需要增加一个新功能，代码写好了以后如果没有在线升级的功能，你只能一个一个去用jtag重新固化程序，这样会耗费大量的人力成本而且极其不方便，而如果有在线升级的功能只需要通过远程登录连接到CPU上发送几条指令就可以完成升级功能，大大提高了工作效率。

### 5.2、 关于在代码中同时使用时钟的上升沿和下降沿操作时有什么风险

　　关于这个问题目前还没有找到比较权威的说法，以下两个说法是提的最多的：

　　1、FPGA内部的触发器都是上升沿触发的，所以，如果在代码中用下降沿触发的话会在时钟引脚的前面综合出一个反相器，这个反相器可能会对时钟信号的质量有影响

　　2、集成电路设计的书中有对晶振的描述，外部晶振产生的时钟信号上升沿的时间几乎是一样的，但是每个周期的下降沿的时间却无法保证完全一致，这与工艺有很大关系，所以采用下降沿触发有可能存在风险。

　　希望有这方面经验的网友能提供一些权威的说法。