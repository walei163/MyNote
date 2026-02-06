# Python Logging 完全指南：从入门到最佳实践

## 背景与动机

### 从 print() 说起

很多初学者（包括我自己）在写 Python 代码时，习惯用 print() 来调试：

```python
def train_model(data):
    print("开始训练...")
    print(f"数据大小: {len(data)}")
    

    for epoch in range(10):
        print(f"Epoch {epoch}")
        loss = compute_loss(data)
        print(f"Loss: {loss}")
    
    print("训练完成！")
```

这种方式在小项目中没问题，但随着项目变大，你会遇到这些困扰：

❌ 无法控制输出级别：调试信息和错误信息混在一起
❌ 难以保存日志：print() 只输出到控制台，无法持久化
❌ 无法追踪来源：不知道是哪个模块、哪个函数输出的
❌ 生产环境污染：调试信息会在生产环境中显示
❌ 性能问题：大量 print() 会拖慢程序

### Logging 的诞生

Python 的 logging 模块就是为了解决这些问题而设计的。它提供了：

✅ 分级日志：DEBUG、INFO、WARNING、ERROR、CRITICAL
✅ 灵活输出：控制台、文件、网络、邮件等
✅ 格式化：时间戳、模块名、行号等
✅ 性能优化：延迟求值、异步写入
✅ 生产就绪：线程安全、进程安全

---

## 为什么需要 Logging？

### 场景 1：调试复杂系统

假设你在训练一个深度学习模型，训练了 10 小时后崩溃了：

```python
#使用 print()

print("Loading data...")  # 这条看到了
print("Building model...")  # 这条看到了

#... 10 小时后 ...

#💥 程序崩溃，之前的输出都丢失了！

#使用 logging

import logging
logging.basicConfig(
    level=logging.INFO,
    filename='training.log',  # 保存到文件
    format='%(asctime)s - %(levelname)s - %(message)s'
)

logger = logging.getLogger(__name__)
logger.info("Loading data...")
logger.info("Building model...")

#... 10 小时后崩溃 ...

#✅ 日志已保存到文件，可以查看崩溃前的状态！

### 
```

### 场景 2：生产环境监控

在生产环境中，你需要：

- 记录所有错误，但不显示调试信息
- 将日志发送到监控系统场景 2：生产环境监控
- 按日期轮转日志文件

```python
import logging
from logging.handlers import RotatingFileHandler

#生产环境配置

logger = logging.getLogger('myapp')
logger.setLevel(logging.WARNING)  # 只记录警告及以上

#文件 handler：自动轮转

file_handler = RotatingFileHandler(
    'app.log',
    maxBytes=10*1024*1024,  # 10MB
    backupCount=5  # 保留 5 个备份
)
logger.addHandler(file_handler)

#现在只有重要信息会被记录

logger.debug("这不会被记录")
logger.warning("这会被记录")
logger.error("这也会被记录")
```

---

## Logging 核心概念

### 1、日志级别（Log Levels）

Python logging 有 5 个标准级别：

| 级别     | 数值 | 使用场景             | 示例               |
| -------- | ---- | -------------------- | ------------------ |
| DEBUG    | 10   | 详细的调试信息       | 变量值、函数调用栈 |
| INFO     | 20   | 一般信息             | 程序启动、配置加载 |
| WARNING  | 30   | 警告信息（默认级别） | 使用了废弃的 API   |
| ERROR    | 40   | 错误信息             | 操作失败但程序继续 |
| CRITICAL | 50   | 严重错误             | 程序即将崩溃       |

```python
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)  # 设置最低级别

logger.debug("调试信息：变量 x = 10")
logger.info("程序启动成功")
logger.warning("配置文件未找到，使用默认配置")
logger.error("无法连接数据库")
logger.critical("内存不足，程序即将退出")
```



### 2、四大组件

Logging 系统由 4 个核心组件构成：

```sh
Logger (记录器)
   ↓
Filter (过滤器) - 可选
   ↓
Handler (处理器)
   ↓
Formatter (格式化器)
```

#### Logger（记录器）

- 应用程序直接使用的接口
- 通过 logging.getLogger(name) 获取
- 有层级关系：myapp.module1.submodule

