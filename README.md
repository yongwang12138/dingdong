# 叮咚 DingDong

一个轻量级的 Windows 托盘定时提醒小工具，基于 Qt 6。程序常驻系统托盘，在设定的时间弹出动画提醒，支持配置多个每日提醒时间、开机自启。

## 功能特性

- 系统托盘常驻，关闭窗口不退出程序
- 支持设置多个每日提醒时间，跨天自动循环调度
- 首次运行默认添加每天 14:55 的提醒
- 提醒弹窗动画显示在屏幕右下角，停留 8 秒后自动关闭（可拖动、按 Esc 关闭）
- 支持开机自动启动
- 图标使用 `resources/app.ico`（用户提供的图标），经 `qt_add_resources` 编入 exe 资源，运行期从 `:/icons/resources/app.ico` 读取
- 配置以 JSON 文件存储于用户目录

## 构建

要求：Qt 6 + CMake 3.20+ 与 C++17 编译器（MSVC / MinGW）。

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt6 的 cmake 目录>
cmake --build build --config Release
```

构建产物输出到仓库根目录的 `bin/`，可执行文件名为 `dingdong.exe`
（打包前请对该目录执行 `windeployqt`）。

## 使用

启动后程序驻留托盘，右键托盘图标选择「设置」：

- **常规**：开机自启、立即触发预览
- **提醒时间**：增删每日提醒时刻（HH:mm）
- **关于**：版本与说明

## 配置

配置文件位于：

```
%APPDATA%/DingDong/DingDong/DingDong.json
```

格式示例：

```json
{
  "version": 1,
  "autostart": false,
  "reminders": [
    { "time": "14:55" }
  ]
}
```

## 代码结构

```
src/
├── main.cpp            # 程序入口、托盘与菜单
├── core/               # 与界面无关的逻辑
│   ├── config.*        # 配置读写（JSON）
│   └── reminder.*      # 提醒调度引擎
├── ui/                 # 界面
│   ├── dingdongwidget.*# 提醒弹窗动画
│   └── settingdialog.* # 设置对话框
└── platform/           # 平台相关（Windows）
    ├── appinit.*       # 日志/控制台/开机自启
    ├── appicon.*       # 应用图标（字体字形渲染）
    └── logger.*        # 日志重定向
resources/              # 图标、rc 文件、动画帧
packaging/              # Inno Setup 打包脚本
```

头文件与源文件同目录存放（本项目是可执行程序，无对外发布的公共 API，
因此不拆 `include/`），内部引用统一以 `src/` 为根，例如 `#include "core/config.h"`。

## 许可证

本工具仅供学习交流使用。
