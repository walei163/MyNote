## [C语言实现“面向对象”](https://zhuanlan.zhihu.com/p/1998837850469320582)

面向对象的三大特性包括 **封装**、**继承**、**多态**。C 语言作为一种 **面向过程** 的编程语言，其本身并 **不支持面向对象** 的相关特性，但我们可以利用结构体 `struct` 实现类似于其他面向对象语言中的面向对象特性。

## 封装

封装是面向对象编程中将数据和操作数据的方法封装在同一个对象中，通过访问控制（如 private、public）隐藏内部实现细节，只对外提供必要的接口供外部使用，从而保护数据完整性和提高代码安全性。

在面向对象语言中（如 C++、Java），一个类内的成员一般会有 `public`，`protected`，`private` 几种访问权限，但在 C 语言当中由于没有相关关键字，很难实现类似的访问权限控制，所以在 C 语言中利用结构体 `struct` 实现的类似封装的功能，里面所有的成员相当于都是 `public` 的。

- 成员变量的实现：直接在结构体 `struct` 中添加相关变量的声明
- 成员函数的实现：在结构体中添加相关函数指针，利用结构体定义变量之后将该变量的函数指针指向已经实现好的相关函数。

```c
#include <stdio.h>

typedef struct Animal
{
    char *name;
    int age;
    int weight;
    int height;

    void (*Speak)();
    void (*PrintInfo)(struct Animal *);
} Animal;

void AnimalIntroduce()
{
    printf("Hello, I am an animal.\n");
}

void AnimalPrintInfo(Animal *animal)
{
    printf("Name: %s\n", animal->name);
    printf("Age: %d\n", animal->age);
    printf("Weight: %d\n", animal->weight);
    printf("Height: %d\n", animal->height);
}

int main()
{
    Animal animal;
    animal.name = "Buddy";
    animal.age = 5;
    animal.weight = 30;
    animal.height = 60;
    animal.Speak = AnimalIntroduce;
    animal.PrintInfo = AnimalPrintInfo;

    animal.Speak();
    animal.PrintInfo(&animal);

    return 0;
}
```

**输出结果：**

```sh
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
```

## 继承

继承是面向对象编程中允许一个类（子类）从另一个类（父类）获取属性和方法的机制，使子类能够复用父类的代码并在此基础上扩展或修改功能，从而提高代码的重用性和可维护性。

前面已经定义了一个 `Animal` 的结构体，假设我们现在要定义一个 `Dog`，这个 `Dog` 要 “继承” `Animal` 并且拥有一个 `bool` 类型 `is_pet` 的变量代表狗是否是一个宠物，和一个 `communicate` 函数，我们可以这样子实现：

- 将 `Animal` 作为 `Dog` 结构体的第一个变量 `base`（**必须是第一个变量，并且不能是指针变量**），保证通过强制转换的时候不会出错。
- 将剩余 `Dog` 结构体独有的变量和函数指针声明在 `base` 下面。

```c
typedef struct Dog
{
    Animal base;
    bool is_pet;

    void (*Communicate)();
} Dog;
```

如果 `Dog` 类型的变量 `dog` 要访问 `Animal` 中的变量时，有以下两种访问方式：

