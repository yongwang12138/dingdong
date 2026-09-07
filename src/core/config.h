#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QList>
#include <QTime>
#include <QString>

// 单条提醒：时刻 + 触发时展示的文本
struct ReminderItem
{
    QTime time;
    QString text;
};

// 配置持久化：把提醒时间/文本与开机自启开关存为 JSON 文件。
// 数据常驻内存（QList<ReminderItem>），仅在启动时读一次、变更时写一次，
// 运行期零解析开销，内存占用最低。
class AppConfig
{
public:
    explicit AppConfig(const QString& filePath = defaultPath());

    QList<ReminderItem> items() const;
    bool autoStart() const;
    int stayMs() const;

    void setItems(const QList<ReminderItem>& items);
    void setAutoStart(bool on);
    void setStayMs(int ms);

    // 首次运行默认写入的提醒（每天 14:55，带默认文本）
    static QList<ReminderItem> defaultItems();

private:
    static QString defaultPath();
    void ensureLoaded() const;
    void writeBack();

    QString m_filePath;
    mutable bool m_loaded{false};
    mutable QList<ReminderItem> m_items;
    mutable bool m_autoStart{false};
    mutable int m_stayMs{30000}; // 弹窗停留毫秒数，默认 30 秒
};

#endif // APPCONFIG_H
