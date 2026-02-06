# 直接采样SPI_SCLK的逻辑代码实现

---

问题：

> [!note]
>
> **之前在瑞星微的系统上，CPLD片内SPI Slave逻辑实现，是采用一个`100MHz`以上的高频时钟来采样`SPI_SCLK`，然后根据`SPI_SCLK`的上升沿或者下降沿来接收和发送SPI的数据。**

这样有一个缺点，FPGA的PLL产生的高频时钟，需要是`SPI_SCLK`4~8倍以上，才能正确的采样。在`SPI_SCLK`的频率较低时（10~20MHz），高频时钟还能正确采样（我的设计程序中默认设置为`120MHz`），如果`SPI_SCLK`的时钟很高（25~40MHz）的情况下，所需要的高频采样时钟则需要高达200MHz以上，此时FPGA片内的PLL，无法产生如此高的高频采样时钟，或者即使能产生，也不能正确工作。因此，程序设计上，最佳方案就是直接将`SPI_SCLK`作为SPI传输逻辑的时钟来使用。

但是这样设计的话，需要考虑几个方面的因素：

**1、需要同时设计时钟的上升沿和下降沿逻辑，也就是双沿之间的数据交换与数据同步。这个之前的设计中没有采用过这种模式。**

**2、`SPI_SCLK`并不是一个连续时钟，其只在`SPI_CS`信号为低时才会发出时钟信号。在AD采样逻辑中，不能直接作为`FIFO`的写时钟，还必须有一个`FIFO`写入时钟来同步将SPI转换出来的数据打入到`FIFO`中。**

经过几天的调试，现在终于将这种模式实现了。其源代码如下：