#### Handler（处理器）

- 决定日志输出到哪里
- 常用类型：
  - StreamHandler：输出到控制台
  - FileHandler：输出到文件
  - RotatingFileHandler：轮转文件
  - SMTPHandler：发送邮件

#### Formatter（格式化器）

- 决定日志的格式
- 常用占位符：
  - %(asctime)s：时间
  - %(name)s：logger 名称
  - %(levelname)s：日志级别
  - %(message)s：日志消息
  - %(filename)s：文件名
  - %(lineno)d：行号

#### Filter（过滤器）

- 更细粒度的日志过滤（较少使用）

---

## 基础用法

### 1、最简单的用法

```python
import logging

#快速配置

logging.basicConfig(level=logging.INFO)

#直接使用

logging.info("这是一条信息")
logging.warning("这是一条警告")
logging.error("这是一条错误")
输出：

INFO:root:这是一条信息
WARNING:root:这是一条警告
ERROR:root:这是一条错误
```

### 2、使用 Logger 对象（推荐）

```python
import logging

#创建 logger

logger = logging.getLogger(__name__)  # 使用模块名作为 logger 名称
logger.setLevel(logging.DEBUG)

#添加控制台 handler

console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)

#设置格式

formatter = logging.Formatter(
    '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
console_handler.setFormatter(formatter)

#添加 handler

logger.addHandler(console_handler)

#使用

logger.info("程序启动")
logger.debug("这条不会显示，因为 handler 级别是 INFO")
```

输出：

```sh
2025-11-08 17:10:23,456 - __main__ - INFO - 程序启动
```



### 3、同时输出到控制台和文件

```python
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

#控制台 handler

console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)

#文件 handler

file_handler = logging.FileHandler('app.log', encoding='utf-8')
file_handler.setLevel(logging.DEBUG)

#格式

formatter = logging.Formatter(
    '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
console_handler.setFormatter(formatter)
file_handler.setFormatter(formatter)

#添加 handlers

logger.addHandler(console_handler)
logger.addHandler(file_handler)

#使用

logger.debug("调试信息（只写入文件）")
logger.info("一般信息（控制台和文件都有）")
logger.error("错误信息（控制台和文件都有）")
```



### 4、使用 basicConfig 快速配置

```python
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S',
    handlers=[
        logging.StreamHandler(),  # 控制台
        logging.FileHandler('app.log', encoding='utf-8')  # 文件
    ]
)

logger = logging.getLogger(__name__)
logger.info("使用 basicConfig 配置")
```

> [!note]
>
> ⚠️ 注意：basicConfig 只在第一次调用时生效，且只配置根 logger。

---

## 进阶用法

### 1、Logger 层级结构

```python
import logging

#创建层级 logger

logger_parent = logging.getLogger('myapp')
logger_child = logging.getLogger('myapp.module1')
logger_grandchild = logging.getLogger('myapp.module1.submodule')

#配置父 logger

logger_parent.setLevel(logging.INFO)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter('%(name)s - %(message)s'))
logger_parent.addHandler(handler)

#子 logger 会继承父 logger 的配置

logger_child.info("来自 module1")  # 会输出
logger_grandchild.info("来自 submodule")  # 会输出

#阻止向上传播

logger_child.propagate = False
logger_child.info("这条不会传播到父 logger")
```



### 2、异常日志

```python
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter('%(levelname)s - %(message)s'))
logger.addHandler(handler)

try:
    result = 10 / 0
except ZeroDivisionError:
    # 方法 1：记录异常信息
    logger.error("发生除零错误")
    

    # 方法 2：记录异常堆栈（推荐）
    logger.exception("发生除零错误")
    
    # 方法 3：使用 exc_info
    logger.error("发生除零错误", exc_info=True)

```

输出：

```sh
ERROR - 发生除零错误
ERROR - 发生除零错误
Traceback (most recent call last):
  File "test.py", line 9, in <module>
    result = 10 / 0
ZeroDivisionError: division by zero
```



### 3、格式化日志消息

