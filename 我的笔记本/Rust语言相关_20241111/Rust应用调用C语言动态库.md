# Rust应用调用C语言动态库

#### 外部功能接口FFI

虽然高级（脚本）编程语言的功能丰富，表达能力强，但对底层的一些特殊操作的支持并不完善，就需要以其他编程语言来实现。调用其他编程语言的接口，被称为Foreign Function Interface，直译为外部功能接口。该接口通常是调用C语言实现的外部功能模块，因为C语言接近于全能，几乎任何功能都能够实现；正如同使用汇编语言也可以实现很多功能一样，但开发效率低下。很多脚本语言提供了FFI功能，例如Python、PHP和JIT版本的Lua解析器等。同样的，Rust也提供了FFI接口，作为标准库中一个功能模块；但本文不会讨论该模块的使用方法。本文记录了笔者编写一个简单的C语言动态库，并通过Rust调用动态库导出的函数；另外，笔者直接使用Rust官方提供的libc库，直接替代笔者编写的C语言动态库，以避免重复造轮子。

#### UDP套接字的读超时

Rust标准库中的UDP网络功能，提供了设置套接字读超时的函数，set_read_timeout，了解C语言网络编译的开发人员都知道，相应的底层调用为setsockopt(SO_RCVTIMEO)。假设Rust标准库中UDP模块未提供该函数，就需要编写C语言代码，将其编译成一个动态库，尝试将Rust链接到该库，并调用其中定义的函数了。笔者编写的代码如下：

```c
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/* export function to set socket receive timeout */
extern int normal_setsock_timeout(int sockfd,
    unsigned long timeout);

int normal_setsock_timeout(int sockfd,
    unsigned long timeout)
{
    int ret, err_n;
    struct timeval tv;

    tv.tv_sec  = (time_t) (timeout / 1000);
    tv.tv_usec = (timeout % 1000) * 1000;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
        &tv, sizeof(tv));
    if (ret == -1) {
        err_n = errno;
        fprintf(stderr, "Error, setsockopt(%d, RECVTIMEO, %lu) has failed: %s\n",
            sockfd, timeout, strerror(err_n));
        fflush(stderr);
        errno = err_n;
        return -1;
    }
    return 0;
}

```

通过以下命令生成动态库`libsetsock.so`：

```sh
gcc -Wall -O2 -fPIC -D_GNU_SOURCE -shared -o libsetsock.so -Wl,-soname=libsetsock.so mysetsock.c
```

笔者使用Rust编写的简单UDP服务端代码如下：

```rust
use std::net::UdpSocket;
use chrono::{DateTime, Local};

fn get_local_time() -> String {
    let nowt: DateTime<Local> = Local::now();
    nowt.to_string()
}

fn main() -> std::io::Result<()> {
    let usock = UdpSocket::bind("127.0.0.1:2021");
    if usock.is_err() {
        let errval = usock.unwrap_err();
        println!("Error, failed to create UDP socket: {:?}", errval);
        return Err(errval);
    }
    // get the UdpSocket structure
    let usock = usock.unwrap();

    // create 2048 bytes of buffer
    let mut buffer = vec![0u8; 2048];
    println!("{} -> Waiting for UDP data...", get_local_time());

    // main loop
    loop {
        let res = usock.recv_from(&mut buffer);
        if res.is_err() {
            println!("{} -> Error, failed to receive from UDP socket: {:?}",
                get_local_time(), res.unwrap_err());
            break;
        }

        let (rlen, peer_addr) = res.unwrap();
        println!("{} -> received {} bytes from {:?}:{}",
            get_local_time(), rlen, peer_addr.ip(), peer_addr.port());
    }

    // just return ok
    Ok(())
}
```

短短50多行代码实现了一个简单的UDP服务端，作为系统编程语言的Rust开发效率可见一斑。不过该UDP服务器的读操作是阻塞的，它会一直等待网络数据的到来：

```sh
udp_socket$ cargo run
    Finished dev [unoptimized + debuginfo] target(s) in 0.02s
     Running `target/debug/udp_socket`
2021-07-11 19:38:29.791363796 +08:00 -> Waiting for UDP data...
2021-07-11 19:38:39.721713256 +08:00 -> received 16 bytes from 127.0.0.1:39180
2021-07-11 19:38:48.553386975 +08:00 -> received 16 bytes from 127.0.0.1:58811
```

#### Rust调用C语言动态库中的函数

