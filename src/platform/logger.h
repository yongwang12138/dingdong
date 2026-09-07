#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>

// 日志重定向：
// - 输出到可执行程序目录下的 log/ 子目录
// - 每次启动生成一个新的日志文件（按启动时间命名）
// - 启动黑窗口存活期间同时输出到控制台，黑窗口关闭后只写文件
class Logger
{
public:
    // 创建日志文件并安装 Qt 消息处理器，返回日志文件路径（失败返回空串）
    static QString init();
    // 关闭日志文件（程序退出前调用）
    static void shutdown();
    // 标记控制台已关闭，之后日志只写文件不再输出到控制台
    static void setConsoleClosed(bool closed);
    // 当前日志文件路径
    static QString filePath();

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);

    static QFile s_logFile;
    static bool s_consoleAlive;
};

#endif // LOGGER_H
