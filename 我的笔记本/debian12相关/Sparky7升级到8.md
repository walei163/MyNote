# Sparky7升级到8的方法

## Upgrade Sparky from 7 to 8

> [!note]
>
> It should work with no problems, but remember: You do that on your own risk!

To make dist upgrade from Sparky 7 to 8 do:

1. Backup your personal files to external disk or partition.
2. Change Debian repositories from<font color=blue> **bookworm**</font> to <font color=blue>**trixie**</font> at the file:

```sh
 /etc/apt/sources.list
```

So it should look like that:

```sh
 deb http://deb.debian.org/debian trixie main contrib non-free non-free-firmware
 deb-src http://deb.debian.org/debian trixie main contrib non-free non-free-firmware
 deb http://security.debian.org/debian-security/ trixie-security/updates main contrib non-free non-free-firmware
 deb-src http://security.debian.org/debian-security/ trixie-security/updates main contrib non-free non-free-firmware
 deb http://deb.debian.org/debian trixie-updates main contrib non-free non-free-firmware
 deb-src http://deb.debian.org/debian trixie-updates main contrib non-free non-free-firmware
 deb http://deb-multimedia.org/ trixie main non-free
```

3. Change Sparky repositories from <font color=blue>**orion** </font>to <font color=blue>**sisters**</font> at the file:

```sh
 /etc/apt/sources.list.d/sparky.list
```

So it should look like that:

```sh
 deb [signed-by=/usr/share/keyrings/sparky.gpg.key] https://repo.sparkylinux.org/ core main
 deb-src [signed-by=/usr/share/keyrings/sparky.gpg.key] https://repo.sparkylinux.org/ core main
 deb [signed-by=/usr/share/keyrings/sparky.gpg.key] https://repo.sparkylinux.org/ sisters main
 deb-src [signed-by=/usr/share/keyrings/sparky.gpg.key] https://repo.sparkylinux.org/ sisters main
```

4. Refresh package list:

```sh
 sudo apt update
```

5. Dist upgrade now:

```sh
 sudo apt full-upgrade
```

If any problem run:

```sh
 sudo dpkg-reconfigure -a
 sudo apt install -f
```