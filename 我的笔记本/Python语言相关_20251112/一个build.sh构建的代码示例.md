# 一个build.sh构建的代码示例

## 2025-11-14：

这个代码做了更新：

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os

build_debug = False

build_items = (
    "exit",
    "all",
    "config",
    "kernel",
    "menuconfig",
    "saveconfig",
    "bootloader",
    "uboot_menuconfig",
    "uboot_saveconfig",
    "rootfs",
    "buildroot_menuconfig",
    "buildroot_saveconfig",
    "pack"
)

build_items_explain = (
    "退出程序",
    "全部构建",
    "配置build.sh编译环境",
    "编译内核",
    "内核编译配置",
    "保存内核编译配置",
    "编译引导程序bootloader",
    "uboot编译配置",
    "保存uboot编译配置",
    "编译buildroot根文件系统",
    "buildroot文件系统编译配置",
    "保存buildroot文件系统编译配置",
    "将全部编译结果打包pack"
)

build_cmd = "./build.sh"

# ANSI escape sequences (use \033 for portability)
red='\033[1;31m'
RED='\033[1;31m'
blue='\033[0;34m'
BLUE='\033[1;34m'
cyan='\033[0;36m'
CYAN='\033[1;36m'
green='\033[0;32m'
GREEN='\033[1;32m'
yellow='\033[1;33m'
YELLOW='\033[1;33m'
PURPLE='\033[1;35m'
purple='\033[0;35m'
error='\033[1;41m'
warn='\033[1;43m'
nc='\033[0m'
    
def print_prefix_colored(prefix, message, color):
    print(f"{prefix}", end='', flush=True)
    print(f"{color}{message}{nc}")
    
def print_colored(message, color):
    # Use Python's print with ANSI escape codes so output integrates with
    # the current Python stdout (no separate shell call). This ensures the
    # previous `print(..., end='')` can be followed by a colored message on
    # the same line.
    
    # print(f"{color}{message}{nc}")
    print_prefix_colored("", message, color)
    
    
def print_menu():
    print("\n=========================系统菜单=========================")
    print("请选择要执行的操作:")
    print_colored(f'当前目录: {os.getcwd()}', CYAN)
    
    #创建一个字典来存储项目和解释的对应关系
    dict1 = {build_items[i]: build_items_explain[i] for i in range(len(build_items))}
        
    if build_debug:
        #创建的这个字典，以build_items的值为KEY，以build_items_explain的值为VALUE
        print(dict1)
       
    # for key_index in build_items:
        # #打印行号，字典里的KEY，字典里的VALUE
        # item_no = build_items.index(key_index)
        # print(f"{item_no}. {key_index.lower()} - {dict1[key_index]}")
    
    #以下方法是采用enumerate函数来获取索引和值
    # for item_no, value in enumerate(build_items):
        # print(f"{item_no}. {value.lower()} - {dict1[value]}")
        
    #另一种方法是使用字典的items()方法
    for item_no, (key, val) in enumerate(dict1.items()):
        print(f"{item_no}. {key.lower()} - {val}")
        
    print("\n==========================================================")
    choice = input("请输入你的选择: ").strip()
    return choice

def build_item(item):
    build_cmd_full = f"{build_cmd} {build_items[item]}"
    print(f"Building item: {build_cmd_full}")
    
    if build_items[item] == "exit":
        print_colored("退出成功", GREEN)
        sys.exit(0)
    
    ret = os.system(build_cmd_full)
    if ret != 0:
        print_colored(f"Error: Item: \"{build_items[item].upper()}\" build failed.\n", RED)
    else:
        print_colored(f"OK: Item: \"{build_items[item].upper()}\" build success.\n", GREEN)

# 主程序入口
if __name__ == "__main__":
    
    while True:
        try:
            choice = print_menu()
            if choice == '':
                raise Exception
            if not choice.isdigit():
                raise TypeError
            if int(choice) < 0 or int(choice) >= len(build_items):
                raise ValueError
            
        except TypeError:
            print_colored("输入的选择不是数字，请重新输入：", RED)
            continue
        except ValueError:
            print_colored("输入的选择超出范围，请重新输入：", RED)
            continue
        except Exception:
            print_colored("无效的选择，请重新输入：", RED)
            continue
        else:
            # keep the label and the selection on the same line
            # print("You selected: ", end='', flush=True)
            # print_colored(f"[{choice}. {build_items[int(choice)]}]", CYAN)
            print_prefix_colored("你选择的项目: ", f"[{choice}. {build_items[int(choice)]}]", CYAN)
            build_item(int(choice))

