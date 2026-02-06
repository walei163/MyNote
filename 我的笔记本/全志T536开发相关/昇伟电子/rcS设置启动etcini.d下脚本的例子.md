# rcS设置启动/etc/init.d下脚本的例子

官方默认的文件系统，需要修改/etc/init.d/rcS，来启动/etc/init.d/Sxx带数字的各个脚本。Copilot给了一个代码示例。已测试通过：

```sh
#!/bin/sh
# 简单的 rcS：执行 /etc/init.d/ 下以 S 开头并带数字的可执行脚本（按数字顺序）

SCRIPTS_DIR="/etc/init.d"

# 找到匹配文件，按“版本/数字”排序（需要 GNU sort 支持 -V）
for script in $(find "$SCRIPTS_DIR" -maxdepth 1 -type f -name 'S[0-9]*' -print | sort -V); do
  [ -x "$script" ] || continue
  echo "启动: $script"
  # 传递 start 参数（若脚本支持），若不支持可改为直接执行 "$script"
  if ! "$script" start; then
    echo "警告：$script 执行失败" >&2
  fi
done
```

