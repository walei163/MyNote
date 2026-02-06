# 如何在github上管理自己的开发项目

## 采用ssl方式和github通讯：

### 1、先创建ssd公钥：

```sh
mkdir -p ~/.ssh && chmod 700 ~/.ssh
if [ -f ~/.ssh/id_ed25519.pub ]; then
  echo "Found existing ~/.ssh/id_ed25519.pub — will reuse it.";
else
  echo "No id_ed25519 found — generating new ed25519 key (no passphrase).";
  ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519 -C "wanglei@$(hostname)" -N "";
fi
echo
echo "----- PUBLIC KEY (copy the whole line) -----"
cat ~/.ssh/id_ed25519.pub || true
echo
echo "----- ~/.ssh contents -----"
ls -la ~/.ssh
echo
echo "To clone after you add the key to GitHub, run:"
echo "  git clone git@github.com:walei163/swm3310.git"
# Do not auto-run clone because key may not yet be added
```

公钥的内容为：

```sh
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIAeogAIUfokEXDhCJUUKvbzof+KWa+YyUIqp45l8DBY2 wanglei@sparky7
```

### 2、登录到github账号里，将公钥添加到账号中：

将下面显示的公钥复制并添加到你的 GitHub 帐号：

```
Settings → 
	SSH and GPG keys → 
		New SSH key
```

点击“New SSH key”，在出现的界面中，将刚刚的公钥内容复制到“Key”那一栏的文本框中，如下图所示：

<img src="/home/jason/BaiduSyncdisk/VNote笔记本_20200401/我的笔记本/_v_images/image-20260128144328084.png" alt="image-20260128144328084"  />

完成后，再点击“Add SSH key”，就完成了添加公钥的工作。

### 3、将repo克隆到本地：

```sh
git clone ssh://git@ssh.github.com:443/walei163/swm3310.git
```

如果本地已经创建好了同名的目录或者仓库，而想要直接推送本地到远程，可以按照如下操作：

#### 1）指定远程origin，如果已经存在可以先删除掉远程origin：

```sh
git remote remove origin
```

#### 2）指定远程origin：

```sh
git remote add origin ssh://git@ssh.github.com:443/walei163/DCA173.git
```

#### 3）可以查看验证一下：

```sh
git remote -v
```

#### 4）切换出本地main分支：

```sh
git brach -m main
```



### 4、将本地文件推送到远程：



```sh
git push origin main
```

或者：

```sh
git push -u origin main
```

两者区别是：

> [!tip]
>
> 作用: -u 是 --set-upstream 的简写：推送的同时把远程分支设置为本地分支的上游（tracking）。
> 首次推送新分支: 常用 -u，例如：
>
> ```sh
> git push -u origin feature/foo
> ```
>
> 这会推送 feature/foo 并把 origin/feature/foo 设为本地 feature/foo 的上游。
>
> - 后续推送: 以后直接用 git push 即可把当前分支推送到已设置的上游；若未设置上游，git push 可能要求你指定远程和分支。
> - 查看追踪状态: 使用 git branch -vv 可以看到本地分支对应的上游和最后一次提交。
> - 配置层面: git push -u 会写入 branch.<name>.remote 和 branch.<name>.merge，只需执行一次（每个分支）。
> - 总结: git push -u = 推送 + 设置上游（便于后续简化命令）；git push = 仅推送（若无上游需显式指定）。

---

> [!note]
>
> **github从本地向远程推送时，单个文件不能大于100MB，否则推送将会失败。**
>
> ```sh
> remote: error: Trace: 529e34e3798a7e3734cc458967f19af5dbb4e54fa082da8e05e65c72216e2e55
> remote: error: See https://gh.io/lfs for more information.
> remote: error: File 软件相关/DCA173_V1.0_系统更新包_20251221.tar.gz is 107.09 MB; this exceeds GitHub's file size limit of 100.00 MB
> remote: error: GH001: Large files detected. You may want to try Git Large File Storage - https://git-lfs.github.com.
> To ssh://ssh.github.com:443/walei163/DCA173.git
>  ! [remote rejected] main -> main (pre-receive hook declined)
> 错误：无法推送一些引用到 'ssh://ssh.github.com:443/walei163/DCA173.git'
> ```

---

### 5、本地与远程同步常见操作：

#### 快速检查

- 查看远程: 

​	git remote -v — 确认 origin 指向哪个仓库。

#### 拉取远程更新

- 抓取最新元数据: 

​	git fetch origin

- 合并远程到当前分支（默认合并）: 

​	git pull

- 或用 rebase 保持历史整洁: 

​	git pull --rebase

#### 推送本地提交

- 推送当前分支到 origin: 

​	git push origin <branch>

- 首次推送并设置上游分支: 

​	git push -u origin <branch>

#### 创建/切换并跟踪远程分支

- 新建本地分支并切换: 

​	git checkout -b myfeature

- 推送并设置上游: 

​	git push -u origin myfeature

#### 如果你 fork 了并想与上游仓库同步

- 添加上游远程: 

​	git remote add upstream git@github.com:ORIGINAL/REPO.git

- 抓取上游并合并到本地主分支:

```sh
git fetch upstream
git checkout main
git pull --rebase upstream main
```

- 然后推送到你的 origin: 

​	git push origin main

#### 冲突处理（简要）

- 发生冲突时：编辑冲突文件，

​	git add <file>，

然后 

​	git rebase --continue 

或 

​	git commit（取决于操作）。

完成后再 

​	git push。