# Python 文件路径处理：os vs pathlib 完全指南



[TOC]

注：在 Python 中处理文件路径时，我们可能会遇到两种主流方式：传统的 os 模块和现代的 pathlib 模块。本文将通过简单的示例帮你理解它们的区别，并指导你在实际项目中如何选择。

---

## 一、快速对比

| 特性     | os / os.path        | pathlib            |
| -------- | ------------------- | ------------------ |
| 引入时间 | Python 1.x（1990s） | Python 3.4（2014） |
| 编程范式 | 函数式              | 面向对象           |
| 返回类型 | 字符串              | Path 对象          |
| 跨平台   | 需手动处理          | 自动处理           |
| 可读性   | 中等                | 优秀               |
| 推荐度   | ⭐⭐                  | ⭐⭐⭐⭐⭐              |

---

## 二、基础操作对比

### 1、路径拼接

#### 使用 os.path：

```python
import os

#拼接路径

data_dir = "/home/user/data"
file_name = "experiment_results.csv"
file_path = os.path.join(data_dir, file_name)
print(file_path)

#输出: /home/user/data/experiment_results.csv
```

#### 使用 pathlib：

```python
from pathlib import Path

#拼接路径（使用 / 操作符）

data_dir = Path("/home/user/data")
file_name = "experiment_results.csv"
file_path = data_dir / file_name
print(file_path)

#输出: /home/user/data/experiment_results.csv
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 使用 / 操作符更直观，接近自然语言
> - os.path.join() 需要记住函数名，稍显冗长

---

### 2、获取文件信息

#### 使用 os.path：

```python
import os

file_path = "/home/user/data/results.csv"

#检查文件是否存在

exists = os.path.exists(file_path)

#获取文件名

file_name = os.path.basename(file_path)  # "results.csv"

#获取目录名

dir_name = os.path.dirname(file_path)  # "/home/user/data"

#获取文件扩展名

_, ext = os.path.splitext(file_path)  # ".csv"

#获取绝对路径

abs_path = os.path.abspath(file_path)
```

#### 使用 pathlib：

```python
from pathlib import Path

file_path = Path("/home/user/data/results.csv")

#检查文件是否存在

exists = file_path.exists()

#获取文件名

file_name = file_path.name  # "results.csv"

#获取目录名

dir_name = file_path.parent  # Path("/home/user/data")

#获取文件扩展名

ext = file_path.suffix  # ".csv"

#获取绝对路径

abs_path = file_path.resolve()
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 使用属性（.name, .suffix）而非函数，更符合面向对象思维
> - pathlib 的方法命名更直观（exists() vs os.path.exists()）

---

### 3、创建目录

#### 使用 os：

```python
import os

output_dir = "/home/user/output"

#创建单层目录

if not os.path.exists(output_dir):
    os.mkdir(output_dir)

#创建多层目录

nested_dir = "/home/user/output/exp1/results"
if not os.path.exists(nested_dir):
    os.makedirs(nested_dir)
```

#### 使用 pathlib：

```python
from pathlib import Path

output_dir = Path("/home/user/output")

#创建单层目录

output_dir.mkdir(exist_ok=True)

#创建多层目录

nested_dir = Path("/home/user/output/exp1/results")
nested_dir.mkdir(parents=True, exist_ok=True)
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 的 mkdir(parents=True, exist_ok=True) 一行搞定，无需手动检查
> - os 需要先判断路径是否存在，代码更冗长

---

### 4、遍历目录

#### 使用 os：

```python
import os

data_dir = "/home/user/data"

#列出所有文件

for file_name in os.listdir(data_dir):
    file_path = os.path.join(data_dir, file_name)
    if os.path.isfile(file_path):
        print(file_path)

#递归遍历所有 CSV 文件

for root, dirs, files in os.walk(data_dir):
    for file_name in files:
        if file_name.endswith(".csv"):
            print(os.path.join(root, file_name))
```



#### 使用 pathlib：

```python
from pathlib import Path

data_dir = Path("/home/user/data")

#列出所有文件

for file_path in data_dir.iterdir():
    if file_path.is_file():
        print(file_path)

#递归遍历所有 CSV 文件

for file_path in data_dir.rglob("*.csv"):
    print(file_path)
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 的 rglob("*.csv") 一行实现递归搜索，简洁优雅
>
> - os.walk() 需要嵌套循环，代码复杂
>
> - pathlib 的 rglob("*.csv") 一行实现递归搜索，简洁优雅
>
>   os.walk() 需要嵌套循环，代码复杂

