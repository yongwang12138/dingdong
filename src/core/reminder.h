#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QDateTime>
#include "core/config.h"

// 下次触发结果：时刻 + 对应文本
struct NextTrigger
{
    QDateTime dt;
    QString text;
};

class ReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit ReminderManager(AppConfig* config, QObject* parent = nullptr);

    QList<ReminderItem> items() const;
    bool autoStart() const;
    int stayMs() const;

    // 下一个将要触发的提醒索引与文本（供气泡编辑框使用）
    int nextIndex() const;
    QString nextText() const;
    void setItemText(int index, const QString& text);

public slots:
    void start();
    void saveItems(const QList<ReminderItem>& items);
    void setAutoStart(bool on);
    void setStayMs(int ms);

signals:
    void triggered(const QString& text);

private:
    void reschedule();
    static NextTrigger calcNextTrigger(const QList<ReminderItem>& items);

    AppConfig* m_config{nullptr};
    QTimer* m_timer{nullptr};
    QString m_nextText;
};

#endif // REMINDERMANAGER_H
