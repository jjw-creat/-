#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "autodetect_manager.h"
#include "serialport_manager.h"
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include<QDebug>
#include<QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_autoDetect(new AutoDetectManager(this))
    , m_serialManager(new SerialPortManager(this))
    , m_isConnected(false)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    // 启动自动检测
    m_autoDetect->startDetection(1500);
    refreshPortList();
    ui->cbBaudRate->addItems({"9600", "19200", "38400", "57600", "115200"});
    ui->cbBaudRate->setCurrentText("115200");
    logMessage("Debug串口自动检测系统已启动");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("ServeDebug AutoDetect v2.0");
    setMinimumSize(800, 600);

    // 设置状态栏
    ui->statusbar->showMessage("就绪");

    // 初始化控件状态
    updateConnectionStatus(false);
}

void MainWindow::setupConnections()
{
    // 自动检测信号
    connect(m_autoDetect, &AutoDetectManager::debuggerDetected,
            this, &MainWindow::onDebuggerDetected);
    connect(m_autoDetect, &AutoDetectManager::debuggerRemoved,
            this, &MainWindow::onDebuggerRemoved);
    connect(m_serialManager, &SerialPortManager::bytesWritten, this, [this](qint64 bytes){
            m_txBytes += bytes;updateStats();});
    // 串口管理信号
    connect(m_serialManager, &SerialPortManager::connected,
            this, &MainWindow::onSerialConnected);
    connect(m_serialManager, &SerialPortManager::disconnected,
            this, &MainWindow::onSerialDisconnected);
    connect(m_serialManager, &SerialPortManager::errorOccurred,
            this, [this](const QString &error) { logMessage(error, true); });

    // 构造函数
    connect(ui->btnClearLog, &QPushButton::clicked,ui->textLog, &QPlainTextEdit::clear);
    disconnect(m_serialManager, &SerialPortManager::dataReceived, 0, 0);
    connect(m_serialManager, &SerialPortManager::dataReceived, this, [this](const QByteArray &data){
    m_rxBytes += data.size();
    updateStats();
    QString displayData;
    if (ui->chkHexDisplay->isChecked()) {
    displayData = data.toHex(' ').toUpper();
        }
    else {
            displayData = QString::fromUtf8(data);
     }
     logMessage(QString("RX: %1").arg(displayData));
});
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::on_btnSend_clicked);
}

void MainWindow::on_btnSend_clicked()
{
    if (!m_isConnected) {
        QMessageBox::warning(this, "错误", "请先连接设备");
        return;
    }

    // 获取发送内容 (假设UI有个 txtSend 输入框)
    // QString text = ui->txtSend->text();
    QString text = "TEST_DATA"; // 临时示例

    if (text.isEmpty()) return;

    QByteArray dataToSend = text.toUtf8();
    // 如果需要支持Hex发送，可以在这里加判断

    m_serialManager->sendData(dataToSend);
    logMessage(QString("TX: %1").arg(text));
}
void MainWindow::updateStats()
{
    QString stats = QString("RX: %1 Bytes | TX: %2 Bytes")
                    .arg(m_rxBytes).arg(m_txBytes);
    ui->statusbar->showMessage(stats);
}
void MainWindow::onDebuggerDetected(const QString &portName, const QString &description)
{
    QString message = QString("🔍 检测到 ServeDebug 设备: %1 (%2)").arg(portName).arg(description);
    logMessage(message);
    // 自动选择新检测到的设备
    for (int i = 0; i < ui->listDevices->count(); ++i) {
        QListWidgetItem *item = ui->listDevices->item(i);
        if (item->text().contains(portName)) {
            ui->listDevices->setCurrentItem(item);
            break;
        }
    }

    // 可选：自动连接（取消注释启用）
     if (!m_isConnected) {
      m_serialManager->connectToPort(portName);
     }
}

void MainWindow::onDebuggerRemoved(const QString &portName)
{
    QString message = QString("❌ 设备已移除: %1").arg(portName);
    logMessage(message, true); // 建议这里标记为红色错误信息

    // 如果当前连接的设备被移除
    // 注意：检查 m_serialManager->currentPort() 是否和移除的 portName 一致
    if (m_isConnected && m_serialManager->currentPort() == portName) {
        logMessage("检测到当前活动设备移除，正在强制断开...");
        m_serialManager->disconnectPort();
        // UI 更新会由 onSerialDisconnected 信号触发
    }
}

void MainWindow::onSerialDisconnected()
{
    m_isConnected = false;
    updateConnectionStatus(false);

    logMessage("🔌 连接已断开");
    ui->statusbar->showMessage("连接已断开");
}

void MainWindow::on_btnManualRefresh_clicked()
{
    refreshPortList();
    logMessage("手动刷新设备列表");
}

void MainWindow::on_btnConnect_clicked()
{
    QListWidgetItem *currentItem = ui->listDevices->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "警告", "请先选择一个设备");
        return;
    }

    QString portName = currentItem->text().split(" - ").first();
    int baudRate = ui->cbBaudRate->currentText().toInt();
    logMessage(QString("正在连接: %1").arg(portName));
    m_serialManager->connectToPort(portName);

}

void MainWindow::on_btnDisconnect_clicked()
{
    m_serialManager->disconnectPort();
}

void MainWindow::refreshPortList()
{
    ui->listDevices->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        QString itemText = QString("%1 - %2")
            .arg(port.portName())
            .arg(port.description());
        ui->listDevices->addItem(itemText);
    }

    ui->labelDeviceCount->setText(QString("找到 %1 个设备").arg(ports.count()));
}

void MainWindow::logMessage(const QString &message, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);

    QTextCharFormat format;
    if (isError) {
        format.setForeground(QBrush(Qt::red));
    } else {
        format.setForeground(QBrush(Qt::blue));
    }

    // 限制日志最大行数为 1000 行
    if (ui->textLog->document()->blockCount() > 1000) {
        // 删除第一行（最旧的一行）
        QTextCursor deleteCursor(ui->textLog->document());
        deleteCursor.movePosition(QTextCursor::Start);
        deleteCursor.select(QTextCursor::BlockUnderCursor);
        deleteCursor.removeSelectedText();
        deleteCursor.deleteChar(); // 删除换行符
    }

    QTextCursor cursor(ui->textLog->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(logEntry + "\n", format);

    // 自动滚动
    QScrollBar *scrollbar = ui->textLog->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void MainWindow::updateConnectionStatus(bool connected)
{
    ui->btnConnect->setEnabled(!connected);
    ui->btnDisconnect->setEnabled(connected);

    if (connected) {
        ui->labelStatus->setText("🟢 已连接");
        ui->labelStatus->setStyleSheet("color: green; font-weight: bold;");
    } else {
        ui->labelStatus->setText("🔴 未连接");
        ui->labelStatus->setStyleSheet("color: red;");
    }
}

// 实现双击列表项直接连接
void MainWindow::on_listDevices_itemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    // 直接调用连接按钮的逻辑
    on_btnConnect_clicked();
}
void MainWindow::onSerialConnected(const QString &portName)
{
    m_isConnected = true;
    updateConnectionStatus(true);

    // 清零计数器
    m_rxBytes = 0;
    m_txBytes = 0;
    updateStats();

    QString message = QString("✅ 设备已连接: %1").arg(portName);
    logMessage(message);
    ui->labelCurrentPort->setText(QStringLiteral("端口: %1").arg(portName));
}
