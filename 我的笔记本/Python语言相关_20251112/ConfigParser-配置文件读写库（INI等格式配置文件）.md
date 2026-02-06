# ConfigParser-配置文件读写库

使用配置文件来灵活的配置一些参数是一件很常见的事情，配置文件的解析并不复杂，在python里更是如此，在官方发布的库中就包含有做这件事情的库，那就是configParser。

configParser解析的配置文件的格式比较象ini的配置文件格式，就是文件中由多个section构成，每个section下又有多个配置项。

configparser 是 Python 的一个内置库，用于读取和写入 INI 、conf格式的配置文件。

## configparser基本介绍

### 安装

python3里面自带configparser模块来读取ini、conf文件

- 用来读取配置文件的python包；
- 一般做自动化测试的时候，会使用到这个模块，用来封装一些常量。比如数据库、邮件、用户名密码、项目常量等等；

这个使用根据个人喜好和项目来确定，不一定一定要使用这个模块，也可以使用其它的方法做配置，比如py文件、xml、excel、yaml、json等等。

configparser源码大约1360行左右，通读源码可有效了解该模块的使用。本文只做简单介绍常用的方法。

### 三种解析器介绍

ConfigParser模块在python3中修改为configparser.这个模块定义了一个ConfigParser类，该类的作用是使用配置文件生效，配置文件的格式和windows的INI文件的格式相同

configparser 是 Python 的一个库，用于读取和写入 INI 文件格式的配置文件。在 Python 3.2 之前，这个库叫做 ConfigParser，而在 Python 3.2 及以后的版本中，它被重命名为 configparser。这个库提供了一种简单的方法来处理配置文件。

configparser 库中主要有三种解析器，即 RawConfigParser，ConfigParser 和 SafeConfigParser。它们之间的主要区别在于如何处理配置文件中的语法错误。

RawConfigParser，ConfigParser 和 SafeConfigParser 继承关系如下
```sh
class RawConfigParser(MutableMapping):
class ConfigParser(RawConfigParser):
class SafeConfigParser(ConfigParser):
```

- RawConfigParser


RawConfigParser 是 ConfigParser 的一个子类，它以原始方式解析配置文件，这意味着它不会对配置文件进行任何预处理或转义。

在使用 RawConfigParser 时，必须在读取配置文件时小心处理任何可能导致解析错误的特殊字符或字符串。

如果你正在编写或处理一个非常复杂的配置文件，并且希望能够更精确地控制解析过程，那么 RawConfigParser 可能是一个好选择。

- ConfigParser


这是 configparser 库的默认解析器。ConfigParser 对配置文件进行了一些预处理和转义，以使其更易于解析。

例如，它处理新行定界符，自动转换选项名和键名的大小写，以及处理等宽字体选项。一般情况下，如果你不需要特别处理配置文件中的特殊字符或字符串，那么使用 ConfigParser 就足够了。

- SafeConfigParser


SafeConfigParser 是 ConfigParser 的一个子类，它在解析配置文件时提供了额外的安全保障。

它只接受预设的键名、选项名和段落名，并且只允许特定的值类型。这使得 SafeConfigParser 在处理不受信任的输入时更加安全。

如果你正在处理来自不可信来源的配置文件，或者想要确保配置文件的格式和内容符合特定的要求，那么使用 SafeConfigParser 是个好主意。

总的来说，你可以根据你的需求和场景选择使用这三种解析器中的任何一种。在大多数情况下，使用默认的 ConfigParser 就足够了。

### 配置文件格式

- 新建一个名为`config.conf`文件；

- 写入如下数据，格式如下：

```ini
[mysqldb]
sql_host = 127.0.0.1
sql_port = 3699
sql_user = root
sql_pass = 123456

[mailinfo]
name = NoamaNelson
passwd = 123456
address = 123456@qq.com
```

配置文件有不同的片段组成和Linux中repo文件中的格式类似：

