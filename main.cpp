#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("ServeDebug AutoDetect");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("ServeDebug");

    MainWindow window;
    window.show();

    return app.exec();
}
