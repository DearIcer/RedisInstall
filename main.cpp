#include "mainwindow.h"
#include "serviceconfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 显示最大化的主窗口
    MainWindow w;
    w.showMaximized();
    
    return a.exec();
}
