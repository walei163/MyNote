# FUXI核心板GPMC总线问题

2025-02-17：

菲森反馈采用我司SWAS831核心板，GPMC总线读写不正常，读出的数据全为0的现象。

经过测试问题查找，发现是核心板的硬件设计上有误，将GPCM_OE_B和GPCM_WE_B这两根信号线连反了。将这两根线交换过来，测试读写就正常了。

我是在万力达的DTU2660的底板上测试的，这个底板上GPMC总线连接到了FPGA里，因此可以通过修改FPGA代码中的读写信号线的定义来修改。

FUXI核心板GPMC这块，需要修改DTB中的定义：

```shell
gpmc: gpmc@a1d00000 {
	compatible = "fuxi,gpmc";
	reg = <0xa1d00000 0x1000>;
	clocks = <&dummy_apb>;
	clock-names = "gpmc";
	pinctrl-names = "default";
	pinctrl-0 = <&gpmc_pins>;
	status = "disabled";
};

...

gpmc_pins: pinctrl_gpmc_pins {
	pinctrl-single,bits = <
		0x50 0x00000001 0x0000000f	/* GPMC_D0 */
		0x50 0x00000010 0x000000f0	/* GPMC_D1 */
		0x50 0x00000100 0x00000f00	/* GPMC_D2 */ 
		0x50 0x00001000 0x0000f000	/* GPMC_D3 */
		0x50 0x00010000 0x000f0000	/* GPMC_D4 */
		0x50 0x00100000 0x00f00000	/* GPMC_D5 */
		0x50 0x01000000 0x0f000000	/* GPMC_D6 */
		0x50 0x10000000 0xf0000000	/* GPMC_D7 */
		
		0x54 0x00000001 0x0000000f	/* GPMC_D8 */
		0x54 0x00000010 0x000000f0	/* GPMC_D9 */
		0x54 0x00000100 0x00000f00	/* GPMC_D10 */
		0x54 0x00001000 0x0000f000	/* GPMC_D11 */
		0x54 0x00010000 0x000f0000	/* GPMC_D12 */
		0x54 0x00100000 0x00f00000	/* GPMC_D13 */
		0x54 0x01000000 0x0f000000	/* GPMC_D14 */
		0x54 0x10000000 0xf0000000	/* GPMC_D15 */
	
		0x58 0x00000001 0x0000000f  /* GPMC_A0 */
		0x58 0x00000010 0x000000f0  /* GPMC_A1 */
		0x58 0x00000100 0x00000f00  /* GPMC_A2 */
		0x58 0x00001000 0x0000f000  /* GPMC_A3 */
		0x58 0x00010000 0x000f0000  /* GPMC_A4 */
		0x58 0x00100000 0x00f00000  /* GPMC_A5 */
		0x58 0x01000000 0x0f000000  /* GPMC_A6 */
		0x58 0x10000000 0xf0000000  /* GPMC_A7 */
		
		0x5C 0x00000001 0x0000000f	/* GPMC_A8 */	
		// 0x5C 0x00000010 0x000000f0	/* GPMC_A9 */	
		// 0x5C 0x00000100 0x00000f00	/* GPMC_A10 */	
		// 0x5C 0x00001000 0x0000f000	/* GPMC_A11 */	
		// 0x5C 0x00010000 0x000f0000	/* GPMC_A12 */	
		// 0x5C 0x00100000 0x00f00000	/* GPMC_A13 */	
		// 0x5C 0x01000000 0x0f000000	/* GPMC_A14 */	
		// 0x5C 0x10000000 0xf0000000	/* GPMC_A15 */	
		
		0x60 0x00000001 0x0000000f	/* GPMC_WE_B */	
		0x60 0x00000010 0x000000f0	/* GPMC_OE_B */	
		0x60 0x00000100 0x00000f00	/* GPMC_BS0_B */	
		0x60 0x00001000 0x0000f000	/* GPMC_BS1_B */	
		0x60 0x00010000 0x000f0000	/* GPMC_CS0_B */	
		0x60 0x00100000 0x00f00000	/* GPMC_CS1_B */	
		// 0x60 0x01000000 0x0f000000	/* GPMC_CS2_B */	
		// 0x60 0x10000000 0xf0000000	/* GPMC_CS3_B */	
		
		// 0x64 0x00000001 0x0000000f	/* GPMC_CLK */	
		// 0x64 0x00000010 0x000000f0	/* GPMC_D16 */	
		// 0x64 0x00000100 0x00000f00	/* GPMC_D17 */	
		// 0x64 0x00001000 0x0000f000	/* GPMC_D18 */	
		// 0x64 0x00010000 0x000f0000	/* GPMC_D19 */	
		// 0x64 0x00100000 0x00f00000	/* GPMC_D20 */	
		// 0x64 0x01000000 0x0f000000	/* GPMC_D21 */	
		// 0x64 0x10000000 0xf0000000	/* GPMC_D22 */	
		// 0x68 0x00000001 0x0000000f	/* GPMC_D23 */	
		// 0x68 0x00000010 0x000000f0	/* GPMC_D24 */	
		// 0x68 0x00000100 0x00000f00	/* GPMC_D25 */	
		// 0x68 0x00001000 0x0000f000	/* GPMC_D26 */	
		// 0x68 0x00010000 0x000f0000	/* GPMC_D27 */	
		// 0x68 0x00100000 0x00f00000	/* GPMC_D28 */	
		// 0x68 0x01000000 0x0f000000	/* GPMC_D29 */	
		// 0x68 0x10000000 0xf0000000	/* GPMC_D30 */	
		// 
		// 0x6C 0x00000001 0x0000000f	/* GPMC_D31 */	
		// 0x6C 0x00000010 0x000000f0	/* GPMC_A16 */	
		// 0x6C 0x00000100 0x00000f00	/* GPMC_A17 */	
		// 0x6C 0x00001000 0x0000f000	/* GPMC_A18 */	
		// 0x6C 0x00010000 0x000f0000	/* GPMC_A19 */	
		// 0x6C 0x00100000 0x00f00000	/* GPMC_A20 */	
		// 0x6C 0x01000000 0x0f000000	/* GPMC_A21 */	
		// 0x6C 0x10000000 0xf0000000	/* GPMC_A22 */	
		// 
		// 0x70 0x00000001 0x0000000f	/* GPMC_A23 */	
		// 0x70 0x00000010 0x000000f0	/* GPMC_A24 */	
		// 0x70 0x00000100 0x00000f00	/* GPMC_A25 */	
		// 0x70 0x00001000 0x0000f000	/* GPMC_A26 */	
		// 0x70 0x00010000 0x000f0000	/* GPMC_A27 */	
		// 0x70 0x00100000 0x00f00000	/* GPMC_A28 */	
		// 0x70 0x01000000 0x0f000000	/* GPMC_A29 */	
		// 0x70 0x10000000 0xf0000000	/* GPMC_A30 */	
		// 0x74 0x00000001 0x0000000f	/* GPMC_A31 */	
		// 0x74 0x00000010 0x000000f0	/* GPMC_BS2_B */	
		// 0x74 0x00000100 0x00000f00	/* GPMC_BS3_B */	
	>;
};

...

&gpmc {
	status = "okay";
	#address-cells = <1>;
	#size-cells = <1>;
	gpmc,num-cs = <4>;
	ranges = <0x0 0xA1A00000 0x10000>;

	sram0@0 {

		// interrupt-parent = <&port5>;
		// interrupts = <9 IRQ_TYPE_EDGE_RISING>;
		// lcd-reset-gpios = <&port4 30 GPIO_ACTIVE_HIGH>;
		// lcd-bl-en-gpios = <&port4 31 GPIO_ACTIVE_LOW>;

		compatible = "gpmc,sram0";
		#address-cells = <1>;
		#size-cells = <1>;
		gpmc,cs = <0>;
		gpmc,reg_sel = <0>;
		gpmc,mem_type ="sram";
		gpmc,width = <16>;
		reg = <0 0x10000>;	//0x10000: 65535, 64KB

		gpmc,t_rc = <19>;	//读取周期时间：0~63
		gpmc,t_as = <1>;	//地址建立时间：0~3
		gpmc,t_wr = <1>;	//写入地址/数据保持时间：0~3
		gpmc,t_wp = <50>;	//写入脉冲宽度：0~63
		gpmc,t_bta = <6>;	//总线周期读到写或者写到读的空闲时间：0~7
	};
};
```

