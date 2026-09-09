#include "config.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

QString AppConfig::defaultPath()
{
    // 配置文件放在可执行程序所在目录的 config 子目录下
    const QString dir = QCoreApplication::applicationDirPath() + "/config";
    QDir().mkpath(dir);
    return dir + "/DingDong.json";
}

AppConfig::AppConfig(const QString& filePath)
    : m_filePath(filePath)
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
}

QList<ReminderItem> AppConfig::defaultItems()
{
    return {{QTime(14, 58), "到点了！到点了！看下支付宝上的基金！！！"}};
}

void AppConfig::ensureLoaded() const
{
    if(m_loaded)
        return;

    QFile file(m_filePath);
    if(!file.exists())
    {
        // 首次运行：写入默认配置（每天 14:58）
        m_items = defaultItems();
        m_autoStart = false;
        m_loaded = true;
        const_cast<AppConfig*>(this)->writeBack();
        return;
    }

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "无法打开配置文件：" << m_filePath;
        m_items = defaultItems();
        m_autoStart = false;
        m_loaded = true;
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if(!doc.isObject())
    {
        qWarning() << "配置文件格式错误，使用默认配置";
        m_items = defaultItems();
        m_autoStart = false;
        m_loaded = true;
        return;
    }

    const QJsonObject root = doc.object();
    m_autoStart = root.value("autostart").toBool(false);
    m_stayMs = root.value("stayMs").toInt(30000);
    const QJsonArray arr = root.value("reminders").toArray();
    m_items.clear();
    for(const QJsonValue& v : arr)
    {
        const QJsonObject obj = v.toObject();
        const QTime t = QTime::fromString(obj.value("time").toString(), "HH:mm");
        if(t.isValid())
            m_items.append({t, obj.value("text").toString()});
    }
    if(m_items.isEmpty())
        m_items = defaultItems();
    m_loaded = true;
}

QList<ReminderItem> AppConfig::items() const
{
    ensureLoaded();
    return m_items;
}

bool AppConfig::autoStart() const
{
    ensureLoaded();
    return m_autoStart;
}

void AppConfig::setItems(const QList<ReminderItem>& items)
{
    ensureLoaded();
    m_items = items;
    writeBack();
}

void AppConfig::setAutoStart(bool on)
{
    ensureLoaded();
    m_autoStart = on;
    writeBack();
}

int AppConfig::stayMs() const
{
    ensureLoaded();
    return m_stayMs;
}

void AppConfig::setStayMs(int ms)
{
    ensureLoaded();
    m_stayMs = ms;
    writeBack();
}

void AppConfig::writeBack()
{
    QJsonArray arr;
    for(const ReminderItem& it : std::as_const(m_items))
    {
        QJsonObject item;
        item["time"] = it.time.toString("HH:mm");
        item["text"] = it.text;
        arr.append(item);
    }
    QJsonObject root;
    root["version"] = 1;
    root["autostart"] = m_autoStart;
    root["stayMs"] = m_stayMs;
    root["reminders"] = arr;

    QFile file(m_filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "无法写入配置文件：" << m_filePath;
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}