与C语言类似，Rust使用`extern`关键字可实现对外部函数的声明，不过在调用的代码需要以[unsafe](https://doc.rust-lang.org/book/ch19-01-unsafe-rust.html)关键字包成代码块。以下是笔者对上面的Rust代码的修改：

```rust
diff --git a/src/main.rs b/src/main.rs
index 304c7dc..5921106 100644
--- a/src/main.rs
+++ b/src/main.rs
@@ -1,5 +1,12 @@
 use std::net::UdpSocket;
 use chrono::{DateTime, Local};
+use std::os::raw::c_int;
+use std::os::unix::io::AsRawFd;
+
+#[link(name = "setsock")]
+extern {
+    pub fn normal_setsock_timeout(sock_fd: c_int, timo: usize) -> c_int;
+}
 
 fn get_local_time() -> String {
     let nowt: DateTime<Local> = Local::now();
@@ -20,6 +27,11 @@ fn main() -> std::io::Result<()> {
     let mut buffer = vec![0u8; 2048];
     println!("{} -> Waiting for UDP data...", get_local_time());
 
+    // set UDP socket receive timeout
+    unsafe {
+        normal_setsock_timeout(usock.as_raw_fd() as c_int, 5000);
+    }
+
     // main loop
     loop {
         let res = usock.recv_from(&mut buffer);

```

修改后的主代码增加了`#[link]`属性，指示Rust编译器在链接时加入`-lsetsock`链接选项。再次编译，会发现链接命令失败：

```sh
udp_socket$ cargo build
   Compiling udp_socket v0.1.0 (/home/yejq/program/rust-lang/udp_socket)
error: linking with `cc` failed: exit status: 1
```

这说明虽然编译是正常的，但在链接时找不到`libsetsock.so`动态库。解决方法是在工程根目录下增加一个编译控制的Rust代码，文件名为`build.rs`，给出动态库所在的目录：

```rust
fn main() {
    println!(r"cargo:rustc-link-search=native=/home/yejq/program/rust-lang/socket_udp");
}
```

再次执行`cargo build`编译工程，链接就能成功了；使用`patchelf`和`nm`等命令行工具察看，生成的可执行文件依赖了C语言编写的动态库`libsetsock.so`，并引用了其导出的函数符号`normal_setsock_timeout`：

```sh
udp_socket$ cargo build
   Compiling udp_socket v0.1.0 (/home/yejq/program/rust-lang/udp_socket)
    Finished dev [unoptimized + debuginfo] target(s) in 1.72s
udp_socket$ patchelf --print-needed ./target/debug/udp_socket
libsetsock.so
libgcc_s.so.1
libpthread.so.0
libdl.so.2
libc.so.6
ld-linux-x86-64.so.2
udp_socket$ nm --undefined-only ./target/debug/udp_socket | grep -e normal_setsock
                 U normal_setsock_timeout
```

此时运行简单UDP服务端程序，可以确定我们增加的套接字读超时功能能够正常工作

```sh
udp_socket$ LD_LIBRARY_PATH=/home/yejq/program/rust-lang/socket_udp ./target/debug/udp_socket
2021-07-11 19:55:26.279653039 +08:00 -> Waiting for UDP data...
2021-07-11 19:55:29.788948366 +08:00 -> received 16 bytes from 127.0.0.1:43303
2021-07-11 19:55:31.977738660 +08:00 -> received 16 bytes from 127.0.0.1:46854
2021-07-11 19:55:37.179290653 +08:00 -> Error, failed to receive from UDP socket: Os { code: 11, kind: WouldBlock, message: "Resource temporarily unavailable" }
```

#### 避免重复造轮子，使用Rust官方C语言库

以上我们用C语言编写了简单的动态库，导出了一个可设置套接字读超时的函数。这个功能过于简单，费大周折编写一个动态库显得得不偿失。另一个解决方案是直接使用Rust官方提供的C语言库，该库提供了很多变量和函数(与glibc提供的宏定义和库函数、系统调用有很多重叠)，可以直接添加setsockopt等系统调用的代码。修改UDP服务器代码：

```rust
diff --git a/src/main.rs b/src/main.rs
index 5921106..3f4bc84 100644
--- a/src/main.rs
+++ b/src/main.rs
@@ -2,11 +2,7 @@ use std::net::UdpSocket;
 use chrono::{DateTime, Local};
 use std::os::raw::c_int;
 use std::os::unix::io::AsRawFd;
-
-#[link(name = "setsock")]
-extern {
-    pub fn normal_setsock_timeout(sock_fd: c_int, timo: usize) -> c_int;
-}
+use libc;
 
 fn get_local_time() -> String {
     let nowt: DateTime<Local> = Local::now();
@@ -27,9 +23,17 @@ fn main() -> std::io::Result<()> {
     let mut buffer = vec![0u8; 2048];
     println!("{} -> Waiting for UDP data...", get_local_time());
 
-    // set UDP socket receive timeout
     unsafe {
-        normal_setsock_timeout(usock.as_raw_fd() as c_int, 5000);
+        let time_val = libc::timeval {
+            tv_sec: 5,
+            tv_usec: 0,
+        };
+
+        // set socket receive timeout via extern create, libc
+        libc::setsockopt(usock.as_raw_fd() as c_int,
+            libc::SOL_SOCKET, libc::SO_RCVTIMEO,
+            &time_val as *const libc::timeval as *const libc::c_void,
+            std::mem::size_of_val(&time_val) as libc::socklen_t);
     }

```

除了以上的修改，还需要在`Cargo.toml`文件中加入C语言库的依赖，这里笔者使用的libc版本为0.2.98：

```toml
diff --git a/Cargo.toml b/Cargo.toml
index f802b0d..eb0b78e 100644
--- a/Cargo.toml
+++ b/Cargo.toml
@@ -6,4 +6,5 @@ edition = "2018"
 # See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html
 
 [dependencies]
+libc = "0.2.98"
 chrono = "0.4.19"
```

以上修改的代码，与之前相同的是，调用C语言库提供的函数也需要用到unsafe代码块；而工程根目录下的编译相关的控制代码build.rs就不再需要了；编译生成的UDP服务器也会在5秒无数据时退出。最后，能够调用C语言编写的动态库，意味着使用Rust语言来进行嵌入式系统软件的开发，是一种具备可行性的技术方案。
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。

原文链接：https://blog.csdn.net/yeholmes/article/details/118660405