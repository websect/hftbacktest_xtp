#!/bin/bash

# CTP 登录测试编译脚本

echo "============================================================"
echo "CTP 登录测试编译脚本"
echo "hftbacktest_xtp - CTP 集成测试"
echo "============================================================"

# 设置路径
PROJECT_DIR="/home/lihaibin/2026_ai/hftbacktest"
CTP_LIB_DIR="$PROJECT_DIR/docs/ctp/v3.7.5_CP_20251125_api_traderapi_linux64_se"
SOURCE_DIR="$PROJECT_DIR/my_extensions"
BUILD_DIR="$PROJECT_DIR/my_extensions/build"

# 创建构建目录
mkdir -p "$BUILD_DIR"

echo ""
echo "编译配置:"
echo "  CTP 库路径: $CTP_LIB_DIR"
echo "  源文件: $SOURCE_DIR/test_ctp_login.cpp"
echo "  输出文件: $BUILD_DIR/test_ctp_login"
echo ""

# 编译命令
g++ -std=c++11 \
    -I"$CTP_LIB_DIR" \
    -L"$CTP_LIB_DIR" \
    -o "$BUILD_DIR/test_ctp_login" \
    "$SOURCE_DIR/test_ctp_login.cpp" \
    "$CTP_LIB_DIR/soptthosttraderapi_se.so" \
    -Wl,-rpath,"$CTP_LIB_DIR"

# 检查编译结果
if [ $? -eq 0 ]; then
    echo ""
    echo "============================================================"
    echo "✓ 编译成功！"
    echo "============================================================"
    echo ""
    echo "可执行文件: $BUILD_DIR/test_ctp_login"
    echo ""
    echo "运行测试:"
    echo "  cd $BUILD_DIR"
    echo "  ./test_ctp_login"
    echo ""
else
    echo ""
    echo "============================================================"
    echo "✗ 编译失败！"
    echo "============================================================"
    exit 1
fi
