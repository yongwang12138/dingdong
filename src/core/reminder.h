#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QDateTime>
#include "core/config.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <QWinEventNotifier>
#endif

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
    ~ReminderManager() override;

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
    QTimer* m_timer{nullptr};      // 等待定时器不可用时的回退（所有平台均声明）
#ifdef Q_OS_WIN
    HANDLE m_waitTimer{nullptr};   // 系统级实时等待定时器（现代待机准点，不唤醒睡眠）
    QWinEventNotifier* m_notifier{nullptr};
#endif
    QDateTime m_nextDt;            // 当前排定的触发时刻（留作迟到保护等扩展）
    QString m_nextText;
};

#endif // REMINDERMANAGER_H