```c
#include <stdbool.h>
#include <stdio.h>

typedef struct Animal
{
    char *name;
    int age;
    int weight;
    int height;

    void (*Speak)();
    void (*PrintInfo)(struct Animal *);
} Animal;

typedef struct Dog
{
    Animal base;
    bool is_pet;

    void (*Bark)();
} Dog;

void AnimalIntroduce()
{
    printf("Hello, I am an animal.\n");
}

void AnimalPrintInfo(Animal *animal)
{
    printf("Name: %s\n", animal->name);
    printf("Age: %d\n", animal->age);
    printf("Weight: %d\n", animal->weight);
    printf("Height: %d\n", animal->height);
}

void DogBark()
{
    printf("Woof! Woof!\n");
}

int main()
{
    Animal animal;
    animal.name = "Buddy";
    animal.age = 5;
    animal.weight = 30;
    animal.height = 60;
    animal.Speak = AnimalIntroduce;
    animal.PrintInfo = AnimalPrintInfo;

    animal.Speak();
    animal.PrintInfo(&animal);

    Dog dog;
    dog.base = animal;
    dog.is_pet = true;
    dog.Bark = DogBark;

    dog.Bark();

    // 1. 通过 dog.base 进行访问
    dog.base.Speak();
    dog.base.PrintInfo(&dog.base);

    // 2. 通过指针将 dog 强制转换成 Animal 类型的指针进行访问
    Animal *animal_ptr = (Animal *)&dog;
    animal_ptr->Speak();
    animal_ptr->PrintInfo(animal_ptr);

    return 0;
}
```

**输出结果：**

```sh
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Woof! Woof!
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
```

## 多态

多态是面向对象编程中允许不同对象对同一方法调用做出不同响应的特性，它通过父类引用指向子类对象，使程序在运行时根据对象的实际类型自动调用对应的方法实现，从而提高代码的灵活性和可扩展性。

利用 C 语言实现类似多态的特性只需要将 “父类” (`Animal`) 的函数指针指向一个为 “子类” (`Dog`) 实现的函数即可。

```c
#include <stdbool.h>
#include <stdio.h>

typedef struct Animal
{
    char *name;
    int age;
    int weight;
    int height;

    void (*Speak)();
    void (*PrintInfo)(struct Animal *);
} Animal;

typedef struct Dog
{
    Animal base;
    bool is_pet;

    void (*Bark)();
} Dog;

void AnimalIntroduce()
{
    printf("Hello, I am an animal.\n");
}

void AnimalPrintInfo(Animal *animal)
{
    printf("Name: %s\n", animal->name);
    printf("Age: %d\n", animal->age);
    printf("Weight: %d\n", animal->weight);
    printf("Height: %d\n", animal->height);
}

void DogBark()
{
    printf("Woof! Woof!\n");
}

void DogIntroduce()
{
    printf("Hello, I am a dog.\n");
}

void DogPrintInfo(Animal *animal)
{
    Dog *dog = (Dog *) animal;
    printf("Name: %s\n", animal->name);
    printf("Age: %d\n", animal->age);
    printf("Weight: %d\n", animal->weight);
    printf("Height: %d\n", animal->height);
    printf("Is Pet: %s\n", dog->is_pet ? "Yes" : "No");
}

int main()
{
    Animal animal;
    animal.name = "Buddy";
    animal.age = 5;
    animal.weight = 30;
    animal.height = 60;
    animal.Speak = AnimalIntroduce;
    animal.PrintInfo = AnimalPrintInfo;

    animal.Speak();
    animal.PrintInfo(&animal);

    Dog dog;
    dog.base = animal;
    dog.is_pet = true;
    dog.Bark = DogBark;

    dog.Bark();

    // 1. 通过 dog.base 进行访问
    dog.base.Speak();
    dog.base.PrintInfo(&dog.base);

    // 2. 通过指针将 dog 强制转换成 Animal 类型的指针进行访问
    Animal *animal_ptr = (Animal *)&dog;
    animal_ptr->Speak();
    animal_ptr->PrintInfo(animal_ptr);

    // 多态实现
    dog.base.Speak = DogIntroduce;
    dog.base.PrintInfo = DogPrintInfo;

    dog.base.Speak();
    dog.base.PrintInfo((Animal *)&dog);

    return 0;
}
```

**输出结果：**

```sh
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Woof! Woof!
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Hello, I am an animal.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Hello, I am a dog.
Name: Buddy
Age: 5
Weight: 30
Height: 60
Is Pet: Yes
```

作者：nuo534202
链接：https://zhuanlan.zhihu.com/p/1998837850469320582
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。