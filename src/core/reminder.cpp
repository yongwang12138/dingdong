#include "reminder.h"
#include "config.h"

#include <QDateTime>
#include <QDebug>

// 单次挂钟核对的最大间隔：既避免“几小时长的 QTimer 在系统空闲时被节流导致明显漂移”，
// 又把活跃状态下的误差限制在 cap 以内。非忙等轮询（每 cap 才唤醒核对一次墙钟）。
static constexpr qint64 kMaxCheckMs = 30 * 1000; // 30 秒

ReminderManager::ReminderManager(AppConfig* config, QObject* parent)
    : QObject(parent)
    , m_config(config)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ReminderManager::onTimeout);
}

ReminderManager::~ReminderManager() = default;

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

void ReminderManager::onTimeout()
{
    const QDateTime now = QDateTime::currentDateTime();
    // 已到达目标时刻则触发；否则只是周期性核对（尚未到点），重新排下一个间隔
    if(!m_nextDt.isValid() || now >= m_nextDt)
    {
        emit triggered(m_nextText);
        reschedule(); // 触发后安排下一次
    }
    else
    {
        reschedule(); // 继续挂一个不超过上限的间隔
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
        m_timer->stop();
        return;
    }
    const NextTrigger next = calcNextTrigger(list);
    qDebug() << "下一次触发时刻：" << next.dt.toString("yyyy-MM-dd HH:mm")
             << "文本：" << next.text;
    m_nextDt = next.dt;
    m_nextText = next.text;

    const qint64 msToNext = QDateTime::currentDateTime().msecsTo(next.dt);
    if(msToNext > 0)
    {
        // 限幅：临近目标时自然精确到秒；还差很远时每 kMaxCheckMs 核对一次
        m_timer->start(qMin(msToNext, kMaxCheckMs));
    }
}