然后可以编写一个mmap内存映射的程序来测试一下：

```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <pthread.h>


#define MMAP_LENGTH		0x10000	
int mmap_addr = 0xa1a00000;

int main (int argc, char *argv[])
{

	int fd = 0;
	int i;
	volatile void *mmap_start = NULL;
	volatile unsigned char *mmap_start8 = NULL;
	volatile unsigned short *mmap_start16 = NULL;
	unsigned char id[4];
	unsigned short val = 0;
	unsigned int reg = 0;
	int bits = 0;
	int cs;
	unsigned char arg[] = {'a', 'b', 'c'};

	fd = open("/dev/mem", O_RDWR,0); 
	if(fd < 0){
		printf("--Open /dev/mem failure.\n");
		exit(1);
	}
	
	if(argc > 3 && argc < 6 && (!strncmp("r", argv[1], 1) || !strncmp("w", argv[1], 1))){
		cs = atoi(argv[2]);
		bits = atoi(argv[1] + 1);
		if((bits != 8) && (bits != 16)){
			printf(
			"Usage.\n\
                          Read:  cpld r(8/16) cs reg\n\
                          Write: cpld w(8/16) cs reg val\n");			
			return -1;
		}
		reg=(unsigned int)strtoul(argv[3], 0, 0);
		if(argc == 5)
			val=(unsigned short)strtoul(argv[4], 0, 0);
	}
	else {
		printf(
			"Usage.\n\
			  Read:  cpld r(8/16) cs reg\n\
			  Write: cpld w(8/16) cs reg val\n");
		return -1;
	}
	
	if(bits == 8){
		mmap_start8 =(unsigned char *)mmap(NULL, MMAP_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, mmap_addr);
		mmap_start = mmap_start8;
	}
	if(bits == 16){
		mmap_start16 =(unsigned short *)mmap(NULL, MMAP_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, mmap_addr);
		mmap_start = mmap_start16;
	}

	if(mmap_start == MAP_FAILED){
		printf("--Map memory [0x%08x]failure. cs = %d\n", mmap_addr, cs);
		exit(1);
	} 

	printf("--Map memory success!\n");
	printf("Physical addreess: 0x%x, mmap address: 0x%p\n", mmap_addr, (bits == 8) ? mmap_start8 : mmap_start16);
	
	i = 0;

	if(!strncmp("r", argv[1], 1)){
		printf("read cs%d reg:0x%x\n", cs, reg);
		while(i < 100){
			if(bits == 8)
				val = *(mmap_start8 + reg + i);
			else
				val = *(mmap_start16 + reg + i);

			if(bits == 8)
				printf("0x%02x   ", val);
			else
				printf("0x%04x   ", val);

			if(((i+1) % 8) == 0)
				printf("\n");
			i++;
		}
		printf("\n");
	}
	else if(!strncmp("w", argv[1], 1)){
		if(bits == 8)
			*(mmap_start8 + reg) = val;
		else
			*(mmap_start16 + reg) = val;
		printf("write cs%d reg[%08x] : %04x\n", cs, reg, val);
	}
	
	munmap((void *)mmap_start, MMAP_LENGTH);
	close(fd);
	return (0);
}	
```

