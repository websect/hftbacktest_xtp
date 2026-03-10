# hftbacktest 代码同步与提交操作指南

本文档详细说明如何从上游仓库同步最新代码，以及如何将自己的代码提交到个人仓库。

## 仓库配置说明

- **个人仓库 (origin)**: `git@github.com:websect/hftbacktest_xtp.git`
  - 用于提交和存储你的二次开发代码
- **上游仓库 (upstream)**: `https://github.com/nkaz001/hftbacktest.git`
  - 原始项目的官方仓库，用于获取最新更新

## 场景一：同步上游最新代码

### 1.1 定期同步（推荐每周执行）

当你需要获取上游仓库的最新更新时，执行以下步骤：

```bash
# 1. 确保在主分支上
cd hftbacktest
git checkout master

# 2. 获取上游仓库的最新更新
git fetch upstream

# 3. 查看上游更新内容（可选）
git log upstream/master --oneline -10

# 4. 将上游更新合并到本地主分支
git merge upstream/master

# 5. 推送更新到你的个人仓库
git push origin master
```

### 1.2 同步时的冲突处理

如果在合并过程中出现冲突：

```bash
# 1. 查看冲突文件
git status

# 2. 手动解决冲突文件中的冲突
# 编辑冲突文件，选择保留的内容，删除冲突标记 <<<<<<< 和 >>>>>>>

# 3. 标记冲突已解决
git add <冲突文件名>

# 4. 完成合并
git commit

# 5. 推送到你的仓库
git push origin master
```

## 场景二：将上游更新合并到功能分支

如果你正在开发功能分支，需要将上游更新合并进去：

```bash
# 1. 先同步主分支（参考场景一）
git checkout master
git fetch upstream
git merge upstream/master
git push origin master

# 2. 切换到你的功能分支
git checkout feature/你的功能名称

# 3. 将主分支（已包含上游更新）合并到功能分支
git merge master

# 4. 解决冲突（如果有）
# 手动解决冲突后：
git add <冲突文件名>
git commit

# 5. 继续开发或推送
git push origin feature/你的功能名称
```

## 场景三：提交自己的代码到个人仓库

### 3.1 提交新功能或修改

```bash
# 1. 确保在功能分支上
git checkout feature/你的功能名称

# 2. 如果是新功能，先同步主分支（包含上游最新代码）
git fetch upstream
git merge upstream/master

# 3. 查看当前修改
git status
git diff

# 4. 添加修改的文件
git add <文件名>
# 或者添加所有修改
git add .

# 5. 提交代码
git commit -m "描述你的修改内容"

# 6. 推送到个人仓库
git push origin feature/你的功能名称
```

### 3.2 提交规范建议

建议使用清晰的提交信息格式：

```bash
# 功能添加
git commit -m "feat: 添加XTP连接器支持"

# 问题修复
git commit -m "fix: 修复订单撮合逻辑错误"

# 性能优化
git commit -m "perf: 优化数据处理速度"

# 文档更新
git commit -m "docs: 更新API文档"

# 重构代码
git commit -m "refactor: 重构订单管理模块"
```

## 场景四：完整的开发流程示例

以下是一个完整的开发到提交流程：

```bash
# 1. 同步上游最新代码
cd hftbacktest
git checkout master
git fetch upstream
git merge upstream/master
git push origin master

# 2. 创建新的功能分支
git checkout -b feature/xtp-connector

# 3. 开发代码（在 my_extensions/ 或其他目录中）
# ... 进行代码开发 ...

# 4. 查看修改
git status
git diff

# 5. 提交代码
git add .
git commit -m "feat: 添加XTP连接器基础功能"

# 6. 推送到个人仓库
git push origin feature/xtp-connector

# 7. 如果需要继续开发，可以继续提交
# ... 更多开发 ...
git add .
git commit -m "feat: 完善XTP订单处理逻辑"
git push origin feature/xtp-connector

# 8. 开发完成后，可以合并到主分支
git checkout master
git merge feature/xtp-connector
git push origin master
```

