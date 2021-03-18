OpenWRT的feeds包括：

    packages – 提供众多库, 工具等基本功能. 也是其他feed所依赖的软件源, 因此在安装其他feed前一定要先安装packages!
    luci – OpenWrt默认的GUI(WEB管理界面).
    xwrt – 另一种可替换LuCI的GUI
    qpe – DreamBox维护的基于Qt的图形界面, 包含Qt2, Qt4, Qtopia, OPIE, SMPlayer等众多图形界面.
    device – DreamBox维护与硬件密切相关的软件, 如uboot, qemu等.
    dreambox_packages – DreamBox维护的国内常用网络工具, 如oh3c, njit8021xclient等.
    desktop - OpenWrt用于桌面的一些软件包.
    xfce - 基于Xorg的著名轻量级桌面环境. Xfce建基在GTK+2.x之上, 它使用Xfwm作为窗口管理器.
    efl - 针对enlightenment.
    phone -针对fso, paroli.

trunk中默认的feeds下载有packages、xwrt、luci、routing、telephony。如要下载其他的软件包，需打开源码根目录下面的feeds.conf.default文件，去掉相应软件包前面的#号，然后更新源:

./scripts/feeds update -a

安装下载好的包:

./scripts/feeds install -a
OpenWrt源码目录结构：

    tools和toolchain目录：包含了一些通用命令, 用来生成固件, 编译器, 和C库.
    build dir/host目录：是一个临时目录, 用来储存不依赖于目标平台的工具.
    build dir/toolchain-目录：用来储存依赖于指定平台的编译链. 只是编译文件存放目录无需修改.
    build dir/target-目录：用来储存依赖于指定平台的软件包的编译文件, 其中包括linux内核, u-boot, packages, 只是编译文件存放目录无需修改.
    staging_dir目录：是编译目标的最终安装位置, 其中包括rootfs, package, toolchain.
    package目录：软件包的下载编译规则, 在OpenWrt固件中, 几乎所有东西都是.ipk, 这样就可以很方便的安装和卸载.
    target目录：目标系统指嵌入式设备, 针对不同的平台有不同的特性, 针对这些特性, "target/linux"目录下按照平台进行目录划分, 里面包括了针对标准内核的补丁, 特殊配置等.
    bin目录：编译完OpenWrt的二进制文件生成目录, 其中包括sdk, uImage, u-boot, dts, rootfs构建一个嵌入式系统完整的二进制文件.
    config目录：存放着整个系统的的配置文件.
    docs目录：里面不断包含了整个宿主机的文件源码的介绍, 里面还有Makefile为目标系统生成docs.
    include目录：里面包括了整个系统的编译需要的头文件, 但是是以Make进行连接的.
    feeds目录：扩展软件包索引目录.
    scripts目录：组织编译整个OpenWrt的规则.
    tmp目录：编译文件夹, 一般情况为空.
    dl目录：所有软件的下载目录, 包括u-boot, kernel.
    logs目录：如果编译出错, 可以在这里找到编译出错的log.
	