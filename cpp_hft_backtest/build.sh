#!/bin/bash

# CTP 回测系统编译脚本

echo "============================================================"
echo "CTP 回测系统编译脚本"
echo "hftbacktest_xtp - C++版本"
echo "============================================================"
echo ""

# 设置路径
PROJECT_DIR="/home/lihaibin/2026_ai/hftbacktest/cpp_hft_backtest"
BUILD_DIR="$PROJECT_DIR/build"

# 创建构建目录
mkdir -p "$BUILD_DIR"

echo "编译配置:"
echo "  项目目录: $PROJECT_DIR"
echo "  构建目录: $BUILD_DIR"
echo ""

# 进入构建目录
cd "$BUILD_DIR"

# 运行 CMake
echo "正在运行 CMake..."
cmake .. || {
    echo ""
    echo "============================================================"
    echo "✗ CMake 配置失败！"
    echo "============================================================"
    exit 1
}

echo ""
echo "正在编译..."
make || {
    echo ""
    echo "============================================================"
    echo "✗ 编译失败！"
    echo "============================================================"
    exit 1
}

echo ""
echo "============================================================"
echo "✓ 编译成功！"
echo "============================================================"
echo ""
echo "可执行文件位于: $BUILD_DIR/bin/"
echo ""
echo "运行示例:"
echo "  cd $BUILD_DIR/bin"
echo "  ./simple_market_making    # 简单做市策略"
echo "  ./grid_trading            # 网格交易策略"
echo "  ./momentum_strategy       # 动量策略"
echo ""
