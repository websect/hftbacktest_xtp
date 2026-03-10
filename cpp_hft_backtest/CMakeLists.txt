cmake_minimum_required(VERSION 3.10)
project(ctp_hft_backtest VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/../docs/ctp/v3.7.5_CP_20251125_api_traderapi_linux64_se
)

# 源文件
set(CORE_SOURCES
    src/core/backtest_engine.cpp
    src/core/order_book.cpp
    src/core/position_manager.cpp
    src/strategy/strategy.cpp
)

# 创建核心库
add_library(ctp_hft_core STATIC ${CORE_SOURCES})

# 简单做市策略示例
add_executable(simple_market_making
    examples/simple_mm/main.cpp
)

target_link_libraries(simple_market_making
    ctp_hft_core
)

# 网格交易策略示例 (待实现)
# add_executable(grid_trading
#     examples/grid_trading/main.cpp
# )
#
# target_link_libraries(grid_trading
#     ctp_hft_core
# )

# 动量策略示例 (待实现)
# add_executable(momentum_strategy
#     examples/momentum/main.cpp
# )
#
# target_link_libraries(momentum_strategy
#     ctp_hft_core
# )

# 打印配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
