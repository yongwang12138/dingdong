#ifndef DINGDONGWIDGET_H
#define DINGDONGWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QImage>
#include <QPoint>

class QLabel;

class DingDongWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DingDongWidget(QWidget* parent = nullptr);

public slots:
    // text 为空则不显示气泡；reposition=false 时保持桌宠当前位置（用于双击预览）
    void showWidget(const QString& text = QString(), bool reposition = true);
    void setStayMs(int ms);

public:
    bool bubbleVisible() const;
    void showBubble(); // 用最近一次触发的提醒文本显示气泡，不重定位桌宠、不自动关闭
    void hideBubble(); // 只隐藏气泡，桌宠保持显示

signals:
    void doubleClicked();   // 双击桌宠，用于弹出气泡文本框

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onAnimationTick();
    void onAutoClose();

private:
    void loadFrames();
    void positionBubble();

    QTimer* m_animTimer{nullptr};
    QTimer* m_autoCloseTimer{nullptr}; //自动关闭计时器
    QVector<QImage> m_frameList{};
    int m_currentFrame{0};
    QPoint m_dragOffset{};

    // 说话气泡（独立的置顶无边框窗口，跟随桌宠位置）
    QWidget* m_bubble{nullptr};
    QLabel* m_bubbleLabel{nullptr};

    static constexpr int m_frameRate{10};
    static constexpr int m_widgetSize{220};
    int m_stayMs{8000}; // 弹窗停留毫秒数（可在设置中调整）
    QString m_triggerText{}; // 最近一次到点触发的提醒文本（供双击显示）
};

#endif // DINGDONGWIDGET_H