格式：

```ini
[section] 
name=value
或者
name: value
"#" 和";" 表示注释

[DEFAULT] #设置默认的变量值，初始化
```

python代码：

```python
[My Section]
foodir: %(dir)s/whatever
dir=frob
long: this value continues
   in the next line

```

%（目录）s会被瞎调代替。默认值会以字典的形式传递给配置分析器的构造器。部分一般存放的哦内置目录下，如果切换到其他的目录需啊哟指定存放位置。

- section不能重复，里面数据通过section去查找
- 每个seletion下可以有多个key和vlaue的键值对
- 注释用英文分号(;)或者 井号#

以下是一个示例，展示了如何实例化一个 RawConfigParser 对象：

```python
import configparser

config = configparser.RawConfigParser(
    default_section='DEFAULT',
    comment_prefixes=['[', ']'],
    inline_comment_prefixes=None,
    strict=True,
    empty_lines_in_values=True,
    default_source='<string>'
)
```

请注意，RawConfigParser 是 ConfigParser 的子类，因此也可以使用 ConfigParser 的参数。有关更多详细信息，请参阅 Python 官方文档中的 configparser 模块文档。

以下是一些 configparser 的主要特性：

> [!tip]
>
> - 可以通过 configparser.ConfigParser() 创建一个配置解析器对象。
> - 配置文件可以包含节（sections）、键（keys）和值（values）。
> - 配置文件中的每一行都应当以 [section] 开始，然后紧接着的是键值对，形如 key = value。
> - 可以在 configparser.ConfigParser() 中设置各种参数，比如 delimiters（分隔符），comment_prefixes（注释前缀）等。

以下是一些RawConfigParser类中的常用方法：

> [!important]
>
> - read(filenames): 从给定的文件或目录中读取配置数据。可以同时读取多个文件。
> - readfp(fp): 从打开的文件对象中读取配置数据。
> - get(section, option): 返回指定节（section）中指定选项（option）的值。
> - getboolean(section, option): 返回指定节中指定选项的布尔值。如果值是字符串，则将“1”解释为True，“0”解释为False。
> - getfloat(section, option): 返回指定节中指定选项的浮点值。
> - getint(section, option): 返回指定节中指定选项的整数值。
> - set(section, option, value): 设置指定节的指定选项的值为value。
> - add_section(section): 添加一个新的节到配置文件中。
> - remove_section(section): 从配置文件中删除指定的节。
> - remove_option(section, option): 从指定的节中删除指定的选项。
> - write(fp): 将配置数据写入打开的文件对象中。

下面是一个简单的示例：
```python
import configparser

# 创建配置解析器对象
config = configparser.ConfigParser()

# 读取配置文件
config.read('example.ini')

# 获取指定节下的指定键的值
print(config.get('section1', 'key1'))

# 设置指定节下的指定键的值
config.set('section1', 'key2', 'value2')

# 移除指定节下的指定键
config.remove('section1', 'key1')

# 获取指定节的所有键值对
for key, value in config.items('section1'):
    print(f'{key}: {value}')

# 将内存中的配置写入到文件
with open('example.ini', 'w') as configfile:
    config.write(configfile)

```

以上就是 configparser 的基本使用方法。

### 对配置文件

#### 读取配置文件

|          方法          | 描述                                                         |
| :--------------------: | ------------------------------------------------------------ |
|    read(filenames)     | filesnames是一个列表，需要从文件加载初始值的应用程序应该在调用read()之前使用readfp()加载所需的文件或文件。 |
| readfp(fp[, filename]) | 在fp中，从文件或文件类对象中读取和解析配置数据(只使用readline()方法)。如果文件名被省略，并且fp有一个name属性，它被用于文件名;默认值为< ? >。 |

#### 写入配置文件

|       方法        | 描述                                                         |
| :---------------: | ------------------------------------------------------------ |
| write(fileobject) | 将配置的表示写入指定的文件对象。这个表示可以由未来的read()调用解析。 |