```verilog
//-----------------------------------------------------------------------------------------------------------
//      发送或者接收一帧spi数据，帧协议参照MCP2515的读写帧格式：
//      读一个字：
//	    0x03	地址字节	输出数据低字节  输出数据高字节
//      写一个字：
//	    0x02	地址字节	数据字节低     数据字节高
//	    用于操作FPGA片内的寄存器，一次操作一个字。
//-----------------------------------------------------------------------------------------------------------
module my_spi_slave_frame(
	sys_clk, reset_n, spi_clk, spi_mosi, spi_cs_n, d_in,
	spi_miso, d_out, spi_frame_cmd_over, spi_frame_over, spi_cmd, spi_address,
        d_in_high, d_out_high
);

input sys_clk;                   //系统时钟，为了捕捉spi_clk（最大50MHz），需要时钟在200MHz
input reset_n;
input spi_clk;
input spi_mosi;
input spi_cs_n;
input [7:0] d_in;		        //8位数据，即需要传输给CPU的并行数据，经过MISO串行输出至CPU。

output spi_miso;
output [7:0] d_out;		        //8位数据，通过接收CPU的MOSI的串行数据，并经过本模块串并转换解码后的8位并行数据
output spi_frame_cmd_over;	    //该信号表示SPI的指令帧已发送完毕
output spi_frame_over;		    //该信号表示SPI一帧已完成
output [7:0] spi_cmd;		    //从SPI帧中解出的SPI控制字。
output [7:0] spi_address;	    //从SPI帧中解出的SPI操作地址。

output [7:0] d_out_high;        //增加1个高字节数据输出。
input [7:0] d_in_high;

parameter SPI_FRAME_BITS_LENGTH = 32;	//定义SPI一帧的位总长度，可以在顶层修改。8位为1个字节，表示一帧协议有4个字节。

//-----------------------------------------------------------------------------------------------------------
//      捕捉spi_cs_n的下降沿和上升沿，作为SPI一帧的开始和结束标志
//-----------------------------------------------------------------------------------------------------------
reg [1:0] spi_ssel_reg;
wire spi_start_msg;
wire spi_end_msg/* systhesis syn_keep = 1 */;

always @(posedge sys_clk or negedge reset_n)
begin
        if(!reset_n)
                spi_ssel_reg <= 2'b0;
        else
                spi_ssel_reg <= {spi_ssel_reg[0], spi_cs_n};
end

assign spi_start_msg = (spi_ssel_reg == 2'b10);         //下降沿作为SPI传输一帧的开始
assign spi_end_msg   = (spi_ssel_reg == 2'b01);         //上升沿作为SPI传输一帧的结束

//-----------------------------------------------------------------------------------------------------------
//      通过MISO发送串行数据，并进行并串转换，当前SPI模式：CPOL=0，CPHA=0。
//      改为以SPI_CLK作为时钟来采样数据
//      SPI一帧协议有4个字节：命令，地址，数据低字节，数据高字节
//      注意！！！一定要上升沿采样，下降沿送数！
//-----------------------------------------------------------------------------------------------------------
`define BIT_COUNTER_WIDTH       6
reg [`BIT_COUNTER_WIDTH - 1:0] bit_counter;

//接收SPI数据时，是采样SPI_CLK的上升沿。同时当检测到SPI_CS_N上升沿时，一帧数据传输结束。
always @(posedge spi_clk or posedge spi_cs_n or negedge reset_n)
begin
        if(!reset_n) begin
                bit_counter <= `BIT_COUNTER_WIDTH'b0;
        end
        else if(spi_cs_n) begin
                bit_counter <= `BIT_COUNTER_WIDTH'b0;
        end                        
        else begin
                bit_counter <= bit_counter + `BIT_COUNTER_WIDTH'b1;
        end
end

wire spi_command_done   = (bit_counter == `BIT_COUNTER_WIDTH'd8);
wire spi_address_done   = (bit_counter == `BIT_COUNTER_WIDTH'd16);
wire spi_data_byte_done = (bit_counter == `BIT_COUNTER_WIDTH'd24);
wire spi_data_word_done = (bit_counter == `BIT_COUNTER_WIDTH'd32);

//-----------------------------------------------------------------------------------------------------------
//接收SPI数据并移位。同时当检测到SPI_CS_N上升沿时，一帧数据传输结束。
//-----------------------------------------------------------------------------------------------------------
reg [SPI_FRAME_BITS_LENGTH - 1:0] byte_recv_shift;

always @(posedge spi_clk or posedge spi_cs_n or negedge reset_n)
begin
        if(!reset_n) begin
                byte_recv_shift <= {SPI_FRAME_BITS_LENGTH{1'b0}};
        end
        else if(spi_cs_n) begin
                byte_recv_shift <= {SPI_FRAME_BITS_LENGTH{1'b0}};
        end                        
        else begin
                byte_recv_shift <= {byte_recv_shift[SPI_FRAME_BITS_LENGTH - 2:0], spi_mosi};
        end
end

//-----------------------------------------------------------------------------------------------------------
//SPI_SCLK的下降沿送数。
//-----------------------------------------------------------------------------------------------------------
reg [SPI_FRAME_BITS_LENGTH - 1:0] byte_reg;

always @(negedge spi_clk or negedge reset_n)
begin
        if(!reset_n)
                byte_reg <= {SPI_FRAME_BITS_LENGTH{1'b0}};
        else if(spi_command_done)
                byte_reg[31:24] <= byte_recv_shift[7:0];
        else if(spi_address_done)
                byte_reg[23:16] <= byte_recv_shift[7:0];
        else if(spi_data_byte_done)
                byte_reg[15:8] <= byte_recv_shift[7:0];
        else if(spi_data_word_done)
                byte_reg[7:0] <= byte_recv_shift[7:0];
end

assign spi_cmd            = byte_reg[SPI_FRAME_BITS_LENGTH - 1:24];
assign spi_address        = byte_reg[23:16];
assign d_out              = byte_reg[15:8];
assign d_out_high         = byte_reg[7:0];
assign spi_frame_cmd_over = (bit_send_counter == `BIT_COUNTER_WIDTH'd16);       //这个信号表明SPI的命令字节和地址字节已经接收完毕。
assign spi_frame_over     = spi_end_msg;		                        //这个信号表明SPI的一帧已经接收完毕

reg [SPI_FRAME_BITS_LENGTH - 1:0]  spi_send_data;
wire [15:0] spi_data_word = {d_in, d_in_high};

always @(negedge spi_clk or posedge spi_cs_n or negedge reset_n)
begin
        if(!reset_n)
                spi_send_data <= 'h55aa9988;
        else if(spi_cs_n)
                spi_send_data <= 'h55aa9988;
        else if(bit_send_counter == `BIT_COUNTER_WIDTH'd16)
                spi_send_data[15:0] <= spi_data_word;
end

//-----------------------------------------------------------------------------------------------------------
//发送SPI数据时，是采样SPI_CLK的下降沿。同时当检测到SPI_CS_N上升沿时，一帧数据传输结束。
//-----------------------------------------------------------------------------------------------------------  
reg [`BIT_COUNTER_WIDTH - 1:0] bit_send_counter;

always @(negedge spi_clk or posedge spi_cs_n or negedge reset_n)
begin
        if(!reset_n)
                bit_send_counter <= `BIT_COUNTER_WIDTH'b0;
        else if(spi_cs_n)
                bit_send_counter <= `BIT_COUNTER_WIDTH'b0;
        else
                bit_send_counter <= bit_send_counter + `BIT_COUNTER_WIDTH'b1;
end

//高位先出
assign spi_miso = (bit_send_counter == `BIT_COUNTER_WIDTH'd16) ? spi_data_word[15] : spi_send_data[SPI_FRAME_BITS_LENGTH - bit_send_counter - 1];

endmodule
```

> [!important]
>
> **这种方案实现时，一定要切记遵循以下几个原则：**
>
> - **`SPI_SCLK`时钟的上升沿只用来采样MOSI，下降沿只用于数据收发与转换。两者不能混淆。**
> - **`SPI_SCLK`时钟上升沿只用于数据接收，而下降沿只用于数据发送。**
> - **接收需要维护一个接收计数器，发送需要维护一个发送计数器，两者不能混用。**
> - **并串转换发送数据时，最高位需要先出，不能放在在时序逻辑中，而需要采用组合逻辑将最高位摘出来。如上面代码最后一行所描述：**
>
> ```verilog
> assign spi_miso = (bit_send_counter == `BIT_COUNTER_WIDTH'd16) ? spi_data_word[15] : spi_send_data[SPI_FRAME_BITS_LENGTH - bit_send_counter - 1];
> ```
>
> 