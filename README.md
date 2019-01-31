
sudo apt-get install libncurses5-dev zlib1g-dev gawk flex patch git-core g++ subversion //安装编译依赖

./scripts/feeds update -a //更新软件包

./scripts/feeds install -a //安装更新包

cp configbak .config //将当前config_7620配置文件拷贝 .config

make menuconfig //配置软件包

make kernel_menuconfig	//配置kernel

make V=s //编译
