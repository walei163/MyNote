# OpenSSL 密钥加/解密大文件

作者：肆不肆傻
链接：https://www.jianshu.com/p/376a96a28f4d
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

一般情况下，如果我们采用rsa生成公钥和私钥，并用公钥对文件进行加密，如下面这个命令行：

```sh
openssl rsautl -encrypt -pubin -inkey public.pem -in LargeFile.zip -out LargeFile_encrypted.zip
```

会出现以下错误提示：

```sh
RSA operation error:
3020:error:0406D06E:rsa routines:RSA_padding_add_PKCS1_type_2:data too large for key size:.\crypto\rsa\rsa_pk1.c:151:
```

也就是如果加密的文件过大，则不能通过。

解决方案是添加参数：smime。以下是具体步骤：

1、生成公钥和私钥：

```sh
openssl req -x509 -nodes -days 100000 -newkey rsa:2048  -keyout privatekey.pem  -out publickey.pem
```

2、加密大文件如zip压缩包等：

```sh
openssl  smime  -encrypt -aes256  -in  LargeFile.zip  -binary  -outform DEM  -out  LargeFile_encrypted.zip  publickey.pem
```

3、解密文件：

```sh
openssl  smime -decrypt  -in  LargeFile_encrypted.zip  -binary -inform DEM -inkey privatekey.pem  -out  LargeFile.zip 
```

以下是如何查找到系统硬盘UUID的代码，可以通过这种方式，结合rsa非对称加密方式来加密文件：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
static char* get_disk_uuid(const char* disk) {
    char cmd[256];
    char buffer[256];
    char* uuid = NULL;
    size_t len = 0;
    FILE* pipe = NULL;
 
    // 构建命令行
    snprintf(cmd, sizeof(cmd), "blkid -s UUID %s", disk);
 
    // 打开管道执行命令
    pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen failed");
        return NULL;
    }
 
    // 读取命令输出直到找到UUID
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // 查找UUID的起始位置
        char* found = strstr(buffer, "UUID=\"");
        if (found) {
            // 跳过"UUID=\"
            found += 6;
            // 查找UUID的结束位置
            char* end = strchr(found, '"');
            if (end) {
                // 计算UUID的长度
                len = end - found;
                // 分配空间存储UUID
                uuid = malloc(len + 1);
                if (uuid) {
                    // 复制UUID到缓冲区
                    memcpy(uuid, found, len);
                    uuid[len] = '\0';
                }
                break;
            }
        }
    }
 
    // 关闭管道
    pclose(pipe);
 
    return uuid;
}
 
int main() 
{
    const char* disk_to_check = "/dev/sda1"; // 根据实际情况替换为你的磁盘设备文件
    char* uuid = get_disk_uuid(disk_to_check);
 
    if (uuid) {
        printf("UUID for %s: %s\n", disk_to_check, uuid);
        free(uuid); // 释放内存
    } else {
        printf("Failed to get UUID for %s\n", disk_to_check);
    }
 
    return 0;
}
```

