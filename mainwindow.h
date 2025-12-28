#pragma once
#include <QMainWindow>
#include <QListWidgetItem>

// 前向声明
namespace Ui {
class MainWindow;
}

class AutoDetectManager;
class SerialPortManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDebuggerDetected(const QString &portName, const QString &description);
    void onDebuggerRemoved(const QString &portName);
    void onSerialConnected(const QString &portName);
    void onSerialDisconnected();
    void on_btnSend_clicked();
    void updateStats();
    void on_btnManualRefresh_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_listDevices_itemDoubleClicked(QListWidgetItem *item);
private:
    void setupUI();
    void setupConnections();
    void refreshPortList();
    void logMessage(const QString &message, bool isError = false);
    void updateConnectionStatus(bool connected);

    Ui::MainWindow *ui;
    AutoDetectManager *m_autoDetect;
    SerialPortManager *m_serialManager;
    bool m_isConnected;
    quint64 m_rxBytes = 0;
    quint64 m_txBytes = 0;
};
