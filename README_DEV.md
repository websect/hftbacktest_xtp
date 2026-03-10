# hftbacktest 二次开发指南

## 项目初始化完成

本项目已成功克隆并配置为支持二次开发，同时保持与上游仓库的同步。

## 当前配置

- **Origin**: git@github.com:websect/hftbacktest_xtp.git (你的工作仓库)
- **Upstream**: https://github.com/nkaz001/hftbacktest.git (上游原始仓库)

## 目录结构

```
hftbacktest/
├── hftbacktest/             # 原始核心代码
├── hftbacktest-derive/      # 原始派生代码
├── py-hftbacktest/          # Python 绑定
├── collector/               # 数据收集器
├── connector/               # 连接器
├── examples/                # 示例代码
├── docs/                    # 原始文档
├── my_extensions/           # 你的扩展代码
├── my_tests/                # 你的测试代码
└── my_docs/                 # 你的文档
```

## 开发工作流

### 1. 创建功能分支

```bash
cd hftbacktest
git checkout -b feature/你的功能名称
```

### 2. 开发并提交

```bash
# 在 my_extensions/ 或其他目录中开发
git add .
git commit -m "描述你的更改"
```

### 3. 推送到远程

```bash
git push origin feature/你的功能名称
```

## 同步上游更新

### 定期同步（建议每周）

```bash
# 1. 切换到主分支
git checkout main

# 2. 获取上游更新
git fetch upstream

# 3. 合并上游更新
git merge upstream/main

# 4. 推送更新到你的 fork
git push origin main
```

### 将上游更新合并到功能分支

```bash
# 1. 切换到功能分支
git checkout feature/你的功能名称

# 2. 合并主分支
git merge main

# 3. 解决冲突（如果有）
# 4. 继续开发
```

## 向上游贡献代码

如果你希望将改进贡献回原项目：

```bash
# 1. 确保功能分支最新
git checkout feature/你的功能名称
git merge main

# 2. 推送到你的 fork
git push origin feature/你的功能名称

# 3. 在 GitHub 上发起 Pull Request 到 nkaz001/hftbacktest
```

## 注意事项

- ⚠️ 建议在 `my_extensions/` 中添加新功能，而不是直接修改原始代码
- ⚠️ 定期同步上游更新，避免累积过多冲突
- ⚠️ 保持主分支干净，只在主分支上合并上游更新
- ⚠️ 每个新功能使用独立的分支

## 快捷命令参考

```bash
# 查看远程仓库
git remote -v

# 查看分支
git branch -a

# 查看状态
git status

# 查看差异
git diff
```

## 项目信息

- 原始仓库: https://github.com/nkaz001/hftbacktest
- 语言: Rust + Python
- 用途: 高频交易回测框架
