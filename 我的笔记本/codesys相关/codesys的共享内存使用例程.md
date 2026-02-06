# codesys的共享内存使用例程

项目目标：实现Linux程序和codesys应用程序之间的数据读写

实现方法：使用共享内存的方式

## 1、使用codesys软件，使用ST编程语言，编写IDE程序：

```pascal
PROGRAM PLC_PRG
 
VAR
 
tt: Ton;
 
hShMemRead: RTS_IEC_HANDLE;
 
ResultRead: RTS_IEC_RESULT;
 
deInstRead : DataExchange;
 
ReadResult: RTS_IEC_RESULT;
 
iRead: __UXINT;
 
 
 
hShMemWrite: RTS_IEC_HANDLE;
 
ResultWrite: RTS_IEC_RESULT;
 
deInstWrite : DataExchange;
 
WriteResult: RTS_IEC_RESULT;
 
iWrite: __UXINT;
 
ulSize: UDINT := SIZEOF(DataExchange);
 
END_VAR
 
 
 
tt(IN := TRUE,PT:=T#500MS);
 
IF tt.Q THEN
 
// Read structure from Shared Memory.
 
hShMemRead := SysSharedMemoryCreate('_CODESYS_SharedMemoryTest_Read', 0, ADR(ulSize), ADR(ResultRead));
 
iRead := SysSharedMemoryRead(hShMemRead, 0, ADR(deInstRead), SIZEOF(deInstRead), ADR(ReadResult));
 
iRead := SysSharedMemoryClose(hShm := hShMemRead);
 
 
 
// Write structure to Shared Memory.
 
hShMemWrite := SysSharedMemoryCreate('_CODESYS_SharedMemoryTest_Write', 0, ADR(ulSize), ADR(ResultWrite));
 
deInstWrite.diValue1 := deInstWrite.diValue1 + 1;
 
deInstWrite.diValue2 := deInstWrite.diValue2 - 1;
 
iWrite := SysSharedMemoryWrite(hShMemWrite, 0, ADR(deInstWrite), SIZEOF(deInstWrite), ADR(WriteResult));
 
iWrite := SysSharedMemoryClose(hShm := hShMemWrite);
 
 
 
tt(IN:=FALSE);
 
tt(IN:=TRUE);
 
END_IF
```

---

## 2、将程序下载到控制器之中：

![image-20260205104714897](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260205104714897.png)

---

## 3、采用shm内存共享的方式，编写c语言程序：

```c
#include <stdio.h>
 
#include <stdlib.h>
 
#include <sys/mman.h>
 
#include <sys/stat.h>
 
#include <fcntl.h>
 
#include <string.h>
 
 
 
struct DataExchange {
 
    int i1;
 
    int i2;
 
};
 
 
 
int main(int argc, char **argv)
 
{
 
    struct DataExchange *pRead, *pWrite;
 
    char sSharedMemReadName[100] = "_CODESYS_SharedMemoryTest_Write";
 
    char sSharedMemWriteName[100] = "_CODESYS_SharedMemoryTest_Read";
 
 
 
    int fdRead = shm_open(sSharedMemReadName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
 
    printf("Shared Memory Read: %s %d\n\n",sSharedMemReadName, fdRead);
 
    ftruncate(fdRead, sizeof(*pRead));
 
    pRead = mmap(0, sizeof(*pRead), PROT_READ | PROT_WRITE, MAP_SHARED, fdRead, 0);
 
    close(fdRead);
 
 
 
    int fdWrite = shm_open(sSharedMemWriteName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
 
    printf("Shared Memory Write: %s %d\n\n",sSharedMemWriteName, fdWrite);
 
    ftruncate(fdWrite, sizeof(*pWrite));
 
    pWrite = mmap(0, sizeof(*pWrite), PROT_READ | PROT_WRITE, MAP_SHARED, fdWrite, 0);
 
    close(fdWrite);
 
 
 
    do
 
    {
 
        sleep(1);
 
        printf("pRead->i1: %d pRead->i2: %d pWrite->i1: %d pWrite->i2: %d\n",
 
               pRead->i1, pRead->i2, pWrite->i1, pWrite->i2);
 
        printf("Press 'Enter' to increment values or 'q' and then 'Enter' to quit\n");
 
        pWrite->i1++;
 
        pWrite->i2--;
 
    } while (getchar() != 'q');
 
 
 
    munmap(pRead, sizeof(*pRead));
 
    shm_unlink(sSharedMemReadName);
 
 
 
    munmap(pWrite, sizeof(*pWrite));
 
    shm_unlink(sSharedMemWriteName);
 
    exit(0);
 
}
```

---

根据使用的硬件平台，用gcc编译成可执行文件，便能够同时在系统和IDE上看到共享数据，如下图所示：

![image-20260205104915572](/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/codesys相关/images/image-20260205104915572.png)

由于刷新频率并不同步，两边的数值显示上有点差别。

原文链接：https://blog.csdn.net/weixin_42605077/article/details/143967688