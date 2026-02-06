# Verilog HDL双沿采样（上升沿、下降沿）触发器电路设计

[TOC]

## 前言

今天下午更新了一篇博客，对于博客中的一个问题我觉得有必要再深入探讨一下，那就是双沿采样电路。

## 题目

Build a circuit that functionally behaves like a dual-edge triggered flip-flop:

![20200520211334953](C:\Users\walei163\AppData\Local\Temp\20200520211334953.png)

方法一

```verilog
module top_module (
    input clk,
    input d,
    output q
);
    reg q_d1;
    reg q_d2;
    
    always@(posedge clk)begin
        q_d1 <= d ^ q_d2;
    end
    always@(negedge clk)begin
        q_d2 <= d ^ q_d1;
    end
    
    assign q = q_d1 ^ q_d2;
    
endmodule
```

为什么这种方法可以呢？

> [!note]
>
> **首先，在上升沿的时候，q_d1变成了d ^ q_d2，那么q = q_d1 ^ q_d2 = d ^ q_d2 ^ q_d2 = d；**
>
> **接着，在下降沿的时候，q_d2变成了d ^ q_d1，那么q = q_d1 ^ q_d2 = q_d1 ^ d ^ q_d1 = d；**
>
> **在每个正负沿，q_d1和q_d2交替出现，因此q总会输出最新的值。**

这样就可以实现双沿采样，这种方法真是非常巧妙~
方法二

```verilog
module top_module (
    input clk,
    input d,
    output q
);
 
    reg q_d1;
    reg q_d2;
 
    always@(posedge clk)begin
        q_d1 <= d;
    end
    
    always@(negedge clk)begin
        q_d2 <= d;
    end
    
    assign q = clk ? q_d1 : q_d2;
    
endmodule
```

这种方法更好理解，在上升沿和下降沿分别对d采样，然后根据时钟的正负，选择输出上升沿和下降沿采样后的信号。博主最先想到的答案也是这个。

> [!warning]
>
> <font face="微软雅黑" color=yellow>**对于做题而言，这个答案没有任何问题，因为毛刺对success不会有很大影响，但是实际应用中，这个电路会产生毛刺。**</font>

大家可以想象，在上升沿和下降沿采到d信号以后，q_d1和q_d2会立即变化，同时时钟也会立即变化，时钟和q_d1、q_d2的变化的延迟是不一致的，这就导致了毛刺的出现，这时候的时钟相当于是一个组合逻辑了，由于路径延时的不同，导致组合逻辑到达的时间存在差异，从而会产生毛刺，如下图所示。

![](https://img-blog.csdnimg.cn/20200522210957316.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dhbmdrYWlfMjAxOQ==,size_16,color_FFFFFF,t_70)

为什么方法一不会产生毛刺呢？大家看看代码，方法一的结果是两个寄存器直接异或，没有涉及到时钟信号，方法二的结果需要时钟信号进行判断，而时钟信号时钟的上升沿和下降沿又是触发器的输出，即q_d1、q_d2，因此会产生毛刺。

## 注意点

> [!caution]
>
> <font face="微软雅黑" color=yellow>**不能使用always@(posedge clk or nededge clk)，不瞒大家说，博主第一次就尝试了这个方法，但是这种写法是不可综合的，严禁使用！**</font>>

## 总结

今天对于这个小问题又多做了一点说明，希望大家能够掌握着两种写法的差异，并且明白毛刺产生的原因。最后希望能和大家一起学习，共同进步~
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/wangkai_2019/article/details/106244516