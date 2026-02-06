# 如何制作一个基于OverlayFS的根文件系统

walei163: 如何制作一个基于overlayfs的根文件系统

GitHub Copilot: 要制作一个基于 `overlayfs` 的根文件系统，可以按照以下步骤进行：

1. **准备基础目录结构**：
   创建三个目录，分别用于存放底层文件系统（lowerdir）、上层文件系统（upperdir）和工作目录（workdir）。

   ```sh
   mkdir -p /mnt/lowerdir /mnt/upperdir /mnt/workdir /mnt/merged
   ```

2. **准备底层文件系统**：
   将现有的根文件系统复制到 `lowerdir` 目录中。可以使用 `rsync` 或者 `cp` 命令。

   ```sh
   rsync -a / /mnt/lowerdir
   ```

3. **准备上层文件系统**：
   上层文件系统用于存放对文件系统的修改。可以是一个空目录。

   ```sh
   mkdir -p /mnt/upperdir
   ```

4. **挂载 overlayfs**：
   使用 `mount` 命令将 `overlayfs` 挂载到 `merged` 目录。

   ```sh
   mount -t overlay overlay -o lowerdir=/mnt/lowerdir,upperdir=/mnt/upperdir,workdir=/mnt/workdir /mnt/merged
   ```

5. **切换根文件系统**：
   使用 `pivot_root` 或者 `chroot` 命令切换到新的根文件系统。

   ```sh
   cd /mnt/merged
   pivot_root . mnt
   exec chroot . /bin/sh
   ```

6. **更新 fstab**：
   在新的根文件系统中更新 

fstab

 文件，以便在系统启动时自动挂载 `overlayfs`。

   ```sh
   echo "overlay / overlay lowerdir=/mnt/lowerdir,upperdir=/mnt/upperdir,workdir=/mnt/workdir 0 0" >> /etc/fstab
   ```

这样，你就可以创建并使用一个基于 `overlayfs` 的根文件系统了。

> [!note]
>
> 注意：如果在嵌入式系统中，想要跨2片eMMC分区来进行文件系统的overlay，则需要查看内核是否开启了：
>
> ```sh
> CONFIG_OVERLAY_FS_INDEX=Y
> CONFIG_OVERLAY_FS_METACOPY=Y
> ```
>
> 则还需要添加：
>
> ```sh
> index=off, metacopy=off
> ```

以下是Copilot给出的原因分析：

---

walei163: 测试中发现，我的overlayfs不能在两个flash分区上使用，必须要设置“index=off, metacopy=off”，这是为什么

GitHub Copilot: 在 `overlayfs` 中使用 `index=off` 和 `metacopy=off` 选项是因为某些文件系统（例如某些类型的闪存分区）可能不支持这些功能。以下是这两个选项的详细解释：

1. **index=off**：
   - `index` 选项用于在上层目录中创建索引文件，以便更高效地查找文件。某些文件系统可能不支持索引文件的创建，或者索引文件可能会导致性能问题或兼容性问题。
   - 关闭索引功能可以避免这些问题，但可能会导致某些操作变得稍微慢一些。

2. **metacopy=off**：
   - `metacopy` 选项用于在上层目录中仅复制文件的元数据，而不是整个文件内容。这可以节省空间和时间，但需要底层文件系统支持元数据复制功能。
   - 关闭元数据复制功能可以确保所有文件操作都在上层目录中完成，避免了底层文件系统不支持元数据复制的问题。

在闪存分区上使用 `overlayfs` 时，可能会遇到以下问题：

- 闪存分区可能不支持索引文件或元数据复制功能。
- 闪存分区的性能可能会受到索引文件或元数据复制的影响。

因此，设置 `index=off` 和 `metacopy=off` 可以确保 `overlayfs` 在闪存分区上正常工作。

你可以使用以下命令挂载 `overlayfs`，并确保设置了这两个选项：

```sh
mount -t overlay overlay -o lowerdir=/mnt/lowerdir,upperdir=/mnt/upperdir,workdir=/mnt/workdir,index=off,metacopy=off /mnt/merged
```

这样可以确保 `overlayfs` 在两个闪存分区上正常工作。