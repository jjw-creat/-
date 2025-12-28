#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#pragma once
#include <QDialog>
#include <QListWidgetItem>
#include <QVariantMap>
#include "ui_portdialog.h"

// 前向声明
namespace Ui {
class PortDialog;
}

class AutoDetectManager;
class SerialPortManager;

class PortDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PortDialog(QVariantMap param, QWidget *parent = 0);
    ~PortDialog();

private slots:
    void onDebuggerDetected(const QString &portName, const QString &description);
    void onDebuggerRemoved(const QString &portName);
    void onSerialConnected(const QString &portName);
    void onSerialDisconnected();
    void on_btnManualRefresh_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

private:
    void setupUI();
    void setupConnections();
    void refreshPortList();
    void logMessage(const QString &message, bool isError = false);
    void updateConnectionStatus(bool connected);

    Ui::PortDialog *ui;
    AutoDetectManager *m_autoDetect;
    SerialPortManager *m_serialManager;
    QVariantMap paramMap;
    bool m_isConnected;
};
#endif