### 5、读写文件

#### 使用 os + open()：

```python
import os

file_path = os.path.join("/home/user", "data.txt")

#写入文件

with open(file_path, "w") as f:
    f.write("Hello, World!")

#读取文件

with open(file_path, "r") as f:
    content = f.read()

#### 
```

#### 使用 pathlib：

```python
from pathlib import Path

file_path = Path("/home/user") / "data.txt"

#写入文件

file_path.write_text("Hello, World!")

#读取文件

content = file_path.read_text()

#也支持二进制操作

file_path.write_bytes(b"Binary data")
binary_content = file_path.read_bytes()
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 提供 read_text() / write_text() 快捷方法，适合简单场景
> - 对于大文件或需要精细控制，仍推荐使用 open() 上下文管理器

---

## 三、实战案例：数据分析项目

假设你正在做一个机器学习项目，需要处理以下目录结构：

```sh
project/
├── data/
│   ├── raw/
│   │   ├── train.csv
│   │   └── test.csv
│   └── processed/
├── models/
└── results/
```



### 使用 os 的实现：

```python
import os
import pandas as pd

#定义路径

project_dir = "/home/user/project"
raw_data_dir = os.path.join(project_dir, "data", "raw")
processed_data_dir = os.path.join(project_dir, "data", "processed")
models_dir = os.path.join(project_dir, "models")

#创建目录

if not os.path.exists(processed_data_dir):
    os.makedirs(processed_data_dir)

#读取所有 CSV 文件

for file_name in os.listdir(raw_data_dir):
    if file_name.endswith(".csv"):
        file_path = os.path.join(raw_data_dir, file_name)
        df = pd.read_csv(file_path)
        
# 处理数据...
    
# 保存处理后的数据
output_path = os.path.join(processed_data_dir, f"processed_{file_name}")
df.to_csv(output_path, index=False)
```

---

### 使用 pathlib 的实现：

```python
from pathlib import Path
import pandas as pd

#定义路径

project_dir = Path("/home/user/project")
raw_data_dir = project_dir / "data" / "raw"
processed_data_dir = project_dir / "data" / "processed"
models_dir = project_dir / "models"

#创建目录

processed_data_dir.mkdir(parents=True, exist_ok=True)

#读取所有 CSV 文件

for file_path in raw_data_dir.glob("*.csv"):
    df = pd.read_csv(file_path)
    
# 处理数据...

# 保存处理后的数据
output_path = processed_data_dir / f"processed_{file_path.name}"
df.to_csv(output_path, index=False)
```

> [!tip]   
>
> 💡 对比：
>
> - pathlib 版本代码量减少约 30%
> - 路径拼接更直观（/ vs os.path.join()）
> - 不需要手动检查目录是否存在

---

## 四、跨平台兼容性

Windows vs Linux/Mac 路径差异
问题场景：

- Windows: C:\Users\user\data\file.csv

- Linux/Mac: /home/user/data/file.csv

### 使用 os.path 的处理：

```python
import os

#❌ 错误：硬编码路径分隔符

file_path = "data\\results\\output.csv"  # 只在 Windows 上工作

#✅ 正确：使用 os.path.join()

file_path = os.path.join("data", "results", "output.csv")  # 跨平台
```



---

### 使用 pathlib 的处理：

```python
from pathlib import Path

#✅ 自动处理跨平台

file_path = Path("data") / "results" / "output.csv"

#Windows: data\results\output.csv

#Linux:   data/results/output.csv
```

> [!tip]
>
> 💡 对比：
>
> - pathlib 自动处理不同操作系统的路径分隔符
> - os.path 也能跨平台，但需要始终使用 join()，容易出错

---

## 五、与第三方库的兼容性

### pandas、numpy 等库的支持

```python
from pathlib import Path
import pandas as pd
import numpy as np

file_path = Path("data") / "experiment.csv"

#✅ pandas 完全支持 Path 对象

df = pd.read_csv(file_path)
df.to_csv(file_path, index=False)

#✅ numpy 也支持

arr = np.loadtxt(file_path, delimiter=",")

#如果遇到不支持的旧库，转换为字符串

legacy_function(str(file_path))
```

> [!note]
>
> 结论： 现代库（pandas、numpy、matplotlib 等）都支持 Path 对象，无需担心兼容性。

---

## 六、性能对比

```python
import timeit
from pathlib import Path
import os

