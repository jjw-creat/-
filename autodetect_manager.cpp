#include "autodetect_manager.h"
#include <QDebug>
#include <QThread>

AutoDetectManager::AutoDetectManager(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_vendorId(0x1A86)    // CH340 默认 VID
    , m_productId(0x7523)   // CH340 默认 PID
    , m_useHandshake(false)
    , m_isScanning(false)
{
    connect(m_timer, &QTimer::timeout, this, &AutoDetectManager::scanPorts);
}

AutoDetectManager::~AutoDetectManager()
{
    stopDetection();
}

void AutoDetectManager::startDetection(int intervalMs)
{
    m_timer->start(intervalMs);
    qDebug() << "ServeDebug 自动检测已启动，间隔:" << intervalMs << "ms";
}

void AutoDetectManager::stopDetection()
{
    m_timer->stop();
    qDebug() << "ServeDebug 自动检测已停止";
}

bool AutoDetectManager::isRunning() const
{
    return m_timer->isActive();
}

void AutoDetectManager::scanPorts()
{
    if (m_isScanning) return;

    m_isScanning = true;
    emit portScanStarted();

    QSet<QString> newPorts;
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();

    // 检测新增端口
    for (const QSerialPortInfo &info : availablePorts) {
        QString portName = info.portName();
        newPorts.insert(portName);

        if (!m_currentPorts.contains(portName)) {
            qDebug() << "发现新端口:" << portName << "描述:" << info.description();

            if (checkIfServeDebug(info)) {
                qDebug() << "✅ 识别到 ServeDebug 设备:" << portName;
                emit debuggerDetected(portName, info.description());
            }
        }
    }

    // 检测移除的端口
    for (const QString &portName : m_currentPorts) {
        if (!newPorts.contains(portName)) {
            qDebug() << "❌ 端口已移除:" << portName;
            emit debuggerRemoved(portName);
        }
    }

    m_currentPorts = newPorts;
    m_isScanning = false;
    emit portScanFinished();
}

bool AutoDetectManager::checkIfServeDebug(const QSerialPortInfo &info)
{
    // 方法1: 通过 VID/PID 识别（最可靠）
        if (info.hasVendorIdentifier() && info.hasProductIdentifier()) {
            if (info.vendorIdentifier() == m_vendorId &&
                info.productIdentifier() == m_productId) {
                return true;
            }
        }

        // 方法2: 通过设备描述识别
        QString description = info.description().toLower();
        // 【修改开始】
        if (description.contains("serial") || description.contains("ch340") ||
            description.contains("cp210") || description.contains("ft232") ||
            description.contains("usb")) {
            // 既然匹配了常见串口关键字，应该返回 true
            return true;
        }
        // 【修改结束】

        return false;
}

/*if (m_useHandshake) {
        return handshakeWithDevice(info);
    }


}

bool AutoDetectManager::handshakeWithDevice(const QSerialPortInfo &info)
{
    QSerialPort port;
    port.setPort(info);
    port.setBaudRate(QSerialPort::Baud115200);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);

    if (!port.open(QIODevice::ReadWrite)) {
        return false;
    }

    // ServeDebug 握手协议示例
    QByteArray handshakeCmd = "SERVEDEBUG_HELLO\n";
    port.write(handshakeCmd);

    if (port.waitForBytesWritten(1000)) {
        if (port.waitForReadyRead(1000)) {
            QByteArray response = port.readAll();
            port.close();

            // 检查响应是否符合 ServeDebug 协议
            if (response.contains("SERVEDEBUG_READY") ||
                response.contains("DEBUG_READY")) {
                return true;
            }
        }
    }

    port.close();
    return false;
}

*/
