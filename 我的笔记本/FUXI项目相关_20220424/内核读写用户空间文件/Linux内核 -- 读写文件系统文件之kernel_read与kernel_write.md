# Linux内核 -- 读写文件系统文件之kernel_read与kernel_write

## Linux 内核中文件读写详解

在 Linux 内核中，读写文件的操作与用户态的文件操作有所不同。内核中操作文件需要使用专门的接口和 API，不能直接使用如 fopen、fread、fwrite 等用户态的标准 C 库函数。内核内的文件操作主要通过 file 结构体进行，配合 vfs（虚拟文件系统）的相关 API 来读写文件。

### 1. 打开文件：`filp_open`

在内核中要操作文件，首先要打开文件。内核中使用的是 `filp_open` 函数。

```c
struct file *filp_open(const char *filename, int flags, umode_t mode);
```

- `filename`：文件路径。
- `flags`：文件打开的标志（例如 `O_RDONLY`、`O_WRONLY`、`O_RDWR` 等，可以与用户态中的标志相同）。
- `mode`：当创建新文件时的权限（可选）。

示例：

```c
struct file *file;
file = filp_open("/path/to/file", O_RDONLY, 0);
if (IS_ERR(file)) {
    printk("Failed to open file\n");
    return PTR_ERR(file);
}
```

### 2. 读取文件：`kernel_read`

读取文件的主要接口是 `kernel_read`，该函数从已打开的文件中读取数据。

```c
ssize_t kernel_read(struct file *file, void *buf, size_t count, loff_t *pos);
```

- `file`：文件结构指针。
- `buf`：读取数据的缓冲区。
- `count`：读取的字节数。
- `pos`：文件偏移量指针。

示例：

```c
loff_t pos = 0;
char buf[128];
ssize_t ret = kernel_read(file, buf, sizeof(buf), &pos);
if (ret >= 0) {
    printk("Read %zd bytes from file\n", ret);
} else {
    printk("Failed to read from file\n");
}
```

### 3. 写入文件：`kernel_write`

写入文件可以使用 `kernel_write` 接口，该函数将数据写入到文件中。

- `file`：文件结构指针。
- `buf`：要写入的数据缓冲区。
- `count`：要写入的字节数。
- `pos`：文件偏移量指针。

示例：

```c
loff_t pos = 0;
const char *data = "Hello, kernel!";
ssize_t ret = kernel_write(file, data, strlen(data), &pos);
if (ret >= 0) {
    printk("Wrote %zd bytes to file\n", ret);
} else {
    printk("Failed to write to file\n");
}
```

### 4. 同步文件：`vfs_fsync`

为了确保文件写入磁盘，可以使用 `vfs_fsync` 来同步文件数据。

```c
int vfs_fsync(struct file *file, int datasync);
```

- `file`：文件结构指针。
- `datasync`：如果为 1，只同步文件数据；如果为 0，则同步所有文件元数据和数据。

示例：

```c
vfs_fsync(file, 0);
```

### 5. 关闭文件：`filp_close`

操作完文件后，需要关闭文件，释放资源。

```c
int filp_close(struct file *filp, fl_owner_t id);
```

示例：

```c
filp_close(file, NULL);
```

### 6. 获取和设置文件偏移量：`vfs_llseek`

文件的读写偏移量可以通过 `vfs_llseek` 函数来获取和设置。

```c
loff_t vfs_llseek(struct file *file, loff_t offset, int whence);
```

- `file`：文件结构指针。
- `offset`：偏移量。
- `whence`：基准位置，如 `SEEK_SET`、`SEEK_CUR`、`SEEK_END`。

示例：

```c
loff_t new_pos;
new_pos = vfs_llseek(file, 0, SEEK_END); // 移动到文件末尾
```

### 7. 不需要 get_fs 与 set_fs 的保护说明

在旧版本的 Linux 内核中，内核与用户态之间的内存空间隔离机制要求在进行文件读写时使用 `get_fs()` 和 `set_fs()` 函数来切换内存访问权限。这些函数将内核代码的内存访问限制临时解除，允许访问用户态内存地址。

但是，在现代的 Linux 内核（从 5.10 版本起），已经废除了这种方式，内核提供的 `kernel_read` 和 `kernel_write` 等接口不再需要 `get_fs` 和 `set_fs` 的切换。这些 API 只在内核空间中工作，不再涉及用户空间的内存访问，因此没有了跨空间访问的风险，开发者不需要手动进行切换保护。

这大大简化了内核中文件操作的代码逻辑，并减少了因跨空间访问而导致的安全隐患。

### 8. 综合示例

下面是一个完整的内核模块示例，它打开一个文件，读取其中的内容，写回一些数据并关闭文件。

```c
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static int __init file_rw_init(void) {
    struct file *file;
    char *buf;
    loff_t pos = 0;
    ssize_t ret;

    // 打开文件
    file = filp_open("/path/to/file", O_RDWR, 0);
    if (IS_ERR(file)) {
        printk("Failed to open file\n");
        return PTR_ERR(file);
    }

    // 分配缓冲区
    buf = kmalloc(128, GFP_KERNEL);
    if (!buf) {
        filp_close(file, NULL);
        return -ENOMEM;
    }

    // 读取文件
    ret = kernel_read(file, buf, 128, &pos);
    if (ret >= 0) {
        printk("Read %zd bytes from file: %s\n", ret, buf);
    } else {
        printk("Failed to read file\n");
    }

    // 写入文件
    pos = 0;
    ret = kernel_write(file, "Hello, kernel!", 14, &pos);
    if (ret >= 0) {
        printk("Wrote %zd bytes to file\n", ret);
    } else {
        printk("Failed to write file\n");
    }

    // 关闭文件
    filp_close(file, NULL);
    kfree(buf);
    return 0;
}

static void __exit file_rw_exit(void) {
    printk("File read/write module exited\n");
}

module_init(file_rw_init);
module_exit(file_rw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple file read/write kernel module");
```

### 总结

在 Linux 内核中进行文件操作，主要依赖 `filp_open`、`kernel_read`、`kernel_write` 等函数完成。与用户态不同，内核中需要通过 `struct file` 和 `vfs` 接口来操作文件。现代内核中已不再需要 `get_fs` 和 `set_fs` 的切换保护，进一步简化了文件操作代码。正确处理文件的打开、关闭，以及内核内存的分配和释放，是避免内核崩溃或其他问题的关键。

原文链接：https://blog.csdn.net/sz66cm/article/details/142653909