```python
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
logger.addHandler(handler)

#方法 1：使用 % 格式化（推荐，延迟求值）

logger.info("用户 %s 登录成功", "张三")

#方法 2：使用 f-string（不推荐，立即求值）

user = "李四"
logger.info(f"用户 {user} 登录成功")

#方法 3：使用 .format()

logger.info("用户 {} 登录成功".format("王五"))

#方法 4：使用字典

logger.info("用户 %(user)s 从 %(ip)s 登录", {'user': '赵六', 'ip': '192.168.1.1'})
```

> [!tip]
>
> ⚠️ 性能提示：使用 % 格式化，因为如果日志级别不够，字符串不会被格式化。

### 4、自定义格式

```python
import logging

#详细格式（开发环境）

dev_formatter = logging.Formatter(
    '%(asctime)s - %(name)s - %(levelname)s - '
    '[%(filename)s:%(lineno)d] - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

#简洁格式（生产环境）

prod_formatter = logging.Formatter(
    '%(asctime)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

#JSON 格式（用于日志收集系统）

import json

class JsonFormatter(logging.Formatter):
    def format(self, record):
        log_data = {
            'timestamp': self.formatTime(record),
            'level': record.levelname,
            'logger': record.name,
            'message': record.getMessage(),
            'file': record.filename,
            'line': record.lineno
        }
        return json.dumps(log_data, ensure_ascii=False)

logger = logging.getLogger(__name__)
handler = logging.StreamHandler()
handler.setFormatter(JsonFormatter())
logger.addHandler(handler)
logger.setLevel(logging.INFO)

logger.info("这是 JSON 格式的日志")
```

输出：

```sh
{"timestamp": "2025-11-08 17:10:23,456", "level": "INFO", "logger": "__main__", "message": "这是 JSON 格式的日志", "file": "test.py", "line": 25}
```



### 5、日志轮转

```python
import logging
from logging.handlers import RotatingFileHandler, TimedRotatingFileHandler

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

#按大小轮转

size_handler = RotatingFileHandler(
    'app.log',
    maxBytes=10*1024*1024,  # 10MB
    backupCount=5,  # 保留 5 个备份文件
    encoding='utf-8'
)logger.addHandler(time_handler)  # 二选一

#按时间轮转

time_handler = TimedRotatingFileHandler(
    'app.log',
    when='midnight',  # 每天午夜轮转
    interval=1,  # 间隔 1 天
    backupCount=30,  # 保留 30 天
    encoding='utf-8'
)

#when 参数选项：

#'S' - 秒

#'M' - 分钟

#'H' - 小时

#'D' - 天

#'midnight' - 午夜

'W0'-'W6' - 星期几（0=星期一）

formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
size_handler.setFormatter(formatter)
time_handler.setFormatter(formatter)

logger.addHandler(size_handler)

#logger.addHandler(time_handler)  # 二选一

for i in range(1000000):
    logger.info(f"日志消息 {i}")
```



---

## 实战案例

### 案例 1：深度学习模型训练时的日志

```python
import logging
from logging.handlers import RotatingFileHandler
import sys

def setup_training_logger(log_dir='logs', log_file='training.log'):
    """设置训练日志配置"""
    
    # 创建 logger
    logger = logging.getLogger('training')
    logger.setLevel(logging.DEBUG)
    
    # 避免重复添加 handler
    if logger.handlers:
        return logger
    
    # 控制台 handler - 只显示 INFO 及以上
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.INFO)
    console_formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%H:%M:%S'
    )
    console_handler.setFormatter(console_formatter)
    
    # 文件 handler - 记录所有 DEBUG 及以上
    import os
    os.makedirs(log_dir, exist_ok=True)
    file_handler = RotatingFileHandler(
        os.path.join(log_dir, log_file),
        maxBytes=50*1024*1024,  # 50MB
        backupCount=3,
        encoding='utf-8'
    )
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    file_handler.setFormatter(file_formatter)
    
    # 添加 handlers
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)
    
    return logger

# 使用
logger = setup_training_logger()

logger.info("=" * 50)
logger.info("开始训练")
logger.info("=" * 50)

# 训练循环
for epoch in range(10):
    logger.info(f"Epoch {epoch + 1}/10")
    
    # 详细的调试信息只写入文件
    logger.debug(f"学习率: 0.001")
    logger.debug(f"批次大小: 32")
    
    # 训练指标显示在控制台和文件
    train_loss = 0.5 - epoch * 0.03
    logger.info(f"  训练损失: {train_loss:.4f}")
    
    val_loss = 0.6 - epoch * 0.04
    logger.info(f"  验证损失: {val_loss:.4f}")
    
    if val_loss < 0.2:
        logger.warning(f"验证损失过低，可能过拟合")

logger.info("训练完成！")
```

