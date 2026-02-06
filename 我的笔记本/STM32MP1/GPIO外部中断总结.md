# GPIO外部中断总结

 STM32 GPIO外部中断总结

一、STM32中断分组：

　　STM32 的每一个GPIO都能配置成一个外部中断触发源，这点也是 STM32 的强大之处。STM32 通过根据引脚的序号不同将众多中断触发源分成不同的组，比如：PA0，PB0，PC0，PD0，PE0，PF0，PG0为第一组，那么依此类推，我们能得出一共有16 组，STM32 规定，每一组中同时只能有一个中断触发源工作，那么，最多工作的也就是16个外部中断。STM32F103 的中断控制器支持 19 个外部中断/事件请求。每个中断设有状态位，每个中断/事件都有独立的触发和屏蔽设置。STM32F103 的19 个外部中断为：

线 0~15：对应外部 IO 口的输入中断。

 |  GPIO引脚   |  中断标志位   |  中断处理函数   |
 | --- | --- | --- |
 |  PA0~PG0   |  EXTI0    |  EXTI0_IRQHandler   |
 |   PA1~PG1  |  EXTI1   |  EXTI1_IRQHandler   |
 |  PA2~PG2   |  EXTI2   |  EXTI2_IRQHandler   |
 |  PA3~PG3   |  EXTI3   | EXTI3_IRQHandler    |
 |  PA4~PG4   |  EXTI4   |  EXTI4_IRQHandler   |
 |  PA5~PG5    |  EXTI5   | EXTI9_5_IRQHandler    |
 |  PA6~PG6   |  EXTI6   |     |
 |  PA7~PG7   |  EXTI7   |     |
 |  PA8~PG8   |  EXTI8   |     |
 |  PA9~PG9   |  EXTI9   |     |
 |  PA10~PG10   |  EXTI10   |  EXTI15_10_IRQHandler   |
 |  PA11~PG11   |  EXTI11   |     |
 |  PA12~PG12   |  EXTI12   |     |
 |  PA13~PG13   |  EXTI13   |     |
 |  PA14~PG14   |  EXTI14   |     |
 |  PA15~PG15   |  EXTI15   |     |
 |  线 16   |     |  连接到 PVD 输出。   |
 |  线 17   |     |  连接到 RTC 闹钟事件。   |
 |  线 18   |     |  连接到 USB 唤醒事件。   |

二：外部中断的配置过程：

1.  配置触发源GPIO口：

　　因为GPIO口作为触发源使用，所以将GPIO口配置成输入模式，触发模式有以下几种：

```
　　a.GPIO_Mode_AIN ，模拟输入（ADC模拟输入，或者低功耗下省电）

　　b.GPIO_Mode_IN_FLOATING ，浮空输入

　　c.GPIO_Mode_IPD ，带下拉输入

　　d.GPIO_Mode_IPU ，带上拉输入　
```


```
　　GPIO_InitTypeDef GPIO_InitStructure;//定义结构体

　　RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);//使能时钟

　　GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//选择IO口   PE2

　　GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入

　　GPIO_Init(GPIOE, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
```

2. 使能AFIO复用时钟功能：

```
　　RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);　
```

　　

3. 将GPIO口与中断线映射起来：　

```
　　GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource2);

```
 

4. 中断线上进行中断初始化：　

```
　　EXTI_InitTypeDef EXTI_InitStructure;//定义初始化结构体

　　EXTI_InitStructure.EXTI_Line=EXTI_Line2; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15

　　EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。

　　EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling

　　EXTI_InitStructure.EXTI_LineCmd = ENABLE;

　　EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化
```


5. 中断优先级配置：　

```
　　NVIC_InitTypeDef NVIC_InitStructure;//定义结构体

　　NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; //使能外部中断所在的通道

　　NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2， 

　　NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02; //子优先级 2

　　NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道 

　　NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化 
```



 

6. 外部中断服务函数的编写：

　　外部中断函数分别为：

　　EXPORT   EXTI0_IRQHandler          

　　EXPORT   EXTI1_IRQHandler       

　　EXPOR T   EXTI2_IRQHandler         

　　EXPORT   EXTI3_IRQHandler        

　　EXPORT   EXTI4_IRQHandler          

　　EXPORT   EXTI9_5_IRQHandler        

　　EXPORT   EXTI15_10_IRQHandler

　　中断线 0-4 每个中断线对应一个中断函数，中断线 5-9 共用中断函数 EXTI9_5_IRQHandler，中断线 10-15 共用中断函数 EXTI15_10_IRQHandler。　　　

```
　　void EXTI2_IRQHandler(void)
　　{
　　　　if(EXTI_GetITStatus(EXTI_Line2)!=RESET)//判断某个线上的中断是否发生 

　　　　{
　　　　　　中断逻辑…
　　　　　　EXTI_ClearITPendingBit(EXTI_Line2);   //清除 LINE 上的中断标志位
　　　　}     
　　}
```


 

三、关于使用GPIO口接按键进行外部中断的配置说明：

　　使用按键进行外部中断的时候，一般都需要进行按键延时消抖以及松手检测的相关处理，中断函数可以参看以下代码：

```
　　void EXTI2_IRQHandler(void)
　　{
　　　　delay_ms(10);//延时消抖
　　　　if(KEY2==0)    //按键真的被按下
　　　　　　{
　　　　　　　　LED0=!LED0;
　　　　　　}
　　　　while(KEY2!=0);//等待松手
　　　　EXTI_ClearITPendingBit(EXTI_Line2); //清楚中断标志位 
　　}
```

　　当然，如果你的按键是允许长按功能的，那么就进行别的逻辑操作，这里不作研究。