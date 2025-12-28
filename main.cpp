#include <QApplication>
#include <QMainWindow>
#include <QIcon>
#include<QResource>
#include<QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Serial AutoDetect");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Auto");
    app.setWindowIcon(QIcon(":/images/logo.png"));
    MainWindow window;

    window.show();

    return app.exec();
}