```



---

我这边用Python重写了T536的build.sh构建，等效于T536开发目录下的脚本：`sanway_build.sh`。

python文件名称为：`sanway_build.py`，源代码如下：

```py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os

build_debug = False

build_items = (
    "exit",
    "all",
    "config",
    "kernel",
    "menuconfig",
    "saveconfig",
    "bootloader",
    "uboot_menuconfig",
    "uboot_saveconfig",
    "rootfs",
    "buildroot_menuconfig",
    "buildroot_saveconfig",
    "pack"
)

build_items_explain = (
    "退出程序",
    "全部构建",
    "配置build.sh编译环境",
    "编译内核",
    "内核编译配置",
    "保存内核编译配置",
    "编译引导程序bootloader",
    "uboot编译配置",
    "保存uboot编译配置",
    "编译buildroot根文件系统",
    "buildroot文件系统编译配置",
    "保存buildroot文件系统编译配置",
    "将全部编译结果打包pack"
)

build_cmd = "./build.sh"

# ANSI escape sequences (use \033 for portability)
red='\033[1;31m'
RED='\033[1;31m'
blue='\033[0;34m'
BLUE='\033[1;34m'
cyan='\033[0;36m'
CYAN='\033[1;36m'
green='\033[0;32m'
GREEN='\033[1;32m'
yellow='\033[1;33m'
YELLOW='\033[1;33m'
PURPLE='\033[1;35m'
purple='\033[0;35m'
error='\033[1;41m'
warn='\033[1;43m'
nc='\033[0m'
    
def print_prefix_colored(prefix, message, color):
    print(f"{prefix}", end='', flush=True)
    print(f"{color}{message}{nc}")
    
def print_colored(message, color):
    # Use Python's print with ANSI escape codes so output integrates with
    # the current Python stdout (no separate shell call). This ensures the
    # previous `print(..., end='')` can be followed by a colored message on
    # the same line.
    
    # print(f"{color}{message}{nc}")
    print_prefix_colored("", message, color)
    
    
def print_menu():
    print("\n=========================系统菜单=========================")
    print("请选择要执行的操作:")
    print_colored(f'当前目录: {os.getcwd()}', CYAN)
    
    #创建一个字典来存储项目和解释的对应关系
    dict1 = {}    
    # for item in range(len(build_items)):        
        # dict1[build_items[item]] = build_items_explain[item]
        
    # 使用列表的index()方法来获取列表中元素的索引，然后使用这个索引来获取列表中元素的值
    for key_index in build_items:
        item_no = build_items.index(key_index)
        dict1[key_index] = build_items_explain[item_no]
    
    if build_debug:
        #创建的这个字典，以build_items的值为KEY，以build_items_explain的值为VALUE
        print(dict1)
       
    for key_index in build_items:
        #打印行号，字典里的KEY，字典里的VALUE
        item_no = build_items.index(key_index)
        print(f"{item_no}. {key_index.lower()} - {dict1[key_index]}")
        
    print("\n==========================================================")
    choice = input("请输入你的选择: ").strip()
    return choice

def build_item(item):
    build_cmd_full = f"{build_cmd} {build_items[item]}"
    print(f"Building item: {build_cmd_full}")
    
    if build_items[item] == "exit":
        print_colored("退出成功", GREEN)
        sys.exit(0)
    
    ret = os.system(build_cmd_full)
    if ret != 0:
        print_colored(f"Error: Item: \"{build_items[item].upper()}\" build failed.\n", RED)
    else:
        print_colored(f"OK: Item: \"{build_items[item].upper()}\" build success.\n", GREEN)

# 主程序入口
if __name__ == "__main__":
    
    while True:
        '''
        choice = print_menu()        
        if choice == '' or not choice.isdigit() or int(choice) < 0 or int(choice) >= len(build_items):
            print_colored("无效的选择，请重新输入：", RED)            
        else:
            # keep the label and the selection on the same line
            # print("You selected: ", end='', flush=True)
            # print_colored(f"[{choice}. {build_items[int(choice)]}]", CYAN)
            print_prefix_colored("你选择的项目: ", f"[{choice}. {build_items[int(choice)]}]", CYAN)
            build_item(int(choice))
        '''
        try:
            choice = int(print_menu())
            if choice < 0 or choice >= len(build_items):
                raise ValueError
        except:
            print_colored("无效的选择，请重新输入：", RED)
            continue
        else:
            # keep the label and the selection on the same line
            # print("You selected: ", end='', flush=True)
            # print_colored(f"[{choice}. {build_items[int(choice)]}]", CYAN)
            print_prefix_colored("你选择的项目: ", f"[{choice}. {build_items[int(choice)]}]", CYAN)
            build_item(int(choice))

```

