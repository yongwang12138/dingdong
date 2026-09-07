#ifndef APPINIT_H
#define APPINIT_H

#include <QString>

// 启动准备工作（都在启动黑窗口显示期间完成）：
// - 日志文件初始化
// - 准备完毕后关闭黑窗口
// - 开机自启注册表项
class AppInit
{
public:
    // 初始化日志，返回日志文件路径
    static QString initLogging();
    // 启动准备完成、托盘提示弹出后关闭黑窗口（延迟 delayMs 毫秒执行）
    static void scheduleCloseConsole(int delayMs = 2000);
    // 程序退出前收尾（关闭日志文件）
    static void shutdown();
    // 设置/取消开机自启（Windows 写 HKEY_CURRENT_USER，不需要管理员权限）
    static bool setAutoStart(bool enable);
};

#endif // APPINIT_H