### 对内存中数据流

#### 增加配置文件中的值

|         方法         | 描述                  |
| :------------------: | --------------------- |
| add_section(section) | 向实例添加一个section |

#### 删除配置文件中的值

|              方法              | 描述                                                         |
| :----------------------------: | ------------------------------------------------------------ |
| remove_option(section, option) | 从指定的部分中删除指定的选项。如果该部分不存在，raise NoSectionError。<br>如果存在的选项被删除，返回True;否则返回False。 |
|    remove_section(section)     | 从配置中删除指定的section。如果这个部分确实存在，返回True。否则返回False |

#### 修改配置文件中的值

|            方法             | 描述                                                         |
| :-------------------------: | ------------------------------------------------------------ |
| set(section, option, value) | 如果给定的部分存在，将给定的选项设置为指定的值;否则raise NoSectionError。<br>值必须是字符串(str或unicode);如果没有，则会出现类型错误 |
|     optionxform(option)     | 也可以在一个实例上重新设置它，对于一个需要字符串参数的函数。例如，将其设置为str，将使选项名称区分大小写 |

#### 获取配置文件中的值

| 方法                        | 描述                                                         |
| :-------------------------- | ------------------------------------------------------------ |
| defaults()                  | 返回包含实例范围默认值的字典。                               |
| sections()                  | 返回可用的section的列表;默认section不包括在列表中            |
| has_section(section)        | 指示指定的section是否出现在配置中。默认的section未被确认     |
| options(section)            | 返回指定section中可用的选项列表。                            |
| has_option(section, option) | 如果给定的section存在，并且包含给定的选项，则返回True;否则返回False |
| get(section, option)        | 为指定的section获取一个选项值。                              |
| getint(section, option)     | 它将指定section中的选项强制转换为整数                        |
| getfloat(section, option)   | 它将指定section中的选项强制转换为浮点型                      |
| getboolean(section, option) | 强制转换为布尔型，<br>”1”, “yes”, “true”, and “on”, 转换为True，<br>”0”, “no”, “false”, and “off”, 转换为Falseo。<br>其他返回ValueError. |
| items(section)              | 返回给定section中每个选项的(name,value)对的列表。            |

```python
import ConfigParser, os

config = ConfigParser.ConfigParser()
config.readfp(open('defaults.cfg'))
config.read(['site.cfg', os.path.expanduser('~/.myapp.cfg')])
```

```python
cfgparser = ConfigParser()
...
cfgparser.optionxform = str
```

### 简单例子

configparser 是 Python 的一个库，它用于读取和写入 INI 文件格式的配置文件。INI 文件是一个简单的文本文件，用于存储应用程序的配置信息。它由节（sections）、键（keys）和值（values）组成，通常采用以下格式：

```ini
 [section1]
 key1 = value1
 key2 = value2
 
 [section2]
 key3 = value3
```

configparser` 库提供了一个类似于字典的接口，可以使用它来读取和修改 INI 文件中的配置数据。下面是一些常用的方法和说明：

#### 创建解析器对象

要使用 configparser，首先需要创建一个解析器对象。可以使用 configparser.ConfigParser() 或 configparser.SafeConfigParser() 来创建解析器对象。后者是前者的子类，提供了额外的安全保障，但功能略有限制。
```python
import configparser

# 创建 ConfigParser 对象
config = configparser.ConfigParser()
```

#### 读取配置文件

使用 read() 方法读取一个或多个配置文件。

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')
```

#### 获取配置项

使用 get() 方法获取指定节（section）下的指定键（key）的值（value）。

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')

# 获取 section1 中 key1 的值
value = config.get('section1', 'key1')
print(value)  # 输出: value1
```

#### 设置配置项

使用 set() 方法设置指定节（section）下的指定键（key）的值（value）。

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')

# 设置 section1 中 key1 的值为新值
config.set('section1', 'key1', 'new_value1')
```

