# ARM64系统上支持32位armhf

```sh
sudo dpkg --add-architecture armhf
sudo apt update

sudo apt-get install libc6:armhf
sudo apt-get install libc6-dev:armhf
sudo apt install libc6-dev-armhf-cross
sudo apt-get install libstdc++6:armhf
```

建议安装：

```
glibc-doc:armhf locales:armhf libnss-nis:armhf libnss-nisplus:armhf
```

推荐安装：

```
libidn2-0:armhf
```