#测试路径拼接性能

def test_os():
    return os.path.join("data", "subdir", "file.csv")

def test_pathlib():
    return Path("data") / "subdir" / "file.csv"

print("os.path.join:", timeit.timeit(test_os, number=100000))
print("pathlib:", timeit.timeit(test_pathlib, number=100000))
```

> [!note]
>
> 结果：
>
> - os.path 略快（约 10-20%），因为是纯字符串操作
> - pathlib 稍慢，但差异在实际应用中可忽略不计（微秒级）
>
> 💡 结论： 除非你在循环中处理百万级路径操作，否则性能差异不重要。优先考虑代码可读性。

---

## 七、何时使用哪个？

### 推荐使用 pathlib：

✅ 所有新项目（Python 3.4+）
✅ 需要频繁操作路径（拼接、解析、遍历）
✅ 追求代码可读性和维护性
✅ 跨平台项目

### 仍可使用 os：

⚠️ 维护 Python 2 或旧代码
⚠️ 极端性能敏感场景（罕见）
⚠️ 团队约定使用 os

---

## 八、迁移指南：从 os 到 pathlib

| os / os.path        | pathlib                     |
| ------------------- | --------------------------- |
| os.path.join(a, b)  | Path(a) / b                 |
| os.path.basename(p) | Path(p).name                |
| os.path.dirname(p)  | Path(p).parent              |
| os.path.splitext(p) | Path(p).suffix              |
| os.path.exists(p)   | Path(p).exists()            |
| os.path.isfile(p)   | Path(p).is_file()           |
| os.path.isdir(p)    | Path(p).is_dir()            |
| os.path.abspath(p)  | Path(p).resolve()           |
| os.makedirs(p)      | Path(p).mkdir(parents=True) |
| os.listdir(p)       | Path(p).iterdir()           |
| os.walk(p)          | Path(p).rglob("*")          |

---

## 九、常见陷阱

### 1、Path 对象不是字符串

```python
from pathlib import Path

file_path = Path("data") / "file.csv"

#❌ 错误：不能直接用于字符串操作

print("Path: " + file_path)  # TypeError

#✅ 正确：转换为字符串

print("Path: " + str(file_path))
print(f"Path: {file_path}")  # f-string 自动转换
```

---

### 2、相对路径 vs 绝对路径

```python
from pathlib import Path

#相对路径

rel_path = Path("data/file.csv")
print(rel_path)  # data/file.csv

#转换为绝对路径

abs_path = rel_path.resolve()
print(abs_path)  # /home/user/project/data/file.csv
```

---

### 3、Windows 路径中的反斜杠

```python
from pathlib import Path

#❌ 错误：反斜杠需要转义

path = Path("C:\Users\data")  # SyntaxError

#✅ 正确方式：

path1 = Path("C:/Users/data")  # 推荐：使用正斜杠
path2 = Path(r"C:\Users\data")  # 使用原始字符串
path3 = Path("C:\\Users\\data")  # 转义反斜杠
```



---

## 十、总结

### pathlib 的优势：

1. 面向对象：路径是对象，方法更直观
2. 可读性强：/ 操作符比 join() 更自然
3. 功能丰富：内置大量便捷方法
4. 跨平台：自动处理不同操作系统差异
5. 现代化：Python 官方推荐的标准

### os 的优势：

1. 历史悠久：兼容所有 Python 版本
2. 性能略优：纯字符串操作更快（差异微小）
3. 广泛使用：大量旧代码和教程使用

---

## 最终建议

> [!note]
>
> 如果你正在开始新项目，毫不犹豫地选择 pathlib！

它会让你的代码更简洁、更易读、更易维护。虽然 os 仍然有效，但 pathlib 代表了 Python 文件路径处理的未来方向。

## 参考资源

[Python 官方文档 - pathlib](https://link.zhihu.com/?target=https%3A//docs.python.org/3/library/pathlib.html)
[Python 官方文档 - os.path](https://link.zhihu.com/?target=https%3A//docs.python.org/3/library/pathlib.html)
[PEP 428 - The pathlib module](https://link.zhihu.com/?target=https%3A//docs.python.org/3/library/pathlib.html)

---

Happy Coding! 🐍✨

原文链接：https://zhuanlan.zhihu.com/p/1962109569724572942?share_code=1jiEI9fbM8lO&utm_psn=1974265935675278888