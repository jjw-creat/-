#include "serialport_manager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_isConnected(false)
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialPortManager::handleError);
}

SerialPortManager::~SerialPortManager()
{
    disconnectPort();
}

bool SerialPortManager::connectToPort(const QString &portName, int baudRate)
{
    if (m_isConnected) {
        disconnectPort();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_currentPort = portName;
        m_isConnected = true;
        qDebug() << "✅ 成功连接到端口:" << portName;
        emit connected(portName);
        return true;
    } else {
        QString error = QString("连接失败: %1").arg(m_serialPort->errorString());
        qDebug() << "❌" << error;
        emit errorOccurred(error);
        return false;
    }
}

qint64 SerialPortManager::sendData(const QByteArray &data)
{
    if (!m_isConnected || !m_serialPort->isOpen()) return -1;

    qint64 bytes = m_serialPort->write(data);
    if (bytes > 0) {
        emit bytesWritten(bytes); // 触发信号
    }
    return bytes;
}
void SerialPortManager::disconnectPort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    m_isConnected = false;
    m_currentPort.clear();
    emit disconnected();
    qDebug() << "🔌 串口已断开";
}

bool SerialPortManager::isConnected() const
{
    return m_isConnected;
}

QString SerialPortManager::currentPort() const
{
    return m_currentPort;
}


void SerialPortManager::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;

    QString errorString = m_serialPort->errorString();
    qDebug() << "串口错误:" << errorString;
    emit errorOccurred(errorString);

    // 发生严重错误时自动断开
    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::PermissionError ||
        error == QSerialPort::DeviceNotFoundError) {
        disconnectPort();
    }
}
