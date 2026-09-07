#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCursor>
#include <QScreen>
#include <QDialog>
#include <QDebug>

#include "platform/appinit.h"
#include "platform/appicon.h"
#include "ui/dingdongwidget.h"
#include "ui/settingdialog.h"
#include "core/config.h"
#include "core/reminder.h"

namespace {

// 右键托盘图标时，菜单从图标右上方弹出
void popupTrayMenu(QMenu& menu, QSystemTrayIcon& trayIcon)
{
    const QPoint cursor = QCursor::pos();
    const QRect anchor = trayIcon.geometry();
    const bool iconGeomValid = !anchor.isNull() && anchor.width() <= 80 && anchor.height() <= 80;
    const QPoint iconCenter = iconGeomValid ? anchor.center() : cursor;
    const int halfIcon = iconGeomValid ? anchor.width() / 2 : 20;

    QScreen* screen = QGuiApplication::screenAt(cursor);
    if(!screen)
        screen = QGuiApplication::primaryScreen();

    const QRect avail = screen->availableGeometry();
    const QSize hint = menu.sizeHint();
    constexpr int kOverlap = 8;
    const int x = qBound(avail.left(), iconCenter.x() + halfIcon - kOverlap,
                         avail.right() - hint.width() + 1);
    const int y = qBound(avail.top(), iconCenter.y() - hint.height(),
                         avail.bottom() - hint.height() + 1);
    menu.popup(QPoint{x, y});
}

// 打开设置对话框并应用结果（模态 exec，同一时刻只会有一个）
void openSettingsDialog(ReminderManager& reminder, DingDongWidget& bellWidget)
{
    SettingDialog dlg(nullptr, reminder.items(), reminder.autoStart(), reminder.stayMs());

    // Qt::QueuedConnection，解决栈上模态对话框信号槽失效
    QObject::connect(&dlg, &SettingDialog::signalTriggerNow, qApp, [&]()
    {
        const QString preview = reminder.nextText();
        bellWidget.showWidget(preview.isEmpty() ? "（预览）提醒来啦~" : preview);
    }, Qt::QueuedConnection);

    if(dlg.exec() != QDialog::Accepted)
        return;

    reminder.saveItems(dlg.getResultItems());
    reminder.setAutoStart(dlg.getAutoStart());
    AppInit::setAutoStart(dlg.getAutoStart());
    reminder.setStayMs(dlg.getStayMs());
    bellWidget.setStayMs(dlg.getStayMs());
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    app.setOrganizationName("DingDong");
    app.setApplicationName("DingDong");
    app.setQuitOnLastWindowClosed(false); // 关闭窗口不要退出程序，只隐藏到托盘

    // ===== 启动准备（都在黑窗口显示期间完成）=====
    const QString logPath = AppInit::initLogging();
    qDebug() << "\n********** DingDong START **********";
    qDebug() << "BuildTime[" __DATE__ " " __TIME__ "]";
    qDebug() << "日志文件：" << logPath;
    qDebug() << "正在加载配置与提醒时间...";

    // ===== 系统托盘 =====
    app.setWindowIcon(AppIcon::make());
    QSystemTrayIcon trayIcon{AppIcon::make()};
    QMenu trayMenu{};
    QAction* actSetting = trayMenu.addAction("设置");
    QAction* actQuit = trayMenu.addAction("退出");
    trayIcon.show();

    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, &app,
                     [&trayMenu, &trayIcon](QSystemTrayIcon::ActivationReason reason)
                     {
                         if(reason == QSystemTrayIcon::Context)
                             popupTrayMenu(trayMenu, trayIcon);
                     });

    // ===== 桌宠与定时调度 =====
    DingDongWidget bellWidget{};
    AppConfig config{};
    ReminderManager reminder{&config};

    QObject::connect(&reminder, &ReminderManager::triggered, &app, [&](const QString& text)
    {
        qDebug() << "到达定时，弹出提醒：" << text;
        bellWidget.showWidget(text);
    });
    reminder.start();
    bellWidget.setStayMs(reminder.stayMs());

    qDebug() << "启动完成，托盘图标已显示";
    trayIcon.showMessage("DingDong.exe", "DingDong 运行中...",
                         QSystemTrayIcon::Information, 3000);

    // 启动准备完毕、托盘提示已弹出后，关闭黑窗口
    AppInit::scheduleCloseConsole(2000);

    // ===== 交互 =====
    QObject::connect(actSetting, &QAction::triggered, &app, [&]()
    {
        openSettingsDialog(reminder, bellWidget);
    });

    // 双击桌宠：切换文本气泡显示/隐藏（到点弹出时不自动显示文本）
    QObject::connect(&bellWidget, &DingDongWidget::doubleClicked, &app, [&bellWidget]()
    {
        if(bellWidget.bubbleVisible())
            bellWidget.hideBubble();
        else
            bellWidget.showBubble();
    });

    QObject::connect(actQuit, &QAction::triggered, &app, [&]()
    {
        qDebug() << "********** DingDong EXIT **********";
        trayIcon.hide();
        app.quit();
    });

    const int ret = app.exec();
    AppInit::shutdown();
    return ret;
}
