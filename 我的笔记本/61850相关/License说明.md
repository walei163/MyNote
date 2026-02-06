# License说明

1、`license0.txt ~ license8.txt`与`info0.enc ~ info8.enc`一一对应，不能搞错，否则主程序无法正确运行：

| 信息文件  | License文件  | 备注 |
| :-------: | :----------: | :--: |
| info0.enc | license0.txt |      |
| info1.enc | license1.txt |      |
| info2.enc | license2.txt |      |
| info4.enc | license4.txt |      |
| info5.enc | license5.txt |      |
| info7.enc | license7.txt |      |
| info8.enc | license8.txt |      |

2、将`license（n）.txt`文件通过ssh上传到对应装置的`61850`主程序运行目录下，并改名为：**license.txt**，`61850`主程序即可正确运行。

