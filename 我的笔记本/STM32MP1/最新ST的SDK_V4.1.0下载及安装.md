# 最新ST的SDK_V4.1.0下载及安装
1 STM32MPU-Ecosystem-v4.1.0 release↑

The STM32MP1 SDK is delivered through a tarball file named : 
**en.SDK-x86_64-stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.tar.gz**

Download and install the STM32MP1 SDK. The software package is provided AS IS, and by downloading it, you agree to be bound to the terms of the software license agreement (SLA). The detailed content licenses can be found here.
Warning white.png Warning
To download a package, it is recommended to be logged in to your "myst" account [1]. If, trying to download, you encounter a “403 error”, you could try to empty your browser cache to workaround the problem. We are working on the resolution of this problem.
We apologize for this inconvenience
	STM32MP1 Developer Package SDK - STM32MP1-Ecosystem-v4.1.0 release
Download 	

You need to be logged on my.st.com before accessing the following link:
[en.SDK-x86_64-stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.tar.gz](https://www.st.com/en/embedded-software/stm32mp1dev.html#get-software/en.SDK-x86_64-stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.tar.gz)
Installation 	
    Uncompress the tarball file to get the SDK installation script.

 ```
 tar xvf en.SDK-x86_64-stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.tar.gz
 ```
 
If needed, change the permissions on the SDK installation script so that it is executable.

 ```
 chmod +x stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23/sdk/st-image-weston-openstlinux-weston-stm32mp1-x86_64-toolchain-4.0.4-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.sh
 ```

Run the SDK installation script.
        Use the -d <SDK installation directory absolute path> option to specify the absolute path to the directory in which you want to install the SDK (<SDK installation directory>).
        If you follow the proposition to organize the working directory, it means:

 ```
 ./stm32mp1-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23/sdk/st-image-weston-openstlinux-weston-stm32mp1-x86_64-toolchain-4.0.4-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23.sh -d <working directory absolute path>/Developer-Package/SDK
 ```
        
A successful installation outputs the following log:


ST OpenSTLinux - Weston - (A Yocto Project Based Distro) SDK installer version 4.0.4-openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23
 ===================================================================================================================================
You are about to install the SDK to "<working directory absolute path>/Developer-Package/SDK". Proceed [Y/n]? 
Extracting SDK.........................................................................................................................................................................................................................done
Setting it up...done
SDK has been successfully set up and is ready to be used.
Each time you wish to use the SDK in a new shell session, you need to source the environment setup script e.g.
 $ . <working directory absolute path>/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi

Release note 	

Details about the content of the SDK are available in the associated STM32 MPU ecosystem release note.

Archive box.png If you are interested in older releases, please have a look into the section Archives.
    The SDK is in the <SDK installation directory>:

<SDK installation directory>                                      SDK for OpenSTLinux distribution: details in Standard SDK directory structure article
├── environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi  Environment setup script for Developer Package
├── site-config-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
├── sysroots
│   ├── cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi                Target sysroot (libraries, headers, and symbols)
│   │   └── [...]
│   └── x86_64-ostl_sdk-linux                                     Native sysroot (libraries, headers, and symbols)
│       └── [...]
└── version-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi


