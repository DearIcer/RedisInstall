#include "mainwindow.h"
#include "serviceconfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Show main management window (maximized)
    MainWindow w;
    w.showMaximized();
    
    return a.exec();
}
