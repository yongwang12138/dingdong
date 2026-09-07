#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QList>
#include "core/config.h"

class QTabWidget;
class QTableWidget;
class QPushButton;
class QCheckBox;
class QSpinBox;

class SettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget* parent = nullptr,
                           const QList<ReminderItem>& initItems = {},
                           bool initAutoStart = false,
                           int initStayMs = 30000);

    QList<ReminderItem> getResultItems() const;
    bool getAutoStart() const;
    int getStayMs() const;

signals:
    void signalTriggerNow();

private slots:
    void slotAddRow();
    void slotDeleteRow(int row);

private:
    void fillTable();
    void addRow(const QTime& timeVal, const QString& text);

    QTabWidget* m_tabWidget{nullptr};

    // 常规tab
    QCheckBox* m_checkAutoStart{nullptr};
    QPushButton* m_btnTriggerNow{nullptr};
    QSpinBox* m_spinStay{nullptr};

    // 提醒时间tab
    QTableWidget* m_table{nullptr};
    QPushButton* m_btnAdd{nullptr};

    // 底部
    QPushButton* m_btnOk{nullptr};
    QPushButton* m_btnCancel{nullptr};

    QList<ReminderItem> m_items;
    bool m_autoStart{false};
    int m_stayMs{30000};
};

#endif // SETTINGDIALOG_H
