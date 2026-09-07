#include "logger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <cstdio>

QFile Logger::s_logFile;
bool Logger::s_consoleAlive = true;

QString Logger::init()
{
    // 日志文件放在可执行程序所在目录的 log 子目录下
    const QString dir = QCoreApplication::applicationDirPath() + "/log";
    QDir().mkpath(dir);
    const QString path = QString("%1/dingdong_%2.log")
        .arg(dir, QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    s_logFile.setFileName(path);
    if(!s_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "无法创建日志文件：" << path;
        return QString();
    }

    qInstallMessageHandler(messageHandler);
    return path;
}

void Logger::shutdown()
{
    s_logFile.close();
}

void Logger::setConsoleClosed(bool closed)
{
    s_consoleAlive = !closed;
}

QString Logger::filePath()
{
    return s_logFile.fileName();
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    QString level = QStringLiteral("Debug");
    switch(type)
    {
    case QtDebugMsg:    level = QStringLiteral("Debug");   break;
    case QtInfoMsg:     level = QStringLiteral("Info");    break;
    case QtWarningMsg:  level = QStringLiteral("Warning"); break;
    case QtCriticalMsg: level = QStringLiteral("Critical");break;
    case QtFatalMsg:    level = QStringLiteral("Fatal");   break;
    }

    const QByteArray line = QString("[%1] [%2] %3\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"), level, msg)
        .toLocal8Bit();

    if(s_logFile.isOpen())
    {
        s_logFile.write(line);
        s_logFile.flush();
    }

    if(s_consoleAlive)
    {
        std::fwrite(line.constData(), 1, static_cast<size_t>(line.size()), stderr);
        std::fflush(stderr);
    }
}
