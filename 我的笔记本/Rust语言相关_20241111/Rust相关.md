# Rust语言学习相关

1. ## 安装：

```sh
$ curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh
```

这将会直接下载并安装与rust相关的cargo包管理器等。安装完成后会修改用户home目录下的`.bashrc`文件，并在最后一行会添加：

```sh
. "$HOME/.cargo/env"
```

更新cargo：

```sh
rustup update
```

卸载cargo：

```sh
rustup self uninstall
```



2. ## 命令：

```
cargo --version
```

```
cargo new 项目名称
```

```
cargo build (--release)
```

```
cargo run
```

```
cargo check
```

3. 库依赖：

编辑`Cargo.toml`文件，如下所示，在`“dependencies”`下添加你所需要的依赖库：

```sh
[package]
name = "hello_world"
version = "0.1.0"
edition = "2021"

[dependencies]
ferris-says = "0.2"
rand = "^0.3.14"
```



4. 其它