#### 删除配置项

使用 remove() 方法删除指定节（section）下的指定键（key）。

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')

# 删除 section1 中的 key1
config.remove('section1', 'key1')
```

#### 写回到文件

使用 write() 方法将修改后的配置数据写回到文件中。注意，如果在读取配置文件后没有对其进行任何修改，调用 write() 方法不会产生任何效果。

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')

# 设置 section1 中 key1 的值为新值
config.set('section1', 'key1', 'new_value1')

# 将修改后的配置数据写回到文件中
with open('config.ini', 'w') as configfile:
	config.write(configfile)
```

### 异常

| 异常                                   | 描述                                                         |
| -------------------------------------- | ------------------------------------------------------------ |
| ConfigParser.Error                     | 所有异常的基类                                               |
| ConfigParser.NoSectionError            | 指定的section没有找到                                        |
| ConfigParser.DuplicateSectionError     | 调用add_section() 时，section名称已经被使用                  |
| ConfigParser.NoOptionError             | 指定的参数没有找到                                           |
| ConfigParser.InterpolationError        | 当执行字符串插值时出现问题时，出现异常的基类                 |
| ConfigParser.InterpolationDepthError   | 当字符串插值无法完成时，因为迭代次数超过了最大的范围，所以无法完成。InterpolationError的子类 |
| InterpolationMissingOptionError        | 当引用的选项不存在时，会出现异常。InterpolationError的子类   |
| ConfigParser.InterpolationSyntaxError  | 当产生替换的源文本不符合所需的语法时，就会出现异常。InterpolationError的子类。 |
| ConfigParser.MissingSectionHeaderError | 当试图解析一个没有分段标题的文件时，会出现异常。             |
| ConfigParser.ParsingError              | 当试图解析文件时发生错误时，会出现异常                       |
| ConfigParser.MAX_INTERPOLATION_DEPTH   | 当raw参数为false时，get()的递归插值的最大深度。这只适用于ConfigParser类 |

## 实例

### 大例子

下例代码汇总：

```python
import configparser
import os


class Conf:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.root_path = os.path.dirname(os.path.abspath(__file__))
        self.f = os.path.join(self.root_path + "/config.conf")
        self.conf.read(self.f)

    def read_sections(self):
        print(f"1、获取所有的sections:{self.conf.sections()}")

    def read_options(self, s1, s2):
        print(f"2、获取mysqldb所有的options:{self.conf.options(s1)}")
        print(f"3、获取mailinfo所有的options:{self.conf.options(s2)}")

    def read_conf(self, m, n):
        name = self.conf.get(m, n)  # 获取指定section的option值
        print(f"4、获取指定section:{m}下的option：{n}的值为{name}")

    def get_items(self, m, n):
        print(f"5、获取sectoion:{m}下的配置信息为：{self.conf.items(m)}")
        print(f"6、获取sectoion:{n}下的配置信息为：{self.conf.items(n)}")

    def set_option(self, m, n, s):
        self.conf.set(m, n, s)
        self.conf.write(open(self.f, "w"))
        print(f"7、设置setion:{m}下的option:{n}的值为：{s}")

    def has_s_o(self, s, o):
        print(f"8、检查section：{s}是否存在：{self.conf.has_section(s)}")
        print(f"9、检查section：{s}下的option：{o}是否存在：{self.conf.has_option(s, o)}")

    def add_s_o(self, s, o, v):
        if not self.conf.has_section(s):
            self.conf.add_section(s)
            print(f"10、添加新的section为{s}")
        else:
            print(f"10、添加新的section为{s}已经存在，无需添加！")
        if not self.conf.has_option(s, o):
            self.conf.set(s, o, v)
            print(f"11、要添加的option为{o}, 值为{v}")
        else:
            print(f"11、要添加的option为{o}, 值为{v}，已经存在，无需添加！")
        self.conf.write(open(self.f, "w"))

    def remove_s_o(self, s, o):
        if self.conf.has_section(s):
            self.conf.remove_section(s)
            print(f"12、删除section:{s}==OK!")
        else:
            print(f"12、要删除的section:{s}不存在，不用删除！")
        if self.conf.has_option(s, o):
            self.conf.remove_option(s, o)
            print(f"13、删除section：{s}下的option：{o}==OK!")
        else:
            print(f"13、要删除的section：{s}下的option：{o}不存在，不用删除！")


if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")
    aa.set_option("mysqldb", "sql_name", "游客")
    aa.has_s_o("mysqldb", "sql_name")
    aa.add_s_o("login", "name", "root")
    aa.remove_s_o("login", "name")
```

