@echo off
chcp 65001 >nul
echo ========================================
echo 快速打包工具 - 使用现有编译文件
echo ========================================
echo.

REM 设置变量 - 请根据实际情况修改
set QT_DIR=C:\Qt\6.9.1\mingw_64
set SOURCE_EXE=build\Desktop_Qt_6_9_1_MinGW_64_bit-Release\RedisInstall.exe
set DEPLOY_DIR=RedisInstall-Package

REM 检查可执行文件是否存在
if not exist "%SOURCE_EXE%" (
    echo [错误] 找不到可执行文件: %SOURCE_EXE%
    echo.
    echo 请检查以下位置：
    dir /s /b RedisInstall.exe
    echo.
    echo 请修改脚本中的 SOURCE_EXE 变量为正确的路径
    pause
    exit /b 1
)

REM 检查 Qt 目录
if not exist "%QT_DIR%" (
    echo [错误] Qt 目录不存在: %QT_DIR%
    echo 请修改脚本中的 QT_DIR 变量
    pause
    exit /b 1
)

echo [1/3] 清理旧的打包目录...
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%

echo [2/3] 创建打包目录并复制文件...
mkdir %DEPLOY_DIR%
copy "%SOURCE_EXE%" %DEPLOY_DIR%\

echo [3/3] 部署 Qt 依赖库...
%QT_DIR%\bin\windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw %DEPLOY_DIR%\RedisInstall.exe

if errorlevel 1 (
    echo [错误] windeployqt 执行失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo 打包完成！
echo ========================================
echo.
echo 输出目录: %DEPLOY_DIR%
echo.
echo 该目录包含所有运行所需的文件，可以直接复制到其他电脑使用
echo.

REM 打开输出目录
explorer %DEPLOY_DIR%

pause
