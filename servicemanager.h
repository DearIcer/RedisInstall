#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QString>
#include <QObject>

class ServiceManager : public QObject
{
    Q_OBJECT
    
public:
    explicit ServiceManager(QObject *parent = nullptr);
    ~ServiceManager();
    
    bool installService(const QString& serviceName, const QString& displayName, 
                       const QString& executablePath);
    bool uninstallService(const QString& serviceName);
    bool startService(const QString& serviceName);
    bool stopService(const QString& serviceName);
    bool isServiceInstalled(const QString& serviceName);
    bool isServiceRunning(const QString& serviceName);
    
    bool setAutoStart(const QString& serviceName, bool enabled);
    
    QString getLastError() const { return m_lastError; }
    
signals:
    void serviceStatusChanged(const QString& status);
    void errorOccurred(const QString& error);
    
private:
#ifdef Q_OS_WIN
    bool installServiceWindows(const QString& serviceName, const QString& displayName,
                              const QString& executablePath);
    bool uninstallServiceWindows(const QString& serviceName);
    bool startServiceWindows(const QString& serviceName);
    bool stopServiceWindows(const QString& serviceName);
    bool isServiceInstalledWindows(const QString& serviceName);
    bool isServiceRunningWindows(const QString& serviceName);
    bool setAutoStartWindows(const QString& serviceName, bool enabled);
#else
    bool installServiceLinux(const QString& serviceName, const QString& displayName,
                            const QString& executablePath);
    bool uninstallServiceLinux(const QString& serviceName);
    bool startServiceLinux(const QString& serviceName);
    bool stopServiceLinux(const QString& serviceName);
    bool isServiceInstalledLinux(const QString& serviceName);
    bool isServiceRunningLinux(const QString& serviceName);
    bool setAutoStartLinux(const QString& serviceName, bool enabled);
#endif
    
    QString m_lastError;
};

#endif // SERVICEMANAGER_H