### 案例 2：多模块项目

#### 项目结构：

```sh
myproject/
├── __init__.py
├── config.py
├── data/
│   ├── __init__.py
│   └── loader.py
├── models/
│   ├── __init__.py
│   └── vae.py
└── train.py
```

#### config.py：

```py
import logging
import sys
from pathlib import Path

def setup_logger(name='myproject', log_dir='logs', level=logging.INFO):
    """统一的日志配置"""
    
    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)
    
    if logger.handlers:
        return logger
    
    # 创建日志目录
    log_dir = Path(log_dir)
    log_dir.mkdir(exist_ok=True)
    
    # 控制台 handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(level)
    console_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        datefmt='%H:%M:%S'
    )
    console_handler.setFormatter(console_formatter)
    
    # 文件 handler
    file_handler = logging.FileHandler(
        log_dir / 'app.log',
        encoding='utf-8'
    )
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s'
    )
    file_handler.setFormatter(file_formatter)
    
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)
    
    return logger
```

#### data/loader.py：

```python
import logging

# 使用模块名创建 logger
logger = logging.getLogger(__name__)  # 'myproject.data.loader'

def load_data(path):
    logger.info(f"开始加载数据: {path}")
    
    try:
        # 加载数据逻辑
        logger.debug(f"数据路径: {path}")
        data = []  # 实际加载逻辑
        logger.info(f"成功加载 {len(data)} 条数据")
        return data
    except FileNotFoundError:
        logger.error(f"文件未找到: {path}")
        raise
    except Exception as e:
        logger.exception(f"加载数据时发生错误")
        raise
```

#### models/vae.py：

```py
import logging

logger = logging.getLogger(__name__)  # 'myproject.models.vae'

class VAE:
    def __init__(self, latent_dim=10):
        self.latent_dim = latent_dim
        logger.info(f"初始化 VAE 模型，潜在维度: {latent_dim}")
    
    def train(self, data):
        logger.info("开始训练 VAE")
        
        for epoch in range(10):
            loss = self._train_epoch(data, epoch)
            logger.info(f"Epoch {epoch}: loss = {loss:.4f}")
            
            if loss < 0.01:
                logger.warning("损失过低，可能过拟合")
    
    def _train_epoch(self, data, epoch):
        logger.debug(f"训练 epoch {epoch}")
        return 0.5 - epoch * 0.05
```

#### train.py：

```python
import logging
from myproject.config import setup_logger
from myproject.data.loader import load_data
from myproject.models.vae import VAE

# 设置日志
setup_logger(level=logging.INFO)

# 获取主 logger
logger = logging.getLogger('myproject')

def main():
    logger.info("=" * 50)
    logger.info("程序启动")
    logger.info("=" * 50)
    
    # 加载数据（会使用 myproject.data.loader 的 logger）
    data = load_data('data.csv')
    
    # 训练模型（会使用 myproject.models.vae 的 logger）
    model = VAE(latent_dim=20)
    model.train(data)
    
    logger.info("程序结束")

if __name__ == '__main__':
    main()
```

输出：

```sh
17:10:23 - myproject - INFO - ==================================================
17:10:23 - myproject - INFO - 程序启动
17:10:23 - myproject - INFO - ==================================================
17:10:23 - myproject.data.loader - INFO - 开始加载数据: data.csv
17:10:23 - myproject.data.loader - INFO - 成功加载 0 条数据
17:10:23 - myproject.models.vae - INFO - 初始化 VAE 模型，潜在维度: 20
17:10:23 - myproject.models.vae - INFO - 开始训练 VAE
17:10:23 - myproject.models.vae - INFO - Epoch 0: loss = 0.5000
17:10:23 - myproject.models.vae - INFO - Epoch 1: loss = 0.4500
...
```

