#include "mainwindow.h"
#include "installwizard.h"
#include "serviceconfig.h"
#include "servicemanager.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Service Manager");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption installOption(QStringList() << "i" << "install",
                                    "Run installation wizard");
    parser.addOption(installOption);
    
    QCommandLineOption serviceOption(QStringList() << "s" << "service",
                                    "Run as service");
    parser.addOption(serviceOption);
    
    parser.process(a);
    
    // Check if service is already installed
    bool isInstalled = ServiceConfig::instance().isServiceInstalled();
    
    if (parser.isSet(installOption) || !isInstalled) {
        // Show installation wizard
        InstallWizard wizard;
        wizard.show();
        
        QObject::connect(&wizard, &InstallWizard::installCompleted, 
                        [&](const QString& ip, int port, bool autoStart) {
            // After installation, can optionally show main window
            MainWindow* w = new MainWindow();
            w->show();
        });
        
        QObject::connect(&wizard, &InstallWizard::installCancelled, &a, &QApplication::quit);
        
        return a.exec();
    } else if (parser.isSet(serviceOption)) {
        // Run as service (background mode)
        // Here you would implement the actual service logic
        // For now, we just exit
        return 0;
    } else {
        // Show main management window
        MainWindow w;
        w.show();
        return a.exec();
    }
}
