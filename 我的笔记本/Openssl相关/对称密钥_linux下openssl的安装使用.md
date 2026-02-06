# linux下openssl的安装使用

这篇文档里有描述如何使用私钥来加密文档的方法：

在Linux下安装和使用Open[ssl](https://geek.csdn.net/educolumn/2acc0e1b65f4e7a89c9f5206ed21d243?spm=1055.2569.3001.10083)，可以按照以下步骤进行：

1. 安装OpenSSL

可以使用[系统](https://geek.csdn.net/educolumn/073441cd796e7450c3f98600b0cbeb95?spm=1055.2569.3001.10083)自带的包管理器进行安装，如在Ubuntu下可以使用以下命令：

```shell
sudo apt-get install openssl
```

1. 生成证书

可以使用OpenSSL生成自签名证书，如下所示：

```shell
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365
```

其中，key.pem是私钥文件，cert.pem是证书文件，-days 365表示证书有效期为365天。

1. 使用证书

可以使用生成的证书和私钥进行加密和解密，如下所示：

```shell
openssl enc -aes-256-cbc -in plaintext.txt -out ciphertext.enc -pass file:./key.pem
openssl enc -d -aes-256-cbc -in ciphertext.enc -out plaintext.txt -pass file:./key.pem
```

其中，plaintext.txt是明文文件，ciphertext.enc是密文文件，-aes-256-cbc表示使用AES-256[算法](https://geek.csdn.net/educolumn/2354e3486d804fe718dd230739581535?spm=1055.2569.3001.10083)进行加密，-pass file:./key.pem表示使用key.pem文件中的密码进行加密和解密。

1. 其他命令

除了上述命令外，OpenSSL还提供了很多其他命令，如生成CSR、签名证书、[验证](https://geek.csdn.net/educolumn/0330ae9ce73d0920177833b396480304?spm=1055.2569.3001.10083)证书等，可以根据需要进行使用。具体命令可以参考OpenSSL的官方[文档](https://geek.csdn.net/edu/300981ef51993cfd737d329c71ba77f9?dp_token=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpZCI6NDQ0MDg2MiwiZXhwIjoxNzA3MzcxOTM4LCJpYXQiOjE3MDY3NjcxMzgsInVzZXJuYW1lIjoid2VpeGluXzY4NjQ1NjQ1In0.RrTYEnMNYPC7AQdoij4SBb0kKEgHoyvF-bZOG2eGQvc&spm=1055.2569.3001.10083)。