#### 获取所有的sections

```python
    def read_sections(self):
        print(f"1、获取所有的sections:{self.conf.sections()}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
"""
```

#### 获取所有的sections对应的options

```python
    def read_options(self, s1, s2):
        print(f"2、获取mysqldb所有的options:{self.conf.options(s1)}")
        print(f"3、获取mailinfo所有的options:{self.conf.options(s2)}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
"""    
```

#### read方法和get方法，获取指定section下的option值

- `read`是读取配置文件，如` self.conf.read(self.f)`是读取指定`config.conf`文件；
- `get`是获取具体的值；

```python
    def read_conf(self, m, n):
        name = self.conf.get(m, n)  # 获取指定section的option值
        print(f"4、获取指定section:{m}下的option：{n}的值为{name}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
4、获取指定section:mysqldb下的option：sql_host的值为127.0.0.1
"""
```

#### items方法，获取指点section所用配置信息

```python
    def get_items(self, m, n):
        print(f"5、获取sectoion:{m}下的配置信息为：{self.conf.items(m)}")
        print(f"6、获取sectoion:{n}下的配置信息为：{self.conf.items(n)}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
4、获取指定section:mysqldb下的option：sql_host的值为127.0.0.1
5、获取sectoion:mysqldb下的配置信息为：[('sql_host', '127.0.0.1'), ('sql_port', '3699'), ('sql_user', 'root'), ('sql_pass', '123456')]
6、获取sectoion:mailinfo下的配置信息为：[('name', 'NoamaNelson'), ('passwd', '123456'), ('address', '123456@qq.com')]
"""
```

#### set和write方法，修改某个option的值

- 如果option不存在则创建它；

```python
    def set_option(self, m, n, s):
        self.conf.set(m, n, s)
        self.conf.write(open(self.f, "w"))
        print(f"7、设置setion:{m}下的option:{n}的值为：{s}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")
    aa.set_option("mysqldb", "sql_name", "游客")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
4、获取指定section:mysqldb下的option：sql_host的值为127.0.0.1
5、获取sectoion:mysqldb下的配置信息为：[('sql_host', '127.0.0.1'), ('sql_port', '3699'), ('sql_user', 'root'), ('sql_pass', '123456')]
6、获取sectoion:mailinfo下的配置信息为：[('name', 'NoamaNelson'), ('passwd', '123456'), ('address', '123456@qq.com')]
7、设置setion:mysqldb下的option:sql_name的值为：游客
"""
```

因为之前的配置文件中没有sql_name，所以会新创建它，如果创建的内容存在，直接修改对应的值；

#### has_section和has_option检查section和option

- `has_section`方法检查对应的`section`是否存在；
- `has_option`方法检查对应的`option`是否存在；

