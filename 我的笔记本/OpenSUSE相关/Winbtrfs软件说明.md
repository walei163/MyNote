# Winbtrfs软件说明

## WinBtrfs v1.9



WinBtrfs is a Windows driver for the next-generation Linux filesystem Btrfs. A reimplementation from scratch, it contains no code from the Linux kernel, and should work on any version from Windows XP onwards. It is also included as part of the free operating system [ReactOS](https://www.reactos.org/).

If your Btrfs filesystem is on a MD software RAID device created by Linux, you will also need [WinMD](https://github.com/maharmstone/winmd) to get this to appear under Windows.

See also [Quibble](https://github.com/maharmstone/quibble), an experimental bootloader allowing Windows to boot from Btrfs, and [Ntfs2btrfs](https://github.com/maharmstone/ntfs2btrfs), a tool which allows in-place conversion of NTFS filesystems.

First, a disclaimer:

You use this software at your own risk. I take no responsibility for any damage it may do to your filesystem. It ought to be suitable for day-to-day use, but make sure you take backups anyway.

Everything here is released under the GNU Lesser General Public Licence (LGPL); see the file LICENCE for more info. You are encouraged to play about with the source code as you will, and I'd appreciate a note ([mark@harmstone.com](mailto:mark@harmstone.com)) if you come up with anything nifty.

See at the end of this document for copyright details of third-party code that's included here.

## Features



- Reading and writing of Btrfs filesystems
- Basic RAID: RAID0, RAID1, and RAID10
- Advanced RAID: RAID5 and RAID6
- Caching
- Discovery of Btrfs partitions, even if Windows would normally ignore them
- Getting and setting of Access Control Lists (ACLs), using the xattr security.NTACL
- Alternate Data Streams (e.g. :Zone.Identifier is stored as the xattr user.Zone.Identifier)
- Mappings from Linux users to Windows ones (see below)
- Symlinks and other reparse points
- Shell extension to identify and create subvolumes, including snapshots
- Hard links
- Sparse files
- Free-space cache
- Preallocation
- Asynchronous reading and writing
- Partition-less Btrfs volumes
- Per-volume registry mount options (see below)
- zlib compression
- LZO compression
- LXSS ("Ubuntu on Windows") support
- Balancing (including resuming balances started on Linux)
- Device addition and removal
- Creation of new filesystems with `mkbtrfs.exe` and `ubtrfs.dll`
- Scrubbing
- TRIM/DISCARD
- Reflink copy
- Subvol send and receive
- Degraded mounts
- Free space tree (compat_ro flag `free_space_cache`)
- Shrinking and expanding
- Passthrough of permissions etc. for LXSS
- Zstd compression
- Windows 10 case-sensitive directory flag
- Oplocks
- Metadata UUID incompat flag (Linux 5.0)
- Three- and four-disk RAID1 (Linux 5.5)
- New checksum types (xxhash, sha256, blake2) (Linux 5.5)
- Block group tree (Linux 6.1)

## Todo



- Full fs-verity support (Linux 5.15)
- Zoned support (Linux 5.11) (HM-SMR not supported on Windows?)
- Defragmentation
- Support for Btrfs quotas
- Full transaction log support
- Support for Windows transactions (TxF)

## Installation



To install the driver, [download and extract the latest release](https://github.com/maharmstone/btrfs/releases), right-click btrfs.inf, and choose Install. The driver is signed, so should work out of the box on modern versions of Windows.

If you using Windows 10 and have Secure Boot enabled, you may have to make a Registry change in order for the driver to be loaded - see [below](https://github.com/maharmstone/btrfs?tab=readme-ov-file#secureboot).

WinBtrfs is also available on the following package managers:

- [Chocolatey](https://chocolatey.org/packages/winbtrfs)

```
choco install winbtrfs
```

​    

- [Scoop](https://scoop.sh/#/apps?q="winbtrfs-np"&s=0&d=1&o=true)

```
scoop bucket add nonportable
scoop install winbtrfs-np -g
```

​    

## Uninstalling



If you want to uninstall, from a command prompt run:

```
RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 btrfs.inf
```

​    

You may need to give the full path to btrfs.inf.

You can also go to Device Manager, find "Btrfs controller" under "Storage volumes", right click and choose "Uninstall". Tick the checkbox to uninstall the driver as well, and let Windows reboot itself.

If you need to uninstall via the registry, open regedit and set the value of HKLM\SYSTEM\CurrentControlSet\services\btrfs\Start to 4, to disable the service. After you reboot, you can then delete the btrfs key and remove C:\Windows\System32\drivers\btrfs.sys.

## Compilation



To compile with Visual C++ 2019, open the directory and let CMake do its thing. If you have the Windows DDK installed correctly, it should just work.

To compile with GCC on Linux, you will need a cross-compiler set up, for either `i686-w64-mingw32` or `x86_64-w64-mingw32`. Create a build directory, then use either `mingw-x86.cmake` or `mingw-amd64.cmake` as CMake toolchain files to generate your Makefile.

## Mappings



The user mappings are stored in the registry key HKLM\SYSTEM\CurrentControlSet\services\btrfs\Mappings. Create a DWORD with the name of your Windows SID (e.g. S-1-5-21-1379886684-2432464051-424789967-1001), and the value of your Linux uid (e.g. 1000). It will take effect next time the driver is loaded.

You can find your current SID by running `wmic useraccount get name,sid`.

Similarly, the group mappings are stored in under GroupMappings. The default entry maps Windows' Users group to gid 100, which is usually "users" on Linux. You can also specify user SIDs here to force files created by a user to belong to a certain group. The setgid flag also works as on Linux.

Note that processes running under User Access Control tokens create files as the BUILTIN\Administrators SID (S-1-5-32-544), rather as a user account.

## LXSS ("Ubuntu on Windows" / "Windows Subsystem for Linux")



The driver will passthrough Linux metadata to recent versions of LXSS, but you will have to let Windows know that you wish to do this. From a Bash prompt on Windows, edit `/etc/wsl.conf` to look like the following:

```
[automount]
enabled = true
options = "metadata"
mountFsTab = false
```

​    

It will then take effect next time you reboot. Yes, you should be able to chroot into an actual Linux installation, if you wish.

## Commands



The DLL file shellbtrfs.dll provides the GUI interface, but it can also be used with rundll32.exe to carry out some tasks from the command line, which may be useful if you wish to schedule something to run periodically.

Bear in mind that rundll32 provides no mechanism to return any error codes, so any of these commands may fail silently.

- `rundll32.exe shellbtrfs.dll,CreateSubvol <path>`
- `rundll32.exe shellbtrfs.dll,CreateSnapshot <source> <destination>`
- `rundll32.exe shellbtrfs.dll,ReflinkCopy <source> <destination>` This also accepts wildcards, and any number of source files.

The following commands need various privileges, and so must be run as Administrator to work:

- `rundll32.exe shellbtrfs.dll,SendSubvol <source> [-p <parent>] [-c <clone subvol>] <stream file>` The -p and -c flags are as `btrfs send` on Linux. You can specify any number of clone subvolumes.
- `rundll32.exe shellbtrfs.dll,RecvSubvol <stream file> <destination>`
- `rundll32.exe shellbtrfs.dll,StartScrub <drive>`
- `rundll32.exe shellbtrfs.dll,StopScrub <drive>`

## Troubleshooting



- How do I debug this?

On the releases page, there's zip files to download containing the PDBs. Or you can try the symbols server http://symbols.burntcomma.com/ - in windbg, set your symbol path to something like this:

```
symsrv*symsrv.dll*C:\symbols*http://msdl.microsoft.com/download/symbols;symsrv*symsrv.dll*C:\symbols*http://symbols.burntcomma.com
```

- The filenames are weird! or
- I get strange errors on certain files or directories!

The driver assumes that all filenames are encoded in UTF-8. This should be the default on most setups nowadays - if you're not using UTF-8, it's probably worth looking into converting your files.

- How do I get this working with Secure Boot turned on?

For the very latest versions of Windows 10, Microsoft introduced more onerous requirements for signing, which seemingly aren't available for open-source drivers.

To work around this, go to `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\CI\Policy` in Regedit, create a new DWORD value called `UpgradedSystem` and set to 1, and reboot.

Or you could always just turn off Secure Boot in your BIOS settings.

- The root of the drive isn't case-sensitive in LXSS

This is something Microsoft hardcoded into LXSS, presumably to stop people hosing their systems by running `mkdir /mnt/c/WiNdOwS`.

- How do I change the drive letter?

With the shell extension installed, right-click the drive in Explorer, click Properties, and go to the Btrfs tab. There should be a button which allows you to change the drive letter.

- I'm still having problems with drive letters

In Regedit, try deleting the relevant entries in `HKEY_LOCAL_MACHINE\SYSTEM\MountedDevices`, then rebooting.

- How do I format a partition as Btrfs?

Use the included command line program mkbtrfs.exe. We can't add Btrfs to Windows' own dialog box, unfortunately, as its list of filesystems has been hardcoded. You can also run `format /fs:btrfs`, if you don't need to set any Btrfs-specific options.

- I can't reformat a mounted Btrfs filesystem

If Windows' Format dialog box refuses to appear, try running format.com with the /fs flag, e.g. `format /fs:ntfs D:`.

- I can't mount a Synology NAS

Synology seems to use LVM for its block devices. Until somebody writes an LVM driver for Windows, you're out of luck.

- I can't mount a Thecus NAS

Thecus uses Linux's MD raid for its block devices. You will need to install [WinMD](https://github.com/maharmstone/winmd) as well.

- 64-bit Windows 7 won't load the driver

Make sure that you have [KB3033929](https://www.microsoft.com/en-gb/download/details.aspx?id=46148) installed. Or consider installing from an "escrow" ISO which includes all updates.

- The drive doesn't show up and Paragon software has been installed

Paragon's filesystem-reading software is known to disable automount. Disable or uninstall Paragon, then re-enable automount by running `diskpart` and typing `automount enable`.

- The drive doesn't show up on very old versions of Windows

On very old versions of Windows (XP, Server 2003?), Windows ignores Linux partitions entirely. If this is the case for you, try running `fdisk` on Linux and changing your partition type from 83 to 7.