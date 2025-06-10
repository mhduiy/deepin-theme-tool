# Deepin 主题工具

一个专为 Deepin 桌面环境设计的图标主题转换工具，支持将标准的 FreeDesktop.org 图标主题包转换为 Deepin 兼容的 DEB 安装包格式。

## 功能特性

### 🎨 主题转换工具
- **多源支持**: 支持选择主题包目录或 DEB 包文件作为源
- **批量处理**: 支持同时选择多个主题包目录或 DEB 包进行转换
- **标准规范支持**: 完全支持 FreeDesktop.org 图标主题规范
- **自动解析**: 智能解析 `index.theme` 配置文件和 DEB 包内容
- **DEB 输出**: 直接生成可安装的 DEB 包格式
- **实时反馈**: 提供详细的转换进度和日志信息

### 🛠️ DCI 工具集
- **DCI 解析**: 树形解析 DCI 图标文件
- **格式转换**: 支持 SVG、PNG 等格式转换为 DCI
- **批量处理**: 支持目录级别的批量转换

## 界面截图

![主界面](docs/screenshot1.png)
![转换界面](docs/screenshot2.png)

## 系统要求

- **操作系统**: Deepin 20+ 或其他基于 Debian 的 Linux 发行版
- **Qt 版本**: Qt 6.0+
- **DTK 版本**: DTK 6.0+
- **编译工具**: GCC 8+ 或 Clang 10+

## 编译构建

### 安装依赖

```bash
# Deepin/UOS 系统
sudo apt install qtbase6-dev libdtk6widget-dev cmake make g++

# Ubuntu/Debian 系统  
sudo apt install qt6-base-dev libdtk6widget-dev cmake make g++
```

### 编译步骤

```bash
# 克隆代码
git clone https://github.com/mhduiy/deepin-theme-tool.git
cd deepin-theme-tool

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 运行程序
./deepin-theme-tool
```

## 使用指南

### 主题转换

1. **选择源类型**: 选择"主题包目录"或"DEB包文件"作为源类型
2. **添加源项目**: 
   - 对于主题包目录：选择包含 `index.theme` 文件的目录
   - 对于DEB包：选择现有的主题DEB包文件
   - 支持添加多个源项目进行批量转换
3. **管理源列表**: 可以移除选中的项目或清空整个列表
4. **设置输出**: 选择输出 DEB 包的保存路径和文件名
5. **自定义主题名**: 可选择自定义转换后的主题名称
6. **开始转换**: 点击"开始转换"按钮开始转换过程
7. **查看日志**: 在底部日志区域查看详细的转换进度

### 支持的源格式

#### 输入格式
- **主题包目录**: 标准 FreeDesktop.org 图标主题包
  - 必须包含 `index.theme` 文件
  - 支持 PNG、SVG、XPM 格式图标
  - 支持多尺寸图标目录结构
  
- **DEB包文件**: 现有的主题DEB包
  - 支持解析现有DEB包内容
  - 可以重新打包和优化
  - 支持批量处理多个DEB包

#### 输出格式
- **DEB安装包**: Deepin 兼容的安装包格式
  - 符合 Debian 包规范
  - 包含完整的依赖信息
  - 支持通过包管理器安装

### 批量处理功能

- **多源合并**: 可以将多个主题包目录合并为一个DEB包
- **批量转换**: 同时处理多个DEB包进行格式优化
- **智能命名**: 自动根据源内容生成合适的主题名称
- **冲突处理**: 自动处理多个源之间的图标冲突

## 项目结构

```
deepin-theme-tool/
├── src/                    # 源代码目录
│   ├── main.cpp           # 程序入口
│   ├── mainwindow.*       # 主窗口
│   ├── themetool.*        # 主题转换工具
│   ├── dcitoolsview.*     # DCI 工具界面
│   └── funcitem.*         # 功能项组件
├── build/                  # 构建目录
├── docs/                   # 文档目录
├── CMakeLists.txt         # CMake 配置
└── README.md              # 本文件
```

## 技术细节

### 转换流程

1. **源验证**: 验证所有源项目的有效性和格式
2. **内容解析**: 
   - 主题包目录：解析 `index.theme` 配置文件
   - DEB包：提取并分析包内容结构
3. **资源整合**: 合并多个源的图标资源
4. **格式转换**: 根据 Deepin 标准调整图标格式
5. **依赖处理**: 生成正确的包依赖关系
6. **DEB打包**: 创建符合规范的DEB安装包

### 支持的图标规范

- **FreeDesktop.org Icon Theme Specification v0.13**
- **支持的上下文**: Actions, Applications, Devices, FileSystems, MimeTypes
- **支持的类型**: Fixed, Scalable, Threshold
- **支持的缩放**: 1x, 2x, 3x 等高DPI缩放

### DEB包特性

- **完整的包信息**: 包含版本、描述、依赖等完整元数据
- **安装脚本**: 自动生成安装和卸载脚本
- **文件权限**: 正确设置图标文件的权限
- **系统集成**: 与 Deepin 桌面环境无缝集成

## 贡献指南

欢迎提交 Issue 和 Pull Request！

### 开发环境

1. Fork 本仓库
2. 创建功能分支: `git checkout -b feature/amazing-feature`
3. 提交更改: `git commit -m 'Add amazing feature'`
4. 推送分支: `git push origin feature/amazing-feature`
5. 创建 Pull Request

### 代码规范

- 遵循 Qt 代码风格
- 使用 4 空格缩进
- 函数和变量使用驼峰命名法
- 类成员变量以 `m_` 前缀开头

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 相关链接

- [FreeDesktop.org 图标主题规范](https://specifications.freedesktop.org/icon-theme-spec/latest/)
- [Debian 包规范](https://www.debian.org/doc/debian-policy/)
- [Deepin 开发者文档](https://github.com/linuxdeepin/developer-center)
- [DTK 开发指南](https://github.com/linuxdeepin/dtk6widget)

## 支持

如果你在使用过程中遇到问题，可以通过以下方式获取帮助：

- 提交 [GitHub Issue](https://github.com/mhduiy/deepin-theme-tool/issues)
- 加入 Deepin 开发者社群
- 发送邮件至开发者

---

**注意**: 本工具仍在积极开发中，某些功能可能还不够完善。欢迎反馈和建议！ 