## 案例 3：配置文件方式

### logging.conf：

```ini
[loggers]
keys=root,myapp

[handlers]
keys=consoleHandler,fileHandler

[formatters]
keys=simpleFormatter,detailedFormatter

[logger_root]
level=WARNING
handlers=consoleHandler

[logger_myapp]
level=DEBUG
handlers=consoleHandler,fileHandler
qualname=myapp
propagate=0

[handler_consoleHandler]
class=StreamHandler
level=INFO
formatter=simpleFormatter
args=(sys.stdout,)

[handler_fileHandler]
class=FileHandler
level=DEBUG
formatter=detailedFormatter
args=('app.log', 'a', 'utf-8')

[formatter_simpleFormatter]
format=%(asctime)s - %(levelname)s - %(message)s
datefmt=%H:%M:%S

[formatter_detailedFormatter]
format=%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s
datefmt=%Y-%m-%d %H:%M:%S
```

使用配置文件：

```python
import logging
import logging.config

# 加载配置
logging.config.fileConfig('logging.conf')

# 使用 logger
logger = logging.getLogger('myapp')
logger.info("使用配置文件的日志")
```

---

## 最佳实践

### 1、命名规范

```python
import logging

# ✅ 推荐：使用 __name__
logger = logging.getLogger(__name__)

# ❌ 不推荐：硬编码名称
logger = logging.getLogger('mylogger')

# ✅ 推荐：使用层级结构
logger = logging.getLogger('myapp.module.submodule')
```

> [!important]
>
> 原因：使用 __name__ 可以自动获取模块的完整路径，便于追踪日志来源。

### 2、避免重复配置

```python
import logging

def get_logger(name):
    """获取或创建 logger"""
    logger = logging.getLogger(name)
    
    # ✅ 检查是否已有 handler
    if not logger.handlers:
        handler = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(logging.INFO)
    
    return logger
```



### 3、使用延迟格式化

```python
import logging

logger = logging.getLogger(__name__)

# ✅ 推荐：延迟格式化
logger.debug("变量值: %s, %s", expensive_function1(), expensive_function2())

# ❌ 不推荐：立即格式化
logger.debug(f"变量值: {expensive_function1()}, {expensive_function2()}")
```

> [!important]
>
> 原因：如果日志级别不够，第一种方式不会调用 expensive_function()。

### 4、异常处理

```python
import logging

logger = logging.getLogger(__name__)

try:
    risky_operation()
except Exception as e:
    # ✅ 推荐：使用 exception() 记录堆栈
    logger.exception("操作失败")
    
    # ❌ 不推荐：只记录错误消息
    logger.error(f"操作失败: {e}")
```



### 5、生产环境配置

```python
import logging
import os

def setup_production_logger():
    """生产环境日志配置"""
    
    logger = logging.getLogger('myapp')
    
    # 根据环境变量设置级别
    level = os.getenv('LOG_LEVEL', 'WARNING')
    logger.setLevel(getattr(logging, level))
    
    # 只输出到文件
    from logging.handlers import TimedRotatingFileHandler
    handler = TimedRotatingFileHandler(
        '/var/log/myapp/app.log',
        when='midnight',
        backupCount=30,
        encoding='utf-8'
    )
    
    # 简洁格式
    formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s - %(message)s'
    )
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    
    return logger
```



### 6、性能优化

```python
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# ✅ 推荐：检查级别
if logger.isEnabledFor(logging.DEBUG):
    expensive_data = compute_expensive_debug_info()
    logger.debug("调试信息: %s", expensive_data)

# ❌ 不推荐：总是计算
logger.debug("调试信息: %s", compute_expensive_debug_info())
```



### 7、多进程日志

