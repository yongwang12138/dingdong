#include "dingdongwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QVBoxLayout>

namespace {
// 带圆角、箭头与边框的气泡窗口（自绘，避免样式表在置顶透明窗口上不生效）
class BubbleFrame : public QWidget
{
public:
    explicit BubbleFrame(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    void setArrowRight(bool right)
    {
        if(m_arrowRight == right)
            return;
        m_arrowRight = right;
        update();
    }

    bool arrowRight() const { return m_arrowRight; }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        constexpr int radius = 12;
        constexpr int arrowSize = 14;   // 箭头占用的宽高
        constexpr int arrowHalf = 7;    // 箭头底边半长
        const int w = width();
        const int h = height();
        const int bodyBottom = h - arrowSize - 1;

        QPainterPath path;
        if(m_arrowRight)
        {
            // 箭头在右下角：气泡位于桌宠左上方
            const int bodyRight = w - arrowSize - 1;
            path.moveTo(radius, 1);
            path.lineTo(bodyRight - radius, 1);
            path.quadTo(bodyRight, 1, bodyRight, radius);
            path.lineTo(bodyRight, bodyBottom - arrowHalf);
            path.lineTo(w - 1, h - 1);                      // 箭头尖端
            path.lineTo(bodyRight - arrowHalf, bodyBottom);
            path.lineTo(radius, bodyBottom);
            path.quadTo(1, bodyBottom, 1, bodyBottom - radius);
            path.lineTo(1, radius);
            path.quadTo(1, 1, radius, 1);
        }
        else
        {
            // 箭头在左下角：气泡位于桌宠右上方
            const int bodyLeft = arrowSize + 1;
            path.moveTo(bodyLeft + radius, 1);
            path.lineTo(w - radius, 1);
            path.quadTo(w - 1, 1, w - 1, radius);
            path.lineTo(w - 1, bodyBottom - radius);
            path.quadTo(w - 1, bodyBottom, w - radius, bodyBottom);
            path.lineTo(bodyLeft + arrowHalf, bodyBottom);
            path.lineTo(1, h - 1);                          // 箭头尖端
            path.lineTo(bodyLeft, bodyBottom - arrowHalf);
            path.lineTo(bodyLeft, radius);
            path.quadTo(bodyLeft, 1, bodyLeft + radius, 1);
        }
        path.closeSubpath();

        p.fillPath(path, QColor("#fffbe6"));
        QPen pen(QColor("#e0c98a"));
        pen.setWidth(1);
        p.setPen(pen);
        p.drawPath(path);
    }

private:
    bool m_arrowRight{true};
};
}

DingDongWidget::DingDongWidget(QWidget* parent)
    : QWidget{parent}
    , m_animTimer{new QTimer{this}}
    , m_autoCloseTimer{new QTimer{this}}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(m_widgetSize, m_widgetSize);

    loadFrames();

    connect(m_animTimer, &QTimer::timeout, this, &DingDongWidget::onAnimationTick);
    m_animTimer->setInterval(1000 / m_frameRate);

    //弹窗停留时间，只触发一次
    m_autoCloseTimer->setSingleShot(true);
    m_autoCloseTimer->setInterval(m_stayMs);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &DingDongWidget::onAutoClose);

    // 说话气泡：独立置顶无边框窗口
    m_bubble = new BubbleFrame(nullptr);
    QVBoxLayout* bLay = new QVBoxLayout(m_bubble);
    // 右侧和底部多留空间给箭头，文字仍保持在圆角主体区域内
    bLay->setContentsMargins(14, 12, 24, 24);
    bLay->setSpacing(0);
    m_bubbleLabel = new QLabel;
    m_bubbleLabel->setWordWrap(true);
    m_bubbleLabel->setStyleSheet(
        "QLabel{color:#5a4a1f;font-size:14px;background:transparent;border:none;}");
    bLay->addWidget(m_bubbleLabel);
    m_bubble->setFixedWidth(240);
    m_bubble->hide();
}

void DingDongWidget::showWidget(const QString& text, bool reposition)
{
    if(reposition)
    {
        // 定时提醒时把桌宠放到主屏右下角（避开任务栏）
        const auto screenGeo = QApplication::primaryScreen()->availableGeometry();
        constexpr int margin = 24;
        move(screenGeo.right() - width() - margin,
             screenGeo.bottom() - height() - margin);
    }

    show();
    raise();
    activateWindow();

    m_currentFrame = 0;
    m_animTimer->start();
    m_autoCloseTimer->setInterval(m_stayMs);
    m_autoCloseTimer->start();

    // 到点只弹桌宠，文本气泡不自动显示，需双击桌宠才会出现
    m_triggerText = text;
    m_bubble->hide();
}

