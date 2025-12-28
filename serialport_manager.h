#pragma once
#include <QObject>
#include <QSerialPort>
#include <QTimer>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();
qint64 sendData(const QByteArray &data); // 发送数据函数
    bool connectToPort(const QString &portName, int baudRate = 115200);
    void disconnectPort();
    bool isConnected() const;
    QString currentPort() const;

signals:
    void connected(const QString &portName);
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void bytesWritten(qint64 bytes); // 发送成功信号

private slots:
    void onReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QString m_currentPort;
    bool m_isConnected;
};
