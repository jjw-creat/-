#pragma once
#include <QObject>
#include <QTimer>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QSet>

class AutoDetectManager : public QObject
{
    Q_OBJECT

public:
    explicit AutoDetectManager(QObject *parent = nullptr);
    ~AutoDetectManager();

    void startDetection(int intervalMs = 1000);
    void stopDetection();
    bool isRunning() const;

    // ServeDebug 设备识别配置
    void setVendorId(quint16 vid) { m_vendorId = vid; }
    void setProductId(quint16 pid) { m_productId = pid; }
    void setHandshakeEnabled(bool enable) { m_useHandshake = enable; }

signals:
    void debuggerDetected(const QString &portName, const QString &description);
    void debuggerRemoved(const QString &portName);
    void portScanStarted();
    void portScanFinished();

private slots:
    void scanPorts();

private:
    bool checkIfServeDebug(const QSerialPortInfo &info);
    bool handshakeWithDevice(const QSerialPortInfo &info);

    QTimer *m_timer;
    QSet<QString> m_currentPorts;
    quint16 m_vendorId;
    quint16 m_productId;
    bool m_useHandshake;
    bool m_isScanning;
    bool isComOpen;
    QByteArray readMsg;
    QByteArray completeInsData;



};
