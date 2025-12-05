#!/bin/bash

echo "========================================"
echo "Redis 服务管理器 - Linux 依赖安装脚本"
echo "========================================"
echo ""

# 检测 Linux 发行版
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "[错误] 无法检测 Linux 发行版"
    exit 1
fi

echo "[检测到系统]: $PRETTY_NAME"
echo ""

# 根据不同发行版安装依赖
case $OS in
    ubuntu|debian|pop)
        echo "[安装 Ubuntu/Debian 依赖]"
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            qt6-base-dev \
            libqt6core6 \
            libqt6gui6 \
            libqt6widgets6 \
            libqt6network6 \
            gcc \
            g++ \
            make \
            tar \
            wget
        ;;
    
    fedora|rhel|centos)
        echo "[安装 Fedora/RHEL/CentOS 依赖]"
        sudo dnf install -y \
            gcc \
            gcc-c++ \
            cmake \
            qt6-qtbase-devel \
            make \
            tar \
            wget
        ;;
    
    arch|manjaro)
        echo "[安装 Arch/Manjaro 依赖]"
        sudo pacman -S --noconfirm \
            base-devel \
            cmake \
            qt6-base \
            gcc \
            make \
            tar \
            wget
        ;;
    
    *)
        echo "[警告] 未识别的 Linux 发行版: $OS"
        echo "请手动安装以下依赖："
        echo "  - build-essential / gcc / g++"
        echo "  - cmake"
        echo "  - Qt 6.5+ (qt6-base-dev)"
        echo "  - make, tar, wget"
        exit 1
        ;;
esac

echo ""
echo "========================================"
echo "依赖安装完成！"
echo "========================================"
echo ""
echo "现在可以运行编译脚本："
echo "  ./build_linux.sh"
echo ""
