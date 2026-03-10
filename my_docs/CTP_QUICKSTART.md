# CTP 连接器快速开始

这是一个快速入门指南，帮助你快速上手 CTP 连接器。

## 5分钟快速开始

### 1. 创建示例数据

```bash
cd /home/lihaibin/2026_ai/hftbacktest
python my_extensions/ctp_data_converter.py
```

### 2. 运行回测示例

```bash
python my_extensions/example_ctp.py
```

### 3. 查看结果

回测完成后，你将看到策略执行的结果和统计信息。

## 下一步

- 阅读 [CTP_INTEGRATION_GUIDE.md](CTP_INTEGRATION_GUIDE.md) 了解详细信息
- 查看 [example_ctp.py](../my_extensions/example_ctp.py) 学习策略编写
- 研究 [ctp_data_converter.py](../my_extensions/ctp_data_converter.py) 了解数据处理

## 常用命令

```bash
# 创建测试数据
python my_extensions/ctp_data_converter.py

# 运行回测
python my_extensions/example_ctp.py

# 构建 CTP 连接器
cd connector && cargo build --release

# 启动连接器
./target/release/connector --name ctp --connector ctp --config ctp_config.toml
```

## 文件结构

```
hftbacktest/
├── connector/
│   └── src/
│       └── ctp/              # CTP 连接器实现
│           ├── mod.rs
│           ├── market_data_stream.rs
│           ├── order_manager.rs
│           ├── rest.rs
│           └── user_data_stream.rs
├── my_extensions/
│   ├── example_ctp.py        # CTP 使用示例
│   └── ctp_data_converter.py # 数据转换工具
├── my_docs/
│   ├── CTP_INTEGRATION_GUIDE.md  # 详细集成指南
│   └── CTP_QUICKSTART.md         # 本文件
└── data/                       # 数据目录
    └── ctp_*.npz              # CTP 格式数据
```

## 注意事项

⚠️ **重要提醒**:
1. 实盘交易前务必充分测试
2. 做好风险控制
3. 遵守交易所规则
4. 保护账号安全

## 获取帮助

- 📖 查看文档: [CTP_INTEGRATION_GUIDE.md](CTP_INTEGRATION_GUIDE.md)
- 🐛 提交问题: https://github.com/websect/hftbacktest_xtp/issues
- 💬 技术讨论: https://github.com/websect/hftbacktest_xtp/discussions

---

祝你使用愉快！🚀
