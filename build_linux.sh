#!/bin/bash

echo "========================================"
echo "Redis 服务管理器 - Linux 编译脚本"
echo "========================================"
echo ""

# 设置变量
BUILD_DIR="build-linux"
INSTALL_DIR="RedisInstall-Linux"

# 检查依赖
echo "[1/6] 检查依赖..."
if ! command -v cmake &> /dev/null; then
    echo "[错误] 未找到 CMake，请先安装: sudo apt install cmake"
    exit 1
fi

if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo "[错误] 未找到 Qt6，请先安装: sudo apt install qt6-base-dev build-essential"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "[错误] 未找到 make，请先安装: sudo apt install build-essential"
    exit 1
fi

echo "[2/6] 清理旧的构建目录..."
rm -rf $BUILD_DIR
rm -rf $INSTALL_DIR

echo "[3/6] 创建构建目录..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo "[4/6] 运行 CMake 配置..."
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "[错误] CMake 配置失败"
    cd ..
    exit 1
fi

echo "[5/6] 编译项目..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "[错误] 编译失败"
    cd ..
    exit 1
fi

cd ..

echo "[6/6] 创建发布包..."
mkdir -p $INSTALL_DIR
cp $BUILD_DIR/RedisInstall $INSTALL_DIR/

# 创建启动脚本
cat > $INSTALL_DIR/start.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
export LD_LIBRARY_PATH="$DIR:$LD_LIBRARY_PATH"
./RedisInstall "$@"
EOF

chmod +x $INSTALL_DIR/start.sh
chmod +x $INSTALL_DIR/RedisInstall

echo ""
echo "========================================"
echo "编译完成！"
echo "========================================"
echo ""
echo "输出目录: $INSTALL_DIR"
echo ""
echo "运行方式："
echo "  cd $INSTALL_DIR"
echo "  ./start.sh"
echo ""
