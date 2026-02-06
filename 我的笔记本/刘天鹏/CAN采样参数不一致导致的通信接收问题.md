# CAN采样参数不一致导致的通信接收问题

2024-8-15 长园反馈dca1600的发送消防可以收到，消防的发送1600无法收到，两者的报文都可被CAN盒抓到。降低波特率也无法解决。

长园消防设备的CAN的采样参数如下：

![image-20240821114512582](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/刘天鹏/vx_images/image-20240821114639407.png)

![image-20240821114639406](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/刘天鹏/vx_images/image-20240821114639406.png)

我司can的tq定值为41，根据 prop-seg ,phase-seg1 ,phase-seg2 做动态波特率设置的

```sh
针对250k做调制波特率逐一尝试：

24MHz时钟 250k

ip -details link show can0

ip link set can0 type can bitrate 250000

ip link set can0 type can tq 41 prop-seg 41 phase-seg1 42 phase-seg2 12 sjw 1 

ip link set can0 type can tq 41 prop-seg 40 phase-seg1 43 phase-seg2 12 sjw 1 

ip link set can0 type can tq 180 prop-seg 15 phase-seg1 6 phase-seg2 2 sjw 1 

ip link set can0 type can tq 150 prop-seg 15 phase-seg1 6 phase-seg2 2 sjw 1 

ip link set can0 type can tq 250 prop-seg 6 phase-seg1 7 phase-seg2 2 sjw 1 
针对100k做调制波特率逐一尝试：

ip link set can0 type can tq 41 prop-seg 104 phase-seg1 105 phase-seg2 30 sjw 1 
ip link set can0 type can tq 64 prop-seg 15 phase-seg1 6 phase-seg2 2 sjw 1 
ip link set can0 type can tq 625 prop-seg 6 phase-seg1 7 phase-seg2 2 sjw 1 
```

通过调试发现即使波特率相同，但是采样相位等参数不同也可能出现异常，可能出现接收不到数据的情况。

而can波特率的设置有些厂家是分频器件不同，采样的固宽也可能不同，这个时候就需要根据实际情况做调频设置了。

```sh
CAN模块的系统时钟fcan是其CAN模块时钟（CAN_CLK）输入的频率。

各参数取值范围：

参数                  范围                   编程值                     说明

BRP                   1~64                   0~63                    波特率分频器，定义时间单元tq的时间长度
Sync_Seg              1tq                                            固定长度，总线输入与系统时钟同步
Prop_Seg              1~8tq                  0~7                      补偿物理延时时间
Phase_Seg1            1~8tq                  0~7                      相位缓冲段1，补偿边沿阶段的误差，可通过同步暂时延长
Phase_Seg2            1~8tq                   0~7                      相位缓冲段2，补偿边沿阶段的误差，可通过同步暂时缩短
SJW                   1~4tq                   0~3                       同步跳转宽度，不能比相位缓冲段长

参数的计算步骤
1、首先计算CAN时钟和波特率的比值
2、根据比值确定BRP和时间量子的数目，一个位时间总的时间份额值可以设置在8~25的范围内
3、Sync_Seg的值固定为1tq，量子数目减1后再分配
4、Prop_Seg其长度视系统测量的延迟时间而定，必须为可扩展的CAN总线系统定义最大的总线长度和最大的节点延迟。
5、Prog_Seg<=Phase_Seg1<=Phase_Seg2；Phase_Seg1=Phase_Seg2 或者 Phase_Seg1+1=Phase_Seg2
6、Phase_Seg2不能比CAN控制器的信息处理时间短
7、同步跳转宽度SJW取4和 Phase_Seg1中小的那个

总的来说，波特率BitRate=Fpclk/((BRP+1)*((TSEG1+1)+(TSEG2+1)+1)
```

![image-20240821114939406](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/刘天鹏/vx_images/image-20240821114939406.png)

![image-20240821115039454](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/刘天鹏/vx_images/image-20240821115039454.png)

EMU CAN

| buad | prop-seg | phase-seg1 | phase-seg2 | sjw  |
| ---- | -------- | ---------- | ---------- | ---- |
| 250k | 41       | 42         | 12         | 1    |
| 125k | 83       | 84         | 24         | 1    |
| 100k | 104      | 105        | 30         | 1    |
| 200k | 52       | 52         | 15         | 1    |
| 500k | 20       | 21         | 6          | 1    |

计算can采样参数程序

原理：ST的Linux CAN驱动调制波特率固定tq，改变prop,phase,sjw来实现；

该程序主要是尽量固定pro,phase,sjw在一个范围，然后计算出tq来进行调波特率

```c
#include <stdio.h>
#include <stdlib.h>


int BRP=0;          //波特率分频器，定义时间单元tq的时间长度
int Prop_Seg=0;     //补偿物理延时时间 1~8tq // 编程值0~7
int Phase_Seg1=0;   //相位缓冲段1，补偿边沿阶段的误差 1~8tq
int Phase_Seg2=0;   //相位缓冲段2，补偿边沿阶段的误差 1~8tq
int SJW=0;          //同步跳转宽度，不能比相位缓冲段长
int BaseTq=41;      //这是ST计算的基础tq,ST调制波特率时会固定tq，调整prop,phase,sjw参数实现

int CLK=24000000;

//TQ=CLK/BUAD/SEG_TOTAL*BASE_TQ
#define CALC_BRP(BUAD,TQ,SEG,SEG1,SEG2,SJW) (((CLK/BUAD)/(SEG+SEG1+SEG2+SJW))*TQ)

//total prop_seg phase_seg1 phase_seg2 sjw
int GlobalCanSeg[][5]={
				{10,4,3,2,1},
        {11,5,3,2,1},
        {12,6,3,2,1},
        {13,7,3,2,1},
        {14,7,4,2,1},
        {15,7,5,2,1},
        {16,7,6,2,1},
        {17,8,6,2,1},
        {18,8,7,2,1},
        {19,8,6,4,1},

};



void main(int argc, char *argv[]){
        int buad=0;
        int i=0;
        int temp=0;

        if(argc != 2){
                printf("%s buad\n",argv[0]);
                return;
        }

        buad=atoi(argv[1]);
        if(CLK%buad != 0){
                printf("这个波特率不能被CAN时钟分频，建议换个波特率试试!\n");
        }

        temp=CLK/buad;
        for(i=0; i<sizeof(GlobalCanSeg)/sizeof(GlobalCanSeg[0]); i++){
                if((temp % GlobalCanSeg[i][0]) == 0){
                        printf("ip link set can0 type can tq %d prop-seg %d phase-seg %d phase-seg2 %d sjw %d\n",CALC_BRP(buad,BaseTq,GlobalCanSeg[i][1],GlobalCanSeg[i][2],GlobalCanSeg[i][3], GlobalCanSeg[i][4]),
                        GlobalCanSeg[i][1],GlobalCanSeg[i][2],GlobalCanSeg[i][3],GlobalCanSeg[i][4]
                        );
                }
        }
}

```

