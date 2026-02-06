# python基础对象方法、类方法、静态方法的区别

**今天主要分享python对象方法、类方法、静态方法的区别#python#**

# 一、三者的定义与核心区别

| 方法类型                 | 定义方式             | 第一个参数       | 能访问什么？                                       | 如何调用？             |
| ------------------------ | -------------------- | ---------------- | -------------------------------------------------- | ---------------------- |
| **实例方法**（对象方法） | 普通 def             | self（实例自身） | 实例属性 + 类属性                                  | 必须通过**实例**调用   |
| **类方法**               | @classmethod 装饰器  | cls（类本身）    | **只能访问类属性**，不能直接访问实例属性           | 可通过**类或实例**调用 |
| **静态方法**             | @staticmethod 装饰器 | **无强制参数**   | **不能直接访问**类或实例的任何属性（除非显式传入） | 可通过**类或实例**调用 |

关键记忆点：

self → 实例

cls → 类

静态方法 → “挂靠”在类里的普通函数

# 二、代码示例：直观对比三者

```python
class Student:
    school = "清华大学"  # 类属性

    def __init__(self, name, age):
        self.name = name  # 实例属性
        self.age = age

    # 1. 实例方法（对象方法）
    def introduce(self):
        print(f"我是 {self.name}，来自 {self.school}")

    # 2. 类方法
    @classmethod
    def get_school(cls):
        print(f"学校是：{cls.school}")
        # ❌ 不能访问 self.name（没有实例！）
        # print(self.name)  # 报错！

    # 3. 静态方法
    @staticmethod
    def is_adult(age):
        return age >= 18
        # ❌ 不能直接访问 cls 或 self
        # print(school)  # 报错！
```

**调用方式演示：**

```python
# 创建实例
stu = Student("张三", 20)

# 实例方法 → 必须用实例调用
stu.introduce()          # ✅ 正确
# Student.introduce()    # ❌ 报错！缺少 self

# 类方法 → 类或实例均可调用
Student.get_school()     # ✅ 正确
stu.get_school()         # ✅ 也正确（但不推荐）

# 静态方法 → 类或实例均可调用
print(Student.is_adult(17))  # ✅ True/False
print(stu.is_adult(25))      # ✅ 也可以（但逻辑上不合理）
```

# 三、他们三者能互相调用吗？

答案：可以，但有条件！

| 调用方 \ 被调用方 | 实例方法                                                | 类方法                                     | 静态方法   |
| ----------------- | ------------------------------------------------------- | ------------------------------------------ | ---------- |
| **实例方法**      | ✅ 直接调用                                              | ✅ self.__class__.类方法() 或 类名.类方法() | ✅ 直接调用 |
| **类方法**        | ❌ **不能直接调用**（没有实例） ✅ 但可**创建实例后再调** | ✅ 直接调用                                 | ✅ 直接调用 |
| **静态方法**      | ❌ 不能直接调用 ✅ 但可**传入实例后调用**                 | ❌ 不能直接调用 ✅ 但可**传入类后调用**      | ✅ 直接调用 |

```python
class Example:
    count = 0

    def __init__(self, value):
        self.value = value
        Example.count += 1

    def instance_method(self):
        print("调用实例方法")
        self.class_method()        # ✅ 实例方法调类方法
        self.static_method()       # ✅ 实例方法调静态方法

    @classmethod
    def class_method(cls):
        print("调用类方法")
        obj = cls(999)             # ✅ 类方法中创建实例
        obj.instance_method()      # ✅ 再调实例方法
        cls.static_method()        # ✅ 类方法调静态方法

    @staticmethod
    def static_method():
        print("调用静态方法")
        # ❌ 无法直接调 instance_method 或 class_method
        # 但可以这样：
        obj = Example(888)
        obj.instance_method()      # ✅ 通过创建实例间接调
        Example.class_method()     # ✅ 通过类名间接调
```

# 四、使用场景与最佳实践（附真实案例）

**1.实例方法→最常用！**

- **场景**：需要操作**特定对象的状态**（实例属性）
- **例子**：用户登录验证（检查该用户的密码）购物车添加商品（修改该用户的购物车）银行账户转账（修改两个账户的余额）

```python
class BankAccount:
    def __init__(self, balance):
        self.balance = balance

    def withdraw(self, amount):  # 实例方法：修改自己的余额
        if amount <= self.balance:
            self.balance -= amount
            return True
        return False
```

**2.类方法→替代构造函数 or 操作类状态**

- **场景**：**提供多种对象创建方式**（工厂模式）**管理类级别的数据**（如计数器、全局配置）
- **经典例子**：dict.fromkeys() 就是一个类方法！

```python
class Person:
    species = "Homo sapiens"
    population = 0

    def __init__(self, name):
        self.name = name
        Person.population += 1

    @classmethod
    def get_population(cls):
        return cls.population

    # 替代构造函数：从字符串创建对象
    @classmethod
    def from_string(cls, person_str):
        name = person_str.split("-")[0]
        return cls(name)  # 返回新实例

# 使用
p1 = Person("Alice")
p2 = Person.from_string("Bob-30")  # 无需知道内部格式
print(Person.get_population())  # 2
```

**3.静态方法→工具函数，与类逻辑相关但不依赖状态**

- **场景**：数据验证如检查邮箱格式、数学计算（如计算距离）日期转换（如时间戳转字符串）
- **原则**：如果这个函数**放在类外面也不影响逻辑**，就可以用静态方法“组织”到类里。

```python
import math

class MathUtils:
    @staticmethod
    def calculate_distance(x1, y1, x2, y2):
        return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    @staticmethod
    def is_valid_email(email):
        return "@" in email and "." in email

# 使用（无需创建实例！）
dist = MathUtils.calculate_distance(0, 0, 3, 4)  # 5.0
print(MathUtils.is_valid_email("test@example.com"))  # True
```

# 五、总结：一张表搞定选择

| 问题                                        | 选哪种方法？                              |
| ------------------------------------------- | ----------------------------------------- |
| 需要访问 self.xxx（实例属性）？             | ✅ **实例方法**                            |
| 需要访问 cls.xxx（类属性）或创建实例？      | ✅ **类方法**                              |
| 函数与类相关，但**不需要任何类/实例数据**？ | ✅ **静态方法**                            |
| 想提供多种创建对象的方式？                  | ✅ **类方法**（如 from_json, from_string） |
| 做通用工具函数（如验证、计算）？            | ✅ **静态方法**                            |

# 最后提醒：

- **不要为了“看起来高级”而滥用类方法/静态方法**。大多数业务逻辑用实例方法即可。
- **静态方法不是“性能更好”**，它只是语义上表示“与状态无关”。
- 在团队协作中，清晰的方法类型有助于他人快速理解代码意图。

![img](https://p3-sign.toutiaoimg.com/tos-cn-i-axegupay5k/5cad8b4baea24f67bb5945f94bf0e341~tplv-tt-origin-web:gif.jpeg?_iz=58558&from=article.pc_detail&lk3s=953192f4&x-expires=1764402371&x-signature=0X4PB4FfTdgoGmw6TpFN9fq4RO4%3D)