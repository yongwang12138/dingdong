#include "reminder.h"
#include "config.h"

#include <QDateTime>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QWinEventNotifier>
#ifndef CREATE_WAITABLE_TIMER_REALTIME
#define CREATE_WAITABLE_TIMER_REALTIME 0x2
#endif
#endif

ReminderManager::ReminderManager(AppConfig* config, QObject* parent)
    : QObject(parent)
    , m_config(config)
{
#ifdef Q_OS_WIN
    // 实时等待定时器：可把系统从现代待机(S0 空闲)/睡眠中唤醒，到点准点触发。
    // 若当前进程无相关特权导致创建失败，回退到普通等待定时器（仍可唤醒 S3 睡眠）。
    m_waitTimer = CreateWaitableTimerExW(nullptr, nullptr,
                                         CREATE_WAITABLE_TIMER_REALTIME,
                                         TIMER_ALL_ACCESS);
    if(!m_waitTimer)
        m_waitTimer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
    if(m_waitTimer)
    {
        m_notifier = new QWinEventNotifier(m_waitTimer, this);
        connect(m_notifier, &QWinEventNotifier::activated, this, [this]()
        {
            emit triggered(m_nextText);
            reschedule();
        });
        return;
    }
#endif
    // 最终回退：普通单次 QTimer（跨平台；Windows 上仅当等待定时器不可用时）
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]()
    {
        emit triggered(m_nextText);
        reschedule();
    });
}

ReminderManager::~ReminderManager()
{
#ifdef Q_OS_WIN
    if(m_waitTimer)
        CloseHandle(m_waitTimer);
#endif
}

QList<ReminderItem> ReminderManager::items() const
{
    return m_config->items();
}

bool ReminderManager::autoStart() const
{
    return m_config->autoStart();
}

void ReminderManager::start()
{
    reschedule();
}

void ReminderManager::saveItems(const QList<ReminderItem>& items)
{
    m_config->setItems(items);
    reschedule();
}

void ReminderManager::setAutoStart(bool on)
{
    m_config->setAutoStart(on);
}

int ReminderManager::stayMs() const
{
    return m_config->stayMs();
}

void ReminderManager::setStayMs(int ms)
{
    m_config->setStayMs(ms);
}

int ReminderManager::nextIndex() const
{
    const QList<ReminderItem> list = m_config->items();
    if(list.isEmpty())
        return -1;
    const NextTrigger next = calcNextTrigger(list);
    for(int i = 0; i < list.size(); ++i)
    {
        if(list[i].time == next.dt.time())
            return i;
    }
    return 0;
}

QString ReminderManager::nextText() const
{
    const QList<ReminderItem> list = m_config->items();
    const int idx = nextIndex();
    if(idx >= 0 && idx < list.size())
        return list[idx].text;
    return QString();
}

void ReminderManager::setItemText(int index, const QString& text)
{
    QList<ReminderItem> list = m_config->items();
    if(index >= 0 && index < list.size())
    {
        list[index].text = text;
        m_config->setItems(list);
        reschedule();
    }
}

// 根据配置的每日时刻列表，计算最近一次触发的 QDateTime 及其文本
NextTrigger ReminderManager::calcNextTrigger(const QList<ReminderItem>& items)
{
    const QDateTime now = QDateTime::currentDateTime();
    NextTrigger next;

    for(const auto& it : items)
    {
        QDateTime candidate{now.date(), it.time};
        if(candidate > now && (!next.dt.isValid() || candidate < next.dt))
        {
            next.dt = candidate;
            next.text = it.text;
        }
    }
    // 如果今天所有时间都过去了，取明天最早的一个
    if(!next.dt.isValid() && !items.isEmpty())
    {
        ReminderItem earliest = items.first();
        for(const auto& it : items)
        {
            if(it.time < earliest.time)
                earliest = it;
        }
        next.dt = QDateTime{now.date().addDays(1), earliest.time};
        next.text = earliest.text;
    }
    return next;
}

void ReminderManager::reschedule()
{
    const QList<ReminderItem> list = m_config->items();
    if(list.isEmpty())
    {
        qDebug() << "没有配置定时时间";
#ifdef Q_OS_WIN
        if(m_waitTimer) CancelWaitableTimer(m_waitTimer);
#else
        if(m_timer) m_timer->stop();
#endif
        return;
    }
    const NextTrigger next = calcNextTrigger(list);
    qDebug() << "下一次触发时刻：" << next.dt.toString("yyyy-MM-dd HH:mm")
             << "文本：" << next.text;
    m_nextDt = next.dt;
    m_nextText = next.text;

#ifdef Q_OS_WIN
    if(m_waitTimer)
    {
        // 目标时刻转换为 FILETIME 绝对时间（100ns，自 1601-01-01 UTC）。
        // SetWaitableTimer 对绝对时间要求传入负数 QuadPart。
        const qint64 msecs = next.dt.toUTC().toMSecsSinceEpoch();
        const ULONGLONG ft = (msecs + 11644473600000LL) * 10000ULL;
        LARGE_INTEGER li{};
        li.QuadPart = -(LONGLONG)ft;
        // fResume = FALSE：系统处于现代待机(S0 空闲)时仍准点触发，
        // 但若已进入 S3/S4 睡眠则不被唤醒，待下次唤醒时再补触发（不强制叫醒电脑）。
        SetWaitableTimer(m_waitTimer, &li, 0, nullptr, nullptr, FALSE);
    }
#else
    const qint64 msToNext = QDateTime::currentDateTime().msecsTo(next.dt);
    if(msToNext > 0 && m_timer)
        m_timer->start(msToNext);
#endif
}