```python
    def has_s_o(self, s, o):
        print(f"8、检查section：{s}是否存在：{self.conf.has_section(s)}")
        print(f"9、检查section：{s}下的option：{o}是否存在：{self.conf.has_option(s, o)}")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")
    aa.set_option("mysqldb", "sql_name", "游客")
    aa.has_s_o("mysqldb", "sql_name")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass', 'sql_name']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
4、获取指定section:mysqldb下的option：sql_host的值为127.0.0.1
5、获取sectoion:mysqldb下的配置信息为：[('sql_host', '127.0.0.1'), ('sql_port', '3699'), ('sql_user', 'root'), ('sql_pass', '123456'), ('sql_name', '游客')]
6、获取sectoion:mailinfo下的配置信息为：[('name', 'NoamaNelson'), ('passwd', '123456'), ('address', '123456@qq.com')]
7、设置setion:mysqldb下的option:sql_name的值为：游客
8、检查section：mysqldb是否存在：True
9、检查section：mysqldb下的option：sql_name是否存在：True
"""
```

#### add_section方法，添加section和option

- `add_section`：添加`section`；

```python
    def add_s_o(self, s, o, v):
        if not self.conf.has_section(s):
            self.conf.add_section(s)
            print(f"10、添加新的section为{s}")
        else:
            print(f"10、添加新的section为{s}已经存在，无需添加！")
        if not self.conf.has_option(s, o):
            self.conf.set(s, o, v)
            print(f"11、要添加的option为{o}, 值为{v}")
        else:
            print(f"11、要添加的option为{o}, 值为{v}，已经存在，无需添加！")
        self.conf.write(open(self.f, "w"))

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")
    aa.set_option("mysqldb", "sql_name", "游客")
    aa.has_s_o("mysqldb", "sql_name")
    aa.add_s_o("login", "name", "root")
```

再次运行代码，会提示已经存在。

#### remove_section和remove_option方法，删除section和option

```python
    def remove_s_o(self, s, o):
        if self.conf.has_section(s):
            self.conf.remove_section(s)
            print(f"12、删除section:{s}==OK!")
        else:
            print(f"12、要删除的section:{s}不存在，不用删除！")
        if self.conf.has_option(s, o):
            self.conf.remove_option(s, o)
            print(f"13、删除section：{s}下的option：{o}==OK!")
        else:
            print(f"13、要删除的section：{s}下的option：{o}不存在，不用删除！")

if __name__ == "__main__":
    aa = Conf()
    aa.read_sections()
    aa.read_options("mysqldb", "mailinfo")
    aa.read_conf("mysqldb", "sql_host")
    aa.get_items("mysqldb", "mailinfo")
    aa.set_option("mysqldb", "sql_name", "游客")
    aa.has_s_o("mysqldb", "sql_name")
    aa.add_s_o("login", "name", "root")
    aa.remove_s_o("login", "name")

"""
D:\Python37\python.exe F:/python_study/conf.py
1、获取所有的sections:['mysqldb', 'mailinfo', 'login']
2、获取mysqldb所有的options:['sql_host', 'sql_port', 'sql_user', 'sql_pass', 'sql_name']
3、获取mailinfo所有的options:['name', 'passwd', 'address']
4、获取指定section:mysqldb下的option：sql_host的值为127.0.0.1
5、获取sectoion:mysqldb下的配置信息为：[('sql_host', '127.0.0.1'), ('sql_port', '3699'), ('sql_user', 'root'), ('sql_pass', '123456'), ('sql_name', '游客')]
6、获取sectoion:mailinfo下的配置信息为：[('name', 'NoamaNelson'), ('passwd', '123456'), ('address', '123456@qq.com')]
7、设置setion:mysqldb下的option:sql_name的值为：游客
8、检查section：mysqldb是否存在：True
9、检查section：mysqldb下的option：sql_name是否存在：True
10、添加新的section为login已经存在，无需添加！
11、要添加的option为name, 值为root，已经存在，无需添加！
12、删除section:login==OK!
13、要删除的section：login下的option：name不存在，不用删除！
"""

```

### 写配置文件

