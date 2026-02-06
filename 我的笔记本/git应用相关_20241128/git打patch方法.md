# centos git diff工具 git diff -p

前言

我们合并别人的代码时候有些都在服务器可以使用cherry-pick 、merge、rebase等，但有时候有些修改是本地的，有些做了单独commit，可以使用git format需要做一个patch文件，但是有些时候有些修改是临时的，没有提交，这个时候我们也可以用git diff命令进行生成patch文件，然后使用patch命令合入，下面就给大家做一个分享。

作者：良知犹存

---

## diff 和 patch

我们经常会使用diff和patch两个组合命令生成patch文件，然后进行合入。一般使用如下：生成patch文件，patch打入

```sh
diff -Nur src src_new >src.patch 
patch -p[n] < src.patch
```

这个比较多使用，文章也比较多，这里不多赘述。

## git diff

今天描述的是git diff配合patch,下面是git diff的详细介绍：

|                 参数                 | 描述                                                         |
| :----------------------------------: | :----------------------------------------------------------- |
|               git diff               | 查看尚未暂存的文件更新了哪些部分，此命令比较的是工作目录(Working tree)和暂存区域快照(index)之间的差异。也就是修改之后还没有暂存起来的变化内容。 |
|          git diff --cached           | 查看已经暂存起来的文件(staged)和上次提交时的快照之间(HEAD)的差异。显示的是下一次 commit 时会提交到 HEAD 的内容(不带 -a 情况下) |
|            git diff HEAD             | 显示工作版本(Working tree)和 HEAD 的差别                     |
|          git diff filename           | 查看尚未暂存的某个文件的更新                                 |
|      git diff --cached filename      | 查看已经暂存起来的某个文件和上次提交的版本之间的差异         |
| git diff sha1:filename sha2:filename | 查看版本 sha1 的文件 filename 和版本 sha2 的文件 filename 的差异 |
|        git diff topic master         | 直接将两个分支上最新的提交做比较                             |
|        git diff topic…master         | 输出自 topic 和 master 分别开发以来，master 分支上的 changed |
|           git diff --stat            | 查看简单的 diff 结果，可以加上 --stat 参数                   |
|            git diff test             | 显示当前目录和另一个叫 `test` 分支的差别                     |
|        git diff HEAD – ./lib         | 显示当前目录下的 lib 目录和上次提交之间的差别                |
|         git diff HEAD^ HEAD          | 比较上次提交 commit 和上上次提交                             |
|          git diff SHA1 SHA2          | 比较两个历史版本之间的差异                                   |

## git diff 与 patch 使用实例

以下是一个暂未提交的修改，通过 `git status`可以查看

![image-20241128191904330](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/image-20241128191904330.png)

我们可以通过 这个`git diff`，先查看对比信息

![centos git diff工具 git diff -p_当前目录_02](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/resize,m_fixed,w_1184)

之后使用基本命令生成patch `git diff > test.patch`

![centos git diff工具 git diff -p_当前目录_03](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1184)

### patch命令 打补丁

直接打补丁 `patch -p1 < test.patch`

![centos git diff工具 git diff -p_当前目录_04](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1185)

也可以到下层目录，使用p2进行打补丁 `patch -p2 < test.patch`

![centos git diff工具 git diff -p_当前目录_05](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1186)

`patch -p[n] < test.patch` 其中n为层级,层级的详细解释：

假如补丁头是

> --- src/a/b/c/d/file     
> +++ src_new/a/b/c/d/file

```sh
使用p0 表示在当前目录下查找src/a/b/c/d/file
使用p1 表示在当前目录下查找a/b/c/d/file
使用p2 表示在当前目录下查找b/c/d/file
使用p3 表示在当前目录下查找c/d/file
使用p4 表示在当前目录下查找d/file
使用p5 表示在当前目录下查找file
不使用pn表示忽略所有斜杠，在当前目录下查找file
```

### git apply 打补丁

除了patch命令，我们也可以使用`git apply`,这里因为没有commit信息所以不能用 `git am`,使用应用补丁之前我们可以先检验一下补丁能否应用，

`git apply --check patch` 如果没有任何输出，那么表示可以顺利接受这个补丁

```sh
git apply --stat test.patch
git apply --check test.patch
```

![centos git diff工具 git diff -p_centos git diff工具_06](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1187)

git apply --check 没有提示信息，表示正常，直接进行打入补丁 `git apply test.patch`

![centos git diff工具 git diff -p_公众号_07](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1188)

因为没有提交信息 所以`git am`无法使用

![centos git diff工具 git diff -p_centos git diff工具_08](/media/sf_D_DRIVE/BaiduNetdiskWorkspace/VNote笔记本_20200401/我的笔记本/git应用相关_20241128/images/resize,m_fixed,w_1189)

## 结语

这就是我自己的一些git diff打patch使用分享。 

centos git diff工具 git diff -p
https://blog.51cto.com/u_16099165/10724080