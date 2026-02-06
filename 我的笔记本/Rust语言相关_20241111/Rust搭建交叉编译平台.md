# Rust搭建交叉编译平台

## 1、rust编译

为了解决第一个问题，rust在自己的管理工具rustup中已经集成了多种平台的编译工具，可以通过下列指令查看rust支持的平台。

```sh
$ rustup target list # 或者也可以使用 rustc --print target-list
aarch64-apple-darwin
aarch64-apple-ios
aarch64-apple-ios-sim
aarch64-linux-android
aarch64-pc-windows-msvc
aarch64-unknown-fuchsia
aarch64-unknown-linux-gnu (installed)
aarch64-unknown-linux-musl
aarch64-unknown-linux-ohos
aarch64-unknown-none
aarch64-unknown-none-softfloat
aarch64-unknown-uefi
arm-linux-androideabi
........
```

具体这里的目标平台指代什么，可以自行在网上查找。我的理解是，这里选择使用什么是根据自己使用的toolchain来决定的。关于arm的工具链，这个[链接](https://www.cnblogs.com/arnoldlu/p/14243491.html)中有一些说明解释。
确定好目标平台后，执行下面指令，会将交叉编译的rust工具自动下载。

```sh
# 根据我自己的toolchain，我选择了使用aarch64-unknown-linux-gnu
$ rustup target add aarch64-unknown-linux-gnu

# 确定rust编译工具已经成功下载
$ rustup show
```

安装好rust交叉编译工具后，可以通过`--target`来选择使用的编译工具:

```sh
# 使用aarch64-unknown-linux-gnu的rust编译工具编译rust源码
$ cargo build --target=aarch64-unknown-linux-gnu
```

## 2、使用toolchain链接

当我们交叉编译开发其他架构的程序时，都会有一套交叉编译的toolchain。在完成rust的编译后，使用目标的交叉编译toolchain来链接得到最终执行在目标平台的程序。

- 修改cargo配置
  目前构建rust的项目主要都是通过cargo进行的。因此，在rust的链接阶段更换为目标平台toolchain中的工具自然也是通过修改cargo的配置来更改。cargo在安装的时候，会在安装用户的用户目录下生成一个文件夹`~/.cargo`。在这个目录下创建一个文件`~/.cargo/config.toml`，并在该文件中配置不同目标的链接器，即可实现在rust编译的最后阶段使用交叉编译的toolchain来链接程序的目的。
- **对`~/.cargo/config.toml`的修改会应用到全局，如果仅仅想对当前的工程起作用，只需在当前工程的目录创建`${PRJ_DIR}/.cargo/config.toml`并修改就行了。**
- `config.toml`参考

```toml
# config.toml
[target.aarch64-unknown-linux-gnu]
linker = "${TOOLCHAIN_PATH}/bin/aarch64-linux-gnu-gcc"
# rustflags = "-C prefer-dynamic" ## 如果想要动态连接rust标准库可以打开这个配置。
```

例如：

我在我的测试项目：`test2`下建立一个文件夹：`.cargo`，并在此文件夹下建立文件：`config.toml`，内容如下：

```toml
# config.toml
[target.arm-unknown-linux-gnueabihf]
linker = "/media/ubuntu16.04/opt/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc"
# rustflags = "-C prefer-dynamic" ## 如果想要动态连接rust标准库可以打开这个配置。
```

经过这样配置后，就可以使用：

```sh
cargo build --target=arm-unknown-linux-gnueabihf
```

来编译程序了。

## 3、交叉编译

rust官方基本上已经把交叉编译的繁琐工作完成了，用户只需要进行简单配置即可。经过上述两个步骤的配置，就能完成rust的交叉编译了，最后放一张在开发板上运行的小螃蟹吧~~~~~

![img](/media/sf_E_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/Rust语言相关_20241111/2929707-20240509103719982-67897156.png)