```python
import ConfigParser

config = ConfigParser.RawConfigParser()

# When adding sections or items, add them in the reverse order of
# how you want them to be displayed in the actual file.
# In addition, please note that using RawConfigParser's and the raw
# mode of ConfigParser's respective set functions, you can assign
# non-string values to keys internally, but will receive an error
# when attempting to write to a file or when you get it in non-raw
# mode. SafeConfigParser does not allow such assignments to take place.
config.add_section('Section1')
config.set('Section1', 'an_int', '15')
config.set('Section1', 'a_bool', 'true')
config.set('Section1', 'a_float', '3.1415')
config.set('Section1', 'baz', 'fun')
config.set('Section1', 'bar', 'Python')
config.set('Section1', 'foo', '%(bar)s is %(baz)s!')

# Writing our configuration file to 'example.cfg'
with open('example.cfg', 'wb') as configfile:
    config.write(configfile)
```

### 读配置文件

```python
import ConfigParser

config = ConfigParser.RawConfigParser()
config.read('example.cfg')

# getfloat() raises an exception if the value is not a float
# getint() and getboolean() also do this for their respective types
a_float = config.getfloat('Section1', 'a_float')
an_int = config.getint('Section1', 'an_int')
print a_float + an_int

# Notice that the next output does not interpolate '%(bar)s' or '%(baz)s'.
# This is because we are using a RawConfigParser().
if config.getboolean('Section1', 'a_bool'):
    print config.get('Section1', 'foo')
```

### 获取插入值

```python
import ConfigParser

config = ConfigParser.ConfigParser()
config.read('example.cfg')

# Set the third, optional argument of get to 1 if you wish to use raw mode.
print config.get('Section1', 'foo', 0)  # -> "Python is fun!"
print config.get('Section1', 'foo', 1)  # -> "%(bar)s is %(baz)s!"

# The optional fourth argument is a dict with members that will take
# precedence in interpolation.
print config.get('Section1', 'foo', 0, {'bar': 'Documentation',
                                        'baz': 'evil'})
```

### 默认值

所有三种类型的配置分析器都可以使用默认值。如果在其他地方没有定义一个选项，那么它们就被用于插值

```python
import ConfigParser

# New instance with 'bar' and 'baz' defaulting to 'Life' and 'hard' each
config = ConfigParser.SafeConfigParser({'bar': 'Life', 'baz': 'hard'})
config.read('example.cfg')

print config.get('Section1', 'foo')  # -> "Python is fun!"
config.remove_option('Section1', 'bar')
config.remove_option('Section1', 'baz')
print config.get('Section1', 'foo')  # -> "Life is hard!"
```

### 在各section之间移动选项

```python
def opt_move(config, section1, section2, option):
    try:
        config.set(section2, option, config.get(section1, option, 1))
    except ConfigParser.NoSectionError:
        # Create non-existent section
        config.add_section(section2)
        opt_move(config, section1, section2, option)
    else:
        config.remove_option(section1, option)
```

### 配置文件中有空值

一些配置文件包含了没有值的设置，但是它与配置分析器支持的语法相一致。对构造函数的不允许值参数可以被用来表示应该接受这样的值

```python
>>> import ConfigParser
>>> import io

>>> sample_config = """
... [mysqld]
... user = mysql
... pid-file = /var/run/mysqld/mysqld.pid
... skip-external-locking
... old_passwords = 1
... skip-bdb
... skip-innodb
... """
>>> config = ConfigParser.RawConfigParser(allow_no_value=True)
>>> config.readfp(io.BytesIO(sample_config))

>>> # Settings with values are treated as before:
>>> config.get("mysqld", "user")
'mysql'

>>> # Settings without values provide None:
>>> config.get("mysqld", "skip-bdb")

>>> # Settings which aren't specified still raise an error:
>>> config.get("mysqld", "does-not-exist")
Traceback (most recent call last):
  ...
ConfigParser.NoOptionError: No option 'does-not-exist' in section: 'mysqld'
```















————————————————
版权声明：本文为CSDN博主「士别三日，当挖目相待」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/qq_40597070/article/details/135297610