bool DingDongWidget::bubbleVisible() const
{
    return m_bubble && m_bubble->isVisible();
}

void DingDongWidget::showBubble()
{
    if(!isVisible())
    {
        show();
        m_currentFrame = 0;
        m_animTimer->start();
    }
    // 暂停自动关闭，气泡会一直显示到再次双击隐藏
    m_autoCloseTimer->stop();

    const QString text = m_triggerText.isEmpty() ? QString("（暂无提醒）") : m_triggerText;
    m_bubbleLabel->setText(text);
    positionBubble();
    m_bubble->show();
    m_bubble->raise();
}

void DingDongWidget::hideBubble()
{
    m_bubble->hide();
    // 隐藏气泡后恢复自动关闭计时（若本次是定时提醒触发的，到时间桌宠也会收起）
    m_autoCloseTimer->setInterval(m_stayMs);
    m_autoCloseTimer->start();
}

void DingDongWidget::positionBubble()
{
    const QRect pet = frameGeometry();
    // 根据桌宠当前中心点找到它所在的屏幕（支持多显示器扩展屏）
    QScreen* screen = QGuiApplication::screenAt(pet.center());
    if(!screen)
        screen = QApplication::primaryScreen();
    const QRect avail = screen->availableGeometry();

    constexpr int kArrowGap = 22;
    const QPoint petHead(pet.x() + pet.width() / 2, pet.y() + pet.height() / 4);

    auto* bubble = static_cast<BubbleFrame*>(m_bubble);
    auto* lay = static_cast<QVBoxLayout*>(m_bubble->layout());

    // 先尝试箭头在右下角（气泡在桌宠左上方）
    bubble->setArrowRight(true);
    lay->setContentsMargins(14, 12, 24, 24);
    m_bubble->adjustSize();
    int bx = petHead.x() - m_bubble->width() + 1 - kArrowGap;

    // 如果气泡左边贴墙/出界，就换成箭头在左下角（气泡在桌宠右上方）
    if(bx < avail.left())
    {
        bubble->setArrowRight(false);
        lay->setContentsMargins(24, 12, 14, 24);
        m_bubble->adjustSize();
        bx = petHead.x() + 1 + kArrowGap;
    }

    int by = petHead.y() - m_bubble->height() + 1 - kArrowGap;
    bx = qBound(avail.left(), bx, avail.right() - m_bubble->width());
    by = qBound(avail.top(), by, avail.bottom() - m_bubble->height());
    m_bubble->move(bx, by);
}

void DingDongWidget::setStayMs(int ms)
{
    if(ms > 0)
        m_stayMs = ms;
}

void DingDongWidget::onAutoClose()
{
    m_animTimer->stop();
    m_autoCloseTimer->stop();
    hide();
    m_bubble->hide();
}

void DingDongWidget::loadFrames()
{
    const int frameCount = 41;
    for(int i = 1; i <= frameCount; ++i)
    {
        // 与根 CMakeLists.txt 中 qt_add_resources 的 PREFIX + 相对路径别名保持一致
        QString resPath = QString(":/anim/resources/anim/idle/%1.png").arg(i);
        QImage img{resPath};
        if(!img.isNull())
        {
            m_frameList.push_back(std::move(img));
        }
        else
        {
            qDebug() << "加载失败:" << resPath;
        }
    }
    qDebug() << "loaded frames:" << m_frameList.size();
}

void DingDongWidget::onAnimationTick()
{
    if(m_frameList.empty())
        return;
    m_currentFrame = (m_currentFrame + 1) % m_frameList.size();
    update();
}

void DingDongWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter{this};
    if(!m_frameList.empty())
    {
        painter.drawImage(rect(), m_frameList[m_currentFrame]);
    }
}

void DingDongWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void DingDongWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        move(event->globalPosition().toPoint() - m_dragOffset);
        if(m_bubble->isVisible())
            positionBubble();
    }
    QWidget::mouseMoveEvent(event);
}

void DingDongWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}

void DingDongWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape)
    {
        m_animTimer->stop();
        m_autoCloseTimer->stop();
        hide();
        m_bubble->hide();
    }
    QWidget::keyPressEvent(event);
}
