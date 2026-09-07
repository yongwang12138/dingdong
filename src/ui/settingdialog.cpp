#include "ui/settingdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QTimeEdit>
#include <QLineEdit>
#include <QHeaderView>
#include <QDebug>

SettingDialog::SettingDialog(QWidget *parent,
                             const QList<ReminderItem> &initItems,
                             bool initAutoStart,
                             int initStayMs)
    : QDialog(parent)
    , m_items(initItems)
    , m_autoStart(initAutoStart)
    , m_stayMs(initStayMs)
{
    setWindowTitle("叮咚提醒设置");
    // 图标继承 QApplication::setWindowIcon（main.cpp 里统一设置），无需单独指定
    setMinimumSize(520, 380);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Tab控件
    m_tabWidget = new QTabWidget;
    mainLayout->addWidget(m_tabWidget);

    // ========= Tab1：常规 =========
    QWidget* pageGeneral = new QWidget;
    QVBoxLayout* layGeneral = new QVBoxLayout(pageGeneral);
    layGeneral->setContentsMargins(12,12,12,12);
    layGeneral->setSpacing(16);

    m_checkAutoStart = new QCheckBox("开机自动启动");
    m_checkAutoStart->setChecked(m_autoStart);
    layGeneral->addWidget(m_checkAutoStart);

    m_btnTriggerNow = new QPushButton("立即触发提醒（预览效果）");
    layGeneral->addWidget(m_btnTriggerNow);

    QHBoxLayout* stayRow = new QHBoxLayout;
    stayRow->addWidget(new QLabel("提醒停留时间(秒)："));
    m_spinStay = new QSpinBox;
    m_spinStay->setRange(1, 600);
    m_spinStay->setValue(m_stayMs / 1000);
    stayRow->addWidget(m_spinStay);
    stayRow->addStretch();
    layGeneral->addLayout(stayRow);

    connect(m_spinStay, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v){
        m_stayMs = v * 1000;
    });

    layGeneral->addStretch();
    m_tabWidget->addTab(pageGeneral, "常规");

    // ========= Tab2：提醒时间 =========
    QWidget* pageTime = new QWidget;
    QVBoxLayout* layTime = new QVBoxLayout(pageTime);
    layTime->setContentsMargins(12,12,12,12);

    m_table = new QTableWidget;
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels(QStringList() << "提醒时间" << "提醒文本" << "");
    m_table->verticalHeader()->setVisible(false);
    m_table->setColumnWidth(0, 110);
    m_table->setColumnWidth(2, 40);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    layTime->addWidget(m_table);

    QHBoxLayout* btnRow = new QHBoxLayout;
    m_btnAdd = new QPushButton("添加");
    btnRow->addWidget(m_btnAdd);
    btnRow->addStretch();
    layTime->addLayout(btnRow);

    m_tabWidget->addTab(pageTime, "提醒时间");

    // ========= Tab3：关于 =========
    QWidget* pageAbout = new QWidget;
    QVBoxLayout* layAbout = new QVBoxLayout(pageAbout);
    layAbout->setContentsMargins(12, 12, 12, 12);
    layAbout->setSpacing(16);

    QLabel* lblAbout = new QLabel(
        "<h2>叮咚 DingDong</h2>"
        "<p>版本：1.0</p>"
        "<p>一个简单的小工具，用于在指定时间弹出提醒。</p>"
        "<p>图标编入可执行文件，无需额外图片资源。</p>"
        "<p>Copyright © 2026 DingDong</p>");
    lblAbout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lblAbout->setWordWrap(true);
    lblAbout->setOpenExternalLinks(true);
    layAbout->addWidget(lblAbout);
    layAbout->addStretch();

    m_tabWidget->addTab(pageAbout, "关于");

    // ========= 底部确定取消 =========
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    m_btnOk = new QPushButton("确定");
    m_btnCancel = new QPushButton("取消");
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_btnOk);
    bottomLayout->addWidget(m_btnCancel);
    mainLayout->addLayout(bottomLayout);

    fillTable();

    //信号绑定
    connect(m_btnAdd, &QPushButton::clicked, this, &SettingDialog::slotAddRow);
    connect(m_btnTriggerNow, &QPushButton::clicked, this, [this](){
        emit signalTriggerNow();
        accept(); // 立即触发时关闭设置页，让弹窗正常显示
    });
    connect(m_btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingDialog::addRow(const QTime& timeVal, const QString& text)
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);

    QWidget* c0 = new QWidget;
    QHBoxLayout* l0 = new QHBoxLayout(c0);
    l0->setContentsMargins(2, 2, 2, 2);
    QTimeEdit* te = new QTimeEdit(timeVal);
    te->setDisplayFormat("HH:mm");
    te->setObjectName("timeEdit");
    l0->addWidget(te);
    m_table->setCellWidget(row, 0, c0);

    QWidget* c1 = new QWidget;
    QHBoxLayout* l1 = new QHBoxLayout(c1);
    l1->setContentsMargins(2, 2, 2, 2);
    QLineEdit* le = new QLineEdit(text);
    le->setPlaceholderText("提醒时显示的文本");
    le->setObjectName("textEdit");
    l1->addWidget(le);
    m_table->setCellWidget(row, 1, c1);

    QWidget* c2 = new QWidget;
    QHBoxLayout* l2 = new QHBoxLayout(c2);
    l2->setContentsMargins(2, 2, 2, 2);
    QPushButton* del = new QPushButton("×");
    del->setFixedWidth(28);
    del->setStyleSheet(R"(
QPushButton{border:none;color:#dd3333;font-size:16px;}
QPushButton:hover{background-color:#ffdddd;}
)");
    l2->addWidget(del);
    m_table->setCellWidget(row, 2, c2);

    connect(del, &QPushButton::clicked, this, [this, row](){
        slotDeleteRow(row);
    });
}

void SettingDialog::fillTable()
{
    m_table->setRowCount(0);
    for(const auto& it : m_items)
        addRow(it.time, it.text);
}

void SettingDialog::slotAddRow()
{
    addRow(QTime(14, 50), "");
}

void SettingDialog::slotDeleteRow(int row)
{
    if(row < 0 || row >= m_table->rowCount())
        return;
    m_table->removeRow(row);
    // 重建内部列表并刷新（删除后行号会变，刷新可修正删除按钮绑定的 row）
    m_items.clear();
    for(int r = 0; r < m_table->rowCount(); ++r)
    {
        QTimeEdit* te = m_table->cellWidget(r, 0)->findChild<QTimeEdit*>("timeEdit");
        QLineEdit* le = m_table->cellWidget(r, 1)->findChild<QLineEdit*>("textEdit");
        if(te)
            m_items.append({te->time(), le ? le->text() : QString()});
    }
    fillTable();
}

QList<ReminderItem> SettingDialog::getResultItems() const
{
    QList<ReminderItem> out;
    for(int r = 0; r < m_table->rowCount(); ++r)
    {
        QWidget* c0 = m_table->cellWidget(r, 0);
        QWidget* c1 = m_table->cellWidget(r, 1);
        if(!c0 || !c1)
            continue;
        QTimeEdit* te = c0->findChild<QTimeEdit*>("timeEdit");
        QLineEdit* le = c1->findChild<QLineEdit*>("textEdit");
        if(te)
            out.append({te->time(), le ? le->text() : QString()});
    }
    return out;
}

bool SettingDialog::getAutoStart() const
{
    return m_checkAutoStart->isChecked();
}

int SettingDialog::getStayMs() const
{
    return m_stayMs;
}
