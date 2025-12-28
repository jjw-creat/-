#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "autodetect_manager.h"
#include "serialport_manager.h"
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include<QDebug>

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

    logMessage("ServeDebug 自动检测系统已启动");
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

    // 串口管理信号
    connect(m_serialManager, &SerialPortManager::connected,
            this, &MainWindow::onSerialConnected);
    connect(m_serialManager, &SerialPortManager::disconnected,
            this, &MainWindow::onSerialDisconnected);
    connect(m_serialManager, &SerialPortManager::errorOccurred,
            this, [this](const QString &error) { logMessage(error, true); });

    // 构造函数
    connect(ui->btnClearLog, &QPushButton::clicked,ui->textLog, &QPlainTextEdit::clear);
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
    logMessage(message);

    // 如果当前连接的设备被移除，自动断开
    if (m_isConnected && m_serialManager->currentPort() == portName) {
        m_serialManager->disconnectPort();
    }
}

void MainWindow::onSerialConnected(const QString &portName)
{
    m_isConnected = true;
    updateConnectionStatus(true);

    QString message = QString("✅ 已连接到: %1").arg(portName);
    logMessage(message);
    ui->labelCurrentPort->setText(QStringLiteral("端口: %1").arg(portName));
    ui->statusbar->showMessage(message);
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