## 场景五：向上游仓库贡献代码

如果你希望将自己的改进贡献回原项目：

```bash
# 1. 确保功能分支是最新的
git checkout feature/你的功能名称
git fetch upstream
git merge upstream/master

# 2. 解决任何冲突
# ... 解决冲突 ...

# 3. 推送到你的个人仓库
git push origin feature/你的功能名称

# 4. 在 GitHub 上操作：
#    - 访问 https://github.com/websect/hftbacktest_xtp
#    - 切换到你的功能分支
#    - 点击 "New Pull Request"
#    - 选择目标仓库：nkaz001/hftbacktest
#    - 填写 PR 描述并提交
```

## 常用命令速查表

### 远程仓库相关

```bash
# 查看远程仓库配置
git remote -v

# 查看远程分支
git branch -r

# 查看所有分支
git branch -a
```

### 同步相关

```bash
# 获取上游更新
git fetch upstream

# 获取 origin 更新
git fetch origin

# 查看上游与本地差异
git diff master upstream/master

# 查看远程分支日志
git log upstream/master --oneline -10
```

### 分支相关

```bash
# 创建新分支
git checkout -b feature/分支名

# 切换分支
git checkout 分支名

# 查看当前分支
git branch

# 删除本地分支
git branch -d 分支名

# 删除远程分支
git push origin --delete 分支名
```

### 提交相关

```bash
# 查看状态
git status

# 查看差异
git diff
git diff 文件名

# 添加文件
git add 文件名
git add .

# 提交
git commit -m "提交信息"

# 修改最后一次提交
git commit --amend

# 推送
git push origin 分支名
git push -u origin 分支名  # 首次推送并设置跟踪
```

## 最佳实践建议

### 1. 定期同步
- **建议频率**：每周至少同步一次上游更新
- **最佳时机**：开始新功能开发前、重要里程碑后

### 2. 分支管理
- **主分支 (master)**：保持干净，只合并上游更新和稳定功能
- **功能分支**：每个功能使用独立分支，命名清晰
- **命名规范**：`feature/功能描述`、`fix/问题描述`

### 3. 提交规范
- **原子提交**：每次提交只包含一个逻辑完整的修改
- **清晰信息**：提交信息描述清楚修改的目的和内容
- **频繁提交**：小步快跑，频繁提交便于版本管理

### 4. 冲突预防
- **定期同步**：减少与上游的代码差异
- **关注上游**：关注上游的 ROADMAP 和更新日志
- **合理扩展**：优先在 `my_extensions/` 中添加代码，减少对原始代码的直接修改

### 5. 备份重要代码
```bash
# 在进行重大操作前，创建备份分支
git branch backup-$(date +%Y%m%d)
```

## 故障排除

### 推送失败 - 认证问题

```bash
# 如果遇到认证问题，检查 SSH 配置
ssh -T git@github.com

# 或者使用 HTTPS 方式（需要配置凭据）
git remote set-url origin https://github.com/websect/hftbacktest_xtp.git
```

### 合并冲突

```bash
# 放弃合并，恢复到合并前状态
git merge --abort

# 或者使用 rebase 方式
git rebase upstream/master
```

### 查看历史

```bash
# 查看提交历史
git log --oneline --graph --all

# 查看特定文件历史
git log -- 文件名

# 查看分支图
git log --graph --decorate --oneline --all
```

## 项目结构提醒

```
hftbacktest/
├── hftbacktest/             # 原始核心代码（尽量避免直接修改）
├── my_extensions/           # 你的扩展代码（推荐在此开发）
├── my_tests/                # 你的测试代码
├── my_docs/                 # 你的文档
├── README_DEV.md            # 本开发指南
└── SYNC_WORKFLOW.md         # 本同步操作指南
```

## 联系与支持

- **上游仓库**: https://github.com/nkaz001/hftbacktest
- **个人仓库**: https://github.com/websect/hftbacktest_xtp
- **问题反馈**: 在个人仓库提交 Issue

---

**最后更新**: 2026-03-10
**维护者**: websect
