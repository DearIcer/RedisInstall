@echo off
chcp 65001 >nul
echo ========================================
echo Redis 服务管理器 - Windows 打包脚本
echo ========================================
echo.

REM 设置变量
set BUILD_DIR=build-release
set DEPLOY_DIR=RedisInstall-Release
set QT_DIR=C:\Qt\6.5.3\msvc2019_64

REM 检查 Qt 路径是否存在
if not exist "%QT_DIR%" (
    echo [错误] Qt 安装目录不存在: %QT_DIR%
    echo 请修改脚本中的 QT_DIR 变量为你的 Qt 安装路径
    pause
    exit /b 1
)

echo [1/5] 清理旧的构建目录...
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%

echo [2/5] 创建构建目录...
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo [3/5] 运行 CMake 配置...
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_DIR%
if errorlevel 1 (
    echo [错误] CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo [4/5] 编译项目...
cmake --build . --config Release
if errorlevel 1 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..

echo [5/5] 部署应用程序...
mkdir %DEPLOY_DIR%

REM 复制可执行文件
copy %BUILD_DIR%\Release\RedisInstall.exe %DEPLOY_DIR%\

REM 使用 windeployqt 部署 Qt 依赖
echo 正在部署 Qt 依赖库...
%QT_DIR%\bin\windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw %DEPLOY_DIR%\RedisInstall.exe

if errorlevel 1 (
    echo [警告] windeployqt 执行出现问题，但继续打包...
)

echo.
echo ========================================
echo 打包完成！
echo 输出目录: %DEPLOY_DIR%
echo ========================================
echo.
echo 你可以将 %DEPLOY_DIR% 文件夹中的所有文件打包分发
echo.

REM 打开输出目录
explorer %DEPLOY_DIR%

pause
