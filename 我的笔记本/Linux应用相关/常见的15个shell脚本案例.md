# 常见的15个shell脚本案例

Created: March 20, 2024 9:44 AM
Tags: Linux应用开发相关, Linux应用相关
Property 1: Jason Wang

# **常见的15个shell脚本案例，请收藏**

2024-02-23 21:55·[懒人编程](https://www.toutiao.com/c/user/token/MS4wLjABAAAAydt8mEdDqr7pTUJBVocGa2vdd2V-6w2qSUeCj_rwmH8/?source=tuwen_detail)

1. **Hello World** - 最简单的脚本示例：

```bash
#!/bin/bash
echo "Hello, World!"
```

2. **计算目录下文件数目** - 统计当前目录下文件数量：

```bash
#!/bin/bash
count=$(ls -1 | wc -l)
echo "There are $count files in the current directory."
```

3. **创建备份** - 按时间戳备份文件或目录：

```bash
#!/bin/bash
filename="important_file.txt"
timestamp=$(date +%Y%m%d%H%M%S)
cp "$filename" "${filename}_$(date +'%Y%m%d')_backup"
```

4. **查找并删除空文件夹** - 在当前目录及其子目录中删除空的子目录：

```bash
#!/bin/bash
find . -type d -empty -exec rmdir {} \;
```

5. **进程监控与重启** - 当某个进程不存在时自动启动它（例如服务）：

```bash
#!/bin/bash
process_name="my_server"
if ! pgrep -x "$process_name" > /dev/null; then
	/path/to/start_script.sh &
fi
```

6. **获取系统基本信息** - 显示系统CPU、内存使用情况：

```bash
#!/bin/bash
echo "CPU Usage: $(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}')%"
free -m | awk 'NR==2{printf "Memory Usage: %s/%sMB (%.2f%%)\n", $3,$2,$3*100/$2}'
```

7. **定时执行任务** - 创建一个每分钟执行的任务计划（需配合crontab）：

```bash
#!/bin/bash
echo "* * * * * /path/to/script.sh" >> ~/.crontab
crontab ~/.crontab
```

8. **文件内容替换** - 替换文本文件中的特定字符串：

```bash
#!/bin/bash
sed -i 's/old_string/new_string/g' file.txt
```

9. **下载文件** - 使用curl命令从互联网下载文件：

```bash
#!/bin/bash
url="https://example.com/file.zip"
wget -O downloaded_file.zip "$url"
```

10. **批量重命名文件** - 根据某种模式重命名一组文件：

```bash
#!/bin/bash
for file in *.txt; do
	mv "$file" "${file%.txt}.md"
done
```

11. **压缩和解压文件** - 压缩目录为tar.gz格式：

```bash
#!/bin/bash
tar -czvf archive.tar.gz directory_to_compress/
```

解压tar.gz文件：

```css
tar -xzvf archive.tar.gz
```

12. **检查磁盘空间** - 输出磁盘剩余空间信息：

```bash
#!/bin/bash
df -hT /home
```

13. **查找大文件** - 查找当前目录下大于100MB的文件：

```bash
#!/bin/bash
du -h --max-depth=1 | grep '[0-9\.]\+G'
```

14. **用户输入处理** - 提示用户输入，并将输入写入文件：

```bash
#!/bin/bash
read -p "请输入一行文字：" input_text
echo "$input_text" >> user_input.log
```

15. **检查网络连接** - 简单地测试网络连通性：

```bash
#!/bin/bash
ping -c 1 google.com && echo "网络连接正常" || echo "网络连接异常"
```