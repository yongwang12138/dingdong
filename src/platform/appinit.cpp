#include "appinit.h"
#include "logger.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QString AppInit::initLogging()
{
    return Logger::init();
}

void AppInit::scheduleCloseConsole(int delayMs)
{
    QTimer::singleShot(delayMs, qApp, []()
    {
#ifdef Q_OS_WIN
        // 之后日志只写入文件，不再输出到已关闭的控制台
        Logger::setConsoleClosed(true);
        FreeConsole();
#endif
    });
}

void AppInit::shutdown()
{
    Logger::shutdown();
}

bool AppInit::setAutoStart(bool enable)
{
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    const QString appName = QApplication::applicationName();
    const QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
    if(enable)
        reg.setValue(appName, appPath);
    else
        reg.remove(appName);
    return reg.status() == QSettings::NoError;
}
