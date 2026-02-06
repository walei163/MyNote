# Python IPC 3：共享内存示例

2025-11-14 22:19·[熊猫学功夫](https://www.toutiao.com/c/user/token/MS4wLjABAAAAKEWUe3SQrlX3YUwkhWWLomfTJxiJh3ab5vhfdAEp06s/?source=tuwen_detail)

## 应用场景

多个进程之间高效共享或交换大块数据，避免序列化与数据拷贝开销。

## 共享内存核心概念

- 共享内存在底层是一段由内核管理的物理内存。
- 多个进程各自把这段物理内存映射到自己的虚拟地址空间。
- 所有进程读写的是自己的虚拟地址，但最终作用的是同一块物理内存。

## 使用共享内存的一般步骤

1、A进程创建共享内存，返回共享内存的名称。

2、B进程附加到共享内存。

3、A进程和B进程读写共享内存数据。

4、A进程释放共享内存。

## 示例1: 共享内存+依次写入数据

场景描述：进程A创建共享内存，写入数据 [1, 2, 3, 4, 5]；进程B修改共享内存，给每个元素+10，得到 [11 12 13 14 15]。

```python
import multiprocessing
from multiprocessing import shared_memory

import numpy as np


def owner_func():
    # 模拟数据
    data = [1, 2, 3, 4, 5]

    # 创建共享内存数组
    data = np.array(data, dtype=np.int64)
    shm = shared_memory.SharedMemory(create=True, size=data.nbytes)

    # 将原始数据写入共享内存
    shared_array = np.ndarray(data.shape, dtype=data.dtype, buffer=shm.buf)
    shared_array[:] = data[:]

    return shm, shm.name, len(data), shared_array


def user_func(shm_name, length):
    # 连接到同名的共享内存
    shm = shared_memory.SharedMemory(name=shm_name)
    shared_array = np.ndarray((length,), dtype=np.int64, buffer=shm.buf)

    # 修改数组内容
    for i in range(length):
        shared_array[i] += 10  # 每个元素加 10

    # 关闭本地句柄
    shm.close()


if __name__ == '__main__':
    # 进程A创建共享内存并写入数据: [1, 2, 3, 4, 5]
    owner_shm, shm_name, length, shared_memory = owner_func()

    # 输出: [1, 2, 3, 4, 5]
    print(shared_memory)

    # 进程B修改共享内存数据: 每个元素+10
    p = multiprocessing.Process(target=user_func, args=(shm_name, length))
    p.start()
    p.join()

    # 输出: [11 12 13 14 15]
    print(shared_memory)

    # 进程A关闭本地句柄、释放共享内存
    owner_shm.close()
    owner_shm.unlink()
```

## 示例2: 共享内存+进程锁

场景描述：3个进程同时修改共享内存的值，每个进程修改3次，每次修改加1。共享内存初始值为0，最终值为9。

```python
import multiprocessing
from multiprocessing import Process, shared_memory, Lock
import time
import os

def worker(shm_name, lock, loop_times):
    shm = shared_memory.SharedMemory(name=shm_name)
    for _ in range(loop_times):
        with lock:
            shm.buf[0] += 1
            print(f"[{os.getpid()} - {multiprocessing.current_process().name}] 写入结束，值={shm.buf[0]}")
            time.sleep(0.1)
    shm.close()

if __name__ == "__main__":
    shm = shared_memory.SharedMemory(create=True, size=1)
    shm.buf[0] = 0

    lock = Lock()
    processes = [
        Process(target=worker,
                args=(shm.name, lock, 3),
                name=f"Worker-{i}")
        for i in range(1, 4)
    ]

    for p in processes:
        p.start()

    for p in processes:
        p.join()

    print("最终值 = ", shm.buf[0])
    shm.close()
    shm.unlink()
```

运行结果示例：

> [11956 - Worker-3] 写入结束，值=1
>
> [14440 - Worker-1] 写入结束，值=2
>
> [10712 - Worker-2] 写入结束，值=3
>
> [11956 - Worker-3] 写入结束，值=4
>
> [14440 - Worker-1] 写入结束，值=5
>
> [10712 - Worker-2] 写入结束，值=6
>
> [11956 - Worker-3] 写入结束，值=7
>
> [14440 - Worker-1] 写入结束，值=8
>
> [10712 - Worker-2] 写入结束，值=9
>
> 最终值 = 9