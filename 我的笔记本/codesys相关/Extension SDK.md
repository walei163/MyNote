# Extension SDK：扩展SDK实现方法

该页网址：https://content.helpme-codesys.com/zh-CHS/CODESYS%20Control/_rtsl_extension_extension_sdk.html

使用扩展 SDK，您可以在 C 中实现自己的函数并从 IEC 应用程序中调用它们。

对于这种连接，C 代码以单独的动态组件的形式集成到运行时系统中（`shared object`）。

有两种方法包括：

- 在控制器上准备：动态组件已配置为 CODESYS 运行时系统（一个 `shared object` 存在并在 Linux 系统上配置）。然后，只需要相应的 IEC 库。
- 以带有集成动态组件的 IEC 库的形式。 Linux 系统本身不需要准备任何东西。使用 IEC 库时，动态组件会自动添加到 CODESYS 运行时系统和使用。

在这两种情况下，用户都必须能够创建一个 `shared object` 适用于运行时系统的架构（x64/ARM6/ARM7/AARCH64）。扩展SDK支持您创建这种动态组件 `(shared object`）。

下面介绍如何为这两种变体创建库和动态组件。

## 开发环境

开发环境

- 打开 CODESYS 并创建一个新库。
- 设置库的标题（项目信息, 概括 选项卡）并以相同的名称保存库。
- 添加所需的功能并将其标记为 外部实施.
  - 在项目导航器中功能的上下文菜单中，单击 特性.在 建造 选项卡，选择 外部实施.
  - 注意：函数必须包含“`_cext`“ 在名字里。
- 现在生成 *c 和 *m4 文件：
  - 点击 构建 → 生成运行时系统文件.
  - 选择 M4接口文件 和 C 存根文件.
  - 选择保存位置。

## Linux

1. 将 Extension SDK 复制到具有目标系统相应编译环境的 Linux 系统中。

   默认情况下，您可以在以下位置找到扩展 SDK：

   `C:\Program Files\CODESYS <version>\CODESYS\CODESYS Control SL Extension Package\<version>\ExtensionSDK`. 在以下步骤中，该目录将被称为 `<SDK DIR>` 。

2. 创建一个新文件夹，您可以在其中创建 C 实现。这个文件夹在下面 `<PROJECT DIR>`.

3. 导航 `<PROJECT DIR>` 并运行： `$ make -f <SDKDIR>/makefile newproject`

4. 将您在开发环境中创建的 *c 和 *m4 文件复制到 `<PROJECT DIR>`.

5. 实现 C 中的函数。

6. 创建共享对象： `$ make all`

   这 `shared object` 生成并位于 `<PROJECT DIR>/out/libCmpMyTest.so`.

因此，您创建了一个动态组件，现在可以通过您的 IEC 库从 IEC 应用程序调用该组件。有关构建过程的详细信息（`make`) 也可以在 makefile 和 `README file` 在扩展 SDK 中。

现在决定使用以下两种方法之一来包含该组件：

## 包括动态组件

您可以通过以下选项来包含该组件：

### 在目标设备上包含动态组件

此方法允许您将以前在 C 中创建的共享对象包含到运行时系统中。用户不需要额外的系统权限，并且可以使用熟悉的方法将库推广到设备。

1. 将上面创建的共享对象复制到运行时系统。

2. 将共享对象复制到 `/usr/lib/` 或系统在其中找到库的等效库目录。

3. 停止运行时系统（例如： `$ sudo /etc/init.d/codesyscontrol stop`)

4. 打开 `/etc/CODESYSControl_User.cfg` 具有管理员权限： `$ sudo nano /etc/CODESYSControl_User.cfg`

5. 在里面 `[ComponentManager]` 部分，添加创建的共享对象：

   ```
   [ComponentManager]
   Component.1=CmpFirstTest
   Component.2=CmpSomeOtherComponent
   ...
   ```

   注：编号为增量编号，不得与现有组件冲突。

6. 启动运行时系统： `$ sudo /etc/init.d/codesyscontrol start`

   现在您可以检查组件是否已加载： `$ cat /tmp/codesyscontrol.log`

### 在 IEC 库中包含动态组件

使用“C 代码集成”功能将动态组件添加到 IEC 库。有关详细信息，请参阅 C 代码集成 在一般帮助中。

1. 打开 IEC 库。

2. 在 POU, 添加一个 C 实现的库 目的。

3. 在这个对象中，添加一个 设备的目标文件：

   - 选择之前构建的动态组件（文件）并添加它。
   - 指定合适的组件名称。
   - 选择合适的设备。

   现在，当将此库与所选设备一起使用时，动态组件会自动添加到运行时系统中，并且可以使用库的 POU 调用。

## 其他

限制：

- 使用扩展 SDK，只能包含 C 代码。
- 调试器不能用于检查或调试 C 代码（例如，在编程系统中）。
- C 代码不能与运行时系统或 IEC 应用程序“交互”。这意味着：
  - 无法自动访问 IEC 应用程序的变量。
  - 无法访问运行时系统的功能。但是，通过将变量传递给用 C 代码实现的函数，当然可以访问 IEC 应用程序的变量 `by reference` （例如，通过 `POINTER`）。

| 接口的使用仅作为 Linux 的成员 `codesysuser` 用户组 | 不   |
| -------------------------------------------------- | ---- |
| 流程分离                                           | 不   |