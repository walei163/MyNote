# [openssl库在linux下的安装使用](https://www.cnblogs.com/gengtongyu/p/17325208.html)

[TOC]



# **一、openssl简介**（详情可百度搜索Openssl）

　　在计算机网络上，OpenSSL是一个开放源代码的软件库包，应用程序可以使用这个包来进行安全通信，避免窃听，同时确认另一端连接者的身份。这个包广泛被应用在互联网的网页服务器上。

　　OpenSSL采用C语言作为开发语言，这使得OpenSSL具有优秀的跨平台性能，支持Linux、Windows、BSD、Mac、VMS等平台。它不仅仅是一个库，它还是一个多用途的、跨平台的密码工具。

　　OpenSSL包含一个命令行工具用来完成OpenSSL库中的所有功能，更好的是，它可能已经安装到你的系统中了。

# 二、功能

　　1、OpenSSL软件包分三个部分：SSL协议库、应用程序、密码算法库。其目录结构也是围绕这三个部分进行规划。

　　2、OpenSSL提供的功能相当强大和全面，囊括了主要的密码算法、常用的密钥和证书封装管理功能以及SSL协议。

　　3、BIO机制是OpenSSL提供的一种高层IO接口，该接口封装了几乎所有类型的IO接口，如内存访问、文件访问以及Socket等。

　　4、OpenSSL对于随机数的生成和管理也提供了一整套的解决方法和支持API函数。

# **三、算法**

## 　　**1、****密钥证书管理**

　　　　OpenSSL实现了ASN.1的证书和密钥相关标准，提供了对证书、公钥、私钥、证书请求以及CRL等数据对象的DER、PEM和BASE64的编解码功能。

## 　　**2、对称加密**

　　　　OpenSSL一共提供了8种对称加密算法，其中7种是分组加密算法，仅有的一种流加密算法是RC4。这7种分组加密算法分别是AES、DES、Blowfish、CAST、IDEA、RC2、RC5，都支持电子密码本模式（ECB）、加密分组链接模式（CBC）、加密反馈模式（CFB）和输出反馈模式（OFB）四种常用的分组密码加密模式。其中，AES使用的加密反馈模式（CFB）和输出反馈模式（OFB）分组长度是128位，其它算法使用的则是64位。事实上，DES算法里面不仅仅是常用的DES算法，还支持三个密钥和两个密钥3DES算法。

## **3、非对称加密**

　　　　OpenSSL一共实现了4种非对称加密算法，包括DH算法、RSA算法、DSA算法和椭圆曲线算法（EC）。DH算法一般用于密钥交换。RSA算法既可以用于密钥交换，也可以用于数字签名，当然，如果你能够忍受其缓慢的速度，那么也可以用于数据加密。DSA算法则一般只用于数字签名。

## **4、信息摘要**

　　　　OpenSSL实现了5种信息摘要算法，分别是MD2、MD5、MDC2、SHA（SHA1）和RIPEMD。SHA算法事实上包括了SHA和SHA1两种信息摘要算法。此外，OpenSSL还实现了DSS标准中规定的两种信息摘要算法DSS和DSS1。

# **四、Openssl在linux系统上的使用**（适用于移植linux开发板）

　　1、OpenSSL最新版本下载地址：http://www.openssl.org/source/　如果下载的压缩包里是.so库和头文件，则可以直接使用。

　　 2、安装过程　

　　　　tar -vxzf openssl-1.1.1d.tar.gz

　　　　b.解压完成以后就会生成一个名为 openssl-1.1.1d 的目录，然后在新建一个名为“openssl”的文件夹，用于存放 openssl 的编译结果。进入到解压出来的 openssl-1.1.1d 目录中，然后执行如下命令进行配置：

　　　　　　./Configure linux-armv4 shared no-asm --prefix=/home/zuozhongkai/linux/IMX6ULL/tool/openssl

　　　　　　CROSS_COMPILE=arm-linux-gnueabihf-

　　　　　　上述配置中“linux-armv4”表示 32 位 ARM 凭条，并没有“linux-armv7”这个选项。CROSS_COMPILE 用于指定交叉编译器。配置成功以后会生成 Makefile，输入如下命令进行编译：

- - - 　　　　make
    - 　　　　make install