```python
import logging
from logging.handlers import QueueHandler, QueueListener
from multiprocessing import Queue, Process
import queue

def worker_process(log_queue):
    """工作进程"""
    # 配置 QueueHandler
    logger = logging.getLogger('worker')
    logger.setLevel(logging.INFO)
    logger.addHandler(QueueHandler(log_queue))
    
    logger.info("工作进程启动")
    # 执行任务
    logger.info("任务完成")

def main():
    # 创建日志队列
    log_queue = Queue()
    
    # 配置主进程的 handler
    console_handler = logging.StreamHandler()
    console_handler.setFormatter(
        logging.Formatter('%(asctime)s - %(processName)s - %(message)s')
    )
    
    # 启动 QueueListener
    listener = QueueListener(log_queue, console_handler)
    listener.start()
    
    # 启动工作进程
    processes = []
    for i in range(4):
        p = Process(target=worker_process, args=(log_queue,))
        p.start()
        processes.append(p)
    
    # 等待完成
    for p in processes:
        p.join()
    
    listener.stop()

if __name__ == '__main__':
    main()
```



### 8、上下文信息

```python
import logging
from contextvars import ContextVar

# 使用 ContextVar 存储请求 ID
request_id_var = ContextVar('request_id', default='N/A')

class ContextFilter(logging.Filter):
    """添加上下文信息到日志"""
    def filter(self, record):
        record.request_id = request_id_var.get()
        return True

# 配置
logger = logging.getLogger(__name__)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter(
    '%(asctime)s - [%(request_id)s] - %(message)s'
))
handler.addFilter(ContextFilter())
logger.addHandler(handler)
logger.setLevel(logging.INFO)

# 使用
def handle_request(request_id):
    request_id_var.set(request_id)
    logger.info("处理请求")
    logger.info("请求完成")

handle_request("REQ-12345")
handle_request("REQ-67890")
```

输出：

```sh
2025-11-08 17:10:23,456 - [REQ-12345] - 处理请求
2025-11-08 17:10:23,457 - [REQ-12345] - 请求完成
2025-11-08 17:10:23,458 - [REQ-67890] - 处理请求
2025-11-08 17:10:23,459 - [REQ-67890] - 请求完成
```



---

## 总结

### 核心要点

1. 使用 logging.getLogger(__name__) 创建 logger
2. 设置合适的日志级别：开发用 DEBUG，生产用 WARNING
3. 使用 Handler 分离输出：控制台 + 文件
4. 格式化日志：包含时间、模块名、级别、消息
5. 异常处理用 logger.exception()
6. 避免重复配置：检查 logger.handlers
7. 生产环境用日志轮转：避免文件过大

### 快速模板

```python
import logging
import sys
from pathlib import Path

def setup_logger(name='myapp', log_dir='logs', level=logging.INFO):
    """通用日志配置模板"""
    

    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)
    
    if logger.handlers:
        return logger
    
    # 创建日志目录
    Path(log_dir).mkdir(exist_ok=True)
    
    # 控制台 handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(level)
    console_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        datefmt='%H:%M:%S'
    )
    console_handler.setFormatter(console_formatter)
    
    # 文件 handler
    from logging.handlers import RotatingFileHandler
    file_handler = RotatingFileHandler(
        f'{log_dir}/app.log',
        maxBytes=10*1024*1024,
        backupCount=5,
        encoding='utf-8'
    )
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s'
    )
    file_handler.setFormatter(file_formatter)
    
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)
    
    return logger

#使用

if __name__ == '__main__':
    logger = setup_logger()
    logger.debug("调试信息")
	logger.info("一般信息")
	logger.warning("警告信息")
	logger.error("错误信息")

	try:
    	1 / 0
	except:
    	logger.exception("发生异常")
```

---

## 参考资源

[Python 官方文档 - logging](https://link.zhihu.com/?target=https%3A//docs.python.org/3/library/logging.html)
[Logging HOWTO](https://link.zhihu.com/?target=https%3A//docs.python.org/3/howto/logging.html)
[Logging Cookbook](https://link.zhihu.com/?target=https%3A//docs.python.org/3/howto/logging.html)
发布于 2025-11-08 17:41・中国香港

原文链接：https://zhuanlan.zhihu.com/p/1970543051182146321?share_code=GGTKMN1MbP0o&utm_psn=1974615823625778747