　　　　c.编译安装完成以后的 openssl 目录内容如下所示：

　　　　　　![img](https://img2023.cnblogs.com/blog/2604348/202304/2604348-20230425122252575-1730094383.png)

 　　　　　图中的 lib 目录是我们需要的，将 lib 目录下的 libcrypto 和 libssl 库拷贝到开发板根文件系统中的/usr/lib 目录下，命令如下：

　　　　　　　　

```shell
sudo cp libcrypto.so* /home/zuozhongkai/linux/nfs/rootfs/lib/ -af
sudo cp libssl.so* /home/zuozhongkai/linux/nfs/rootfs/lib/ -af
```

　　3、使用

　　　　你也可以使用`make test`测试一下有没有问题，至此，就可以在代码里包含Openssl的头文件进行测试了。

　　　　如果是在linux上可以通过`man openssl`查看帮助文档。使用范例参考https://www.cnblogs.com/yangxiaolan/p/6256838.html。

# **五、非对称加密算法RSA的使用**

　　提取openssl中RSA库函数封装成方便我们使用的函数，如下：

```c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include "base64.h"
#include "RSA.h"
 
/**
 * @brief  RSA函数封装
 * @note   
 * @param  rsa_ctx: RSA结构体
 * @param  *instr: 输入明文或密文数据
 * @param  path_key: 密钥转换后的路径，内部自动完成
 * @param  inlen: 输入明文或密文的长度
 * @param  outstr: 输出加解密结果
 * @param  type: 加密解密方式
 * @retval 失败返回负数，成功返回模数字节数据
 */ 
static int do_operation(RSA* rsa_ctx,char *instr,char* path_key,int inlen,char** outstr,int type)
{
    if(rsa_ctx == NULL || instr == NULL || path_key == NULL)
    {
        perror("input elems error,please check them!");
        return -1;
    }
 
    int rsa_len,num;
    rsa_len=RSA_size(rsa_ctx);
    *outstr=(unsigned char *)malloc(rsa_len+1);
    memset(*outstr,0,rsa_len+1);
    if(inlen == 0){
            perror("input str len is zero!");
            goto err;
    }

    switch(type){
        case 1: //pub enc  公钥加密
            num = RSA_public_encrypt(inlen,(unsigned char *)instr,(unsigned char*)*outstr,rsa_ctx,RSA_PKCS1_PADDING);
        break;

        case 2: //prv dec  私钥解密
            num = RSA_private_decrypt(inlen,(unsigned char *)instr,(unsigned char*)*outstr,rsa_ctx,RSA_PKCS1_PADDING);
        break;
        
        case 3: //prv enc  私钥加密
            num = RSA_private_encrypt(inlen,(unsigned char *)instr,(unsigned char*)*outstr,rsa_ctx,RSA_PKCS1_PADDING);
        break;

        case 4: //pub dec  公钥解密
            num = RSA_public_decrypt(inlen,(unsigned char *)instr,(unsigned char*)*outstr,rsa_ctx,RSA_PKCS1_PADDING);
        break;

        default:
        break;
    }
 
    if(num == -1)
    {
        printf("Got error on enc/dec!\n");
err:
        free(*outstr);
        *outstr = NULL;
        num = -1;
    }
 
    return num;
}

/**
 * @brief  RSA加密
 * @note   
 * @param  *str: 要加密的字符串
 * @param  *Inkey：要输入的密钥
 * @param  *out_key_path: 密钥解析后的路径   宏 PUBLICKEY(公钥加密) 宏 PRIVATEKEY(私钥加密)
 * @param  outstr: 输出的密文
 * @retval 
 */ 
int Rsa_Encrypt(char *str,char *Inkey,char *out_key_path,char** outstr)
{
    RSA *p_rsa;
    BIO *bio = NULL;
    //FILE *file;//也可用读文件方式
    int flen,rsa_len,num;
  
    if(strcmp(out_key_path,PUBLICKEY)==0)  //相等
    { 
        if ((bio = BIO_new_mem_buf(Inkey, -1)) == NULL) //pubkey      //从字符串读取RSA公钥 ---公钥加密
        {
            perror("BIO_new_mem_buf failed!");
        }

        if((p_rsa=PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL))==NULL){
            ERR_print_errors_fp(stdout);
            return -1;
        }
        num = do_operation(p_rsa,str,out_key_path,strlen(str),outstr,1);
    }   

    if(strcmp(out_key_path,PRIVATEKEY)==0)  //相等 
    {  
        if ((bio = BIO_new_mem_buf(Inkey, -1)) == NULL)   //prikey    //从字符串读取RSA私钥 ---私钥加密
        {
            perror("BIO_new_mem_buf failed!");
        }

        if((p_rsa=PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL))==NULL){

            ERR_print_errors_fp(stdout);
            return -1;
        }
        num = do_operation(p_rsa,str,out_key_path,strlen(str),outstr,3);
    }  
    
    RSA_free(p_rsa);

    BIO_free_all(bio);

    return num;
}

/**
 * @brief  RSA解密
 * @note   
 * @param  *str: *str: 要解密的字符串
 * @param  *Inkey: 要输入的密钥
 * @param  *out_key_path: 密钥解析后的路径  宏 PRIVATEKEY(私钥解密) 宏 PUBLICKEY(公钥解密) 
 * @param  inlen: base64-HEX解密后的密钥摸数据
 * @param  outstr: 输出的明文
 * @retval 
 */
int Rsa_Decrypt(char *str,char *Inkey,char *out_key_path,int inlen,char** outstr)
{
    RSA *p_rsa;
    BIO *bio = NULL;
    //FILE *file;
    int rsa_len,num;
    
    if(strcmp(out_key_path,PRIVATEKEY)==0)  //相等
    {
        
        if ((bio = BIO_new_mem_buf(Inkey, -1)) == NULL)  //prikey     //从字符串读取RSA公钥 ---私钥解密
        {
            perror("BIO_new_mem_buf failed!");
        }

        if((p_rsa=PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL))==NULL){
            ERR_print_errors_fp(stdout);
            return -1;
        }
        num = do_operation(p_rsa,str,out_key_path,inlen,outstr,2);
    }   
   
    if(strcmp(out_key_path,PUBLICKEY)==0)  //相等
    {   
        if ((bio = BIO_new_mem_buf(Inkey, -1)) == NULL)  //pubkey     //从字符串读取RSA公钥 ---公钥解密
        {
            perror("BIO_new_mem_buf failed!");
        }
        
        printf("");

        if((p_rsa=PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL))==NULL){

            ERR_print_errors_fp(stdout);
            return -1;
        }
        num = do_operation(p_rsa,str,out_key_path,inlen,outstr,4);
    }    
 
    RSA_free(p_rsa);
    BIO_free_all(bio);
 
    return num;
}

void rsa_test()
{
    char* pubkey ="-----BEGIN PUBLIC KEY-----\n\
    MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAJqQ8N99+XicH4fjwjnpjVeJDXMJqrB3\
    qhdta0PqU00v60LSv95IsRG8hryQxxTN6lJ0XDC0N/A4JR09t++Y3x8CAwEAAQ==\
    \n-----END PUBLIC KEY-----\n";

    char* prikey="-----BEGIN PRIVATE KEY-----\n\
    MIIBVwIBADANBgkqhkiG9w0BAQEFAASCAUEwggE9AgEAAkEAmpDw3335eJwfh+PCOemNV4kNcwmq\
    sHeqF21rQ+pTTS/rQtK/3kixEbyGvJDHFM3qUnRcMLQ38DglHT2375jfHwIDAQABAkEAmH9wZ9DO\
    liTTJyubsa+weDEMR/e9pKYr9zlJn5RN49Rej6AMW/DCBIqn+jBQCkETdM/R1wWpfkv9PlDdYYgf\
    4QIhAM/TWVYt1QroEtvk3NGhZp24X8u50/dLyUpJrLjBP8CdAiEAvmUbU8Fojj6EWqRsMabWLlCN\
    sEjrfph4cBP7T0YEm+sCIQCDAvcUbrwYrggFexcBR8AKuUoagcDv6mSwYhVyOtDXuQIhALMnqIMz\
    ZxMSOLCiLzy5ZKjIG8tZkl7ZUdsEsmFx3TxpAiEAg0APOfMN9eZNnTTqZxbmFyFtAplF/GQ4SVcn\
    o5dbfxs=\
    \n-----END PRIVATE KEY-----\n";

    char *ptr_en=NULL,*ptr_de=NULL;
    char *ptr_enbase=NULL,*ptr_debase=NULL;
    int len;
    char indata[] = "1frsd4gtrsd4qt01!000000002n";
    #if 1
    len = Rsa_Encrypt(indata,prikey,PRIVATEKEY,&ptr_en);//PRIVATEKEY PUBLICKEY

    printf("encrypt_len=%d:\n",len);//len = 64
    printf("ptr_en:%02x,%02x,%02x\n",ptr_en[0],ptr_en[1],ptr_en[2]);//16进制模数

    ptr_enbase=base64_encode(ptr_en,len);//返回base64格式的密文数据
    printf("base64_encode len=%d\n",strlen(ptr_enbase));
    printf("%s\n",ptr_enbase);

    ptr_debase=base64_decode(ptr_enbase,&len);
    #endif
    
    #if 0  //也可以使用这种方式直接解密
    ptr_debase=base64_decode("PRTL++faj5X+0iCpOyaMBDuEEmYoN0s5mmhF1wiyThZkn/PvvpmO0bJpHnkA7WbNWcpISWCTdCBqGsoZQLOeGA==",&len);
    #endif
    
    printf("decode_len=%d\n",len);
 
    Rsa_Decrypt(ptr_debase,pubkey,PUBLICKEY,len,&ptr_de); //PUBLICKEY  PRIVATEKEY
    printf("prvkey decrypt:%s\n",ptr_de==NULL?"NULL":ptr_de);
 
    if(ptr_enbase!=NULL){
        free(ptr_enbase);
    }
    if(ptr_debase!=NULL){
        free(ptr_debase);
    }
 
    if(ptr_en!=NULL){
        free(ptr_en);
    }
    if(ptr_de!=NULL){
        free(ptr_de);
    }
}
/**
 * @brief  使用公钥解密客户小程序的数据
 * @note   
 * @param  char*Indata: 客户小程序的密文数据
 * @param  char*PubKey: 客户提供的公钥,pem格式或者字符串都可以
 * @param  char*outdata: 解密后的明文
 * @retval 解密失败返回负数，成功返回解密后的长度 
 */
int rsa_pubkey_decrypt(char*Indata,char*PubKey,char**outdata)
{
    if(PubKey == NULL)
        return -1;
    char *ptr_debase=NULL;
    int len,ret = -1;
    char tmp_key[256] = {0},buff[256] = {0};
    
    if(!strstr(PubKey,"-----BEGIN PUBLIC KEY-----") && !strstr(PubKey,"-----END PUBLIC KEY-----"))//表示如果不是PEM格式就拼接成这个格式
    {
        for(int i=0;i<strlen(PubKey)/64;i++)
        {
            strncpy(&buff[64*i+i],&PubKey[64*i],64);//base64格式每64位有一个换行
            buff[64*(i+1)+i] = '\n';//占一个字节
        }
        sprintf(tmp_key,"-----BEGIN PUBLIC KEY-----\n%s-----END PUBLIC KEY-----\n",buff); 
        strcpy(PubKey,tmp_key);
    }
    //printf("PubKey:%s\n",PubKey);
    ptr_debase=base64_decode(Indata,&len);
    //printf("decode_len=%d\n",len);
    
    ret = Rsa_Decrypt(ptr_debase,PubKey,PUBLICKEY,len,outdata); //PUBLICKEY  PRIVATEKEY
    //printf("prvkey decrypt:%s\n",&outdata==NULL?"NULL":*outdata);
 
    if(ptr_debase!=NULL){
        free(ptr_debase);
    }
    return ret;
}
```



```c
#ifndef _RSA_H
#define _RSA_H
 
#define PRIVATEKEY "key.pem"
#define PUBLICKEY "key_pub.pem"
 
//int rsa_pub_encrypt(char *str,char *path_key,char** outstr);
int Rsa_Encrypt(char *str,char *Inkey,char *out_key_path,char** outstr);
//int rsa_prv_decrypt(char *str,char *path_key,int inlen,char** outstr);
int Rsa_Decrypt(char *str,char *Inkey,char *out_key_path,int inlen,char** outstr);

int rsa_pubkey_decrypt(char*Indata,char*PubKey,char**outdata);
 
void rsa_test();
#endif
```

