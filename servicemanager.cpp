#include "servicemanager.h"
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
{
}

ServiceManager::~ServiceManager()
{
}

bool ServiceManager::installService(const QString& serviceName, const QString& displayName,
                                    const QString& executablePath)
{
#ifdef Q_OS_WIN
    return installServiceWindows(serviceName, displayName, executablePath);
#else
    return installServiceLinux(serviceName, displayName, executablePath);
#endif
}

bool ServiceManager::uninstallService(const QString& serviceName)
{
#ifdef Q_OS_WIN
    return uninstallServiceWindows(serviceName);
#else
    return uninstallServiceLinux(serviceName);
#endif
}

bool ServiceManager::startService(const QString& serviceName)
{
#ifdef Q_OS_WIN
    return startServiceWindows(serviceName);
#else
    return startServiceLinux(serviceName);
#endif
}

bool ServiceManager::stopService(const QString& serviceName)
{
#ifdef Q_OS_WIN
    return stopServiceWindows(serviceName);
#else
    return stopServiceLinux(serviceName);
#endif
}

bool ServiceManager::isServiceInstalled(const QString& serviceName)
{
#ifdef Q_OS_WIN
    return isServiceInstalledWindows(serviceName);
#else
    return isServiceInstalledLinux(serviceName);
#endif
}

bool ServiceManager::isServiceRunning(const QString& serviceName)
{
#ifdef Q_OS_WIN
    return isServiceRunningWindows(serviceName);
#else
    return isServiceRunningLinux(serviceName);
#endif
}

bool ServiceManager::setAutoStart(const QString& serviceName, bool enabled)
{
#ifdef Q_OS_WIN
    return setAutoStartWindows(serviceName, enabled);
#else
    return setAutoStartLinux(serviceName, enabled);
#endif
}

#ifdef Q_OS_WIN
bool ServiceManager::installServiceWindows(const QString& serviceName, const QString& displayName,
                                          const QString& executablePath)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        m_lastError = "Failed to open Service Control Manager";
        return false;
    }

    SC_HANDLE schService = CreateServiceW(
        schSCManager,
        serviceName.toStdWString().c_str(),
        displayName.toStdWString().c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        executablePath.toStdWString().c_str(),
        NULL, NULL, NULL, NULL, NULL
    );

    if (schService == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_EXISTS) {
            m_lastError = "Service already exists";
        } else {
            m_lastError = QString("Failed to create service (Error: %1)").arg(error);
        }
        CloseServiceHandle(schSCManager);
        return false;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool ServiceManager::uninstallServiceWindows(const QString& serviceName)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        m_lastError = "Failed to open Service Control Manager";
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(), DELETE);
    if (schService == NULL) {
        m_lastError = "Failed to open service";
        CloseServiceHandle(schSCManager);
        return false;
    }

    if (!DeleteService(schService)) {
        m_lastError = "Failed to delete service";
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return false;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool ServiceManager::startServiceWindows(const QString& serviceName)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        m_lastError = "Failed to open Service Control Manager";
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(),
                                       SERVICE_START);
    if (schService == NULL) {
        m_lastError = "Failed to open service";
        CloseServiceHandle(schSCManager);
        return false;
    }

    if (!StartService(schService, 0, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_SERVICE_ALREADY_RUNNING) {
            m_lastError = QString("Failed to start service (Error: %1)").arg(error);
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool ServiceManager::stopServiceWindows(const QString& serviceName)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        m_lastError = "Failed to open Service Control Manager";
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(),
                                       SERVICE_STOP);
    if (schService == NULL) {
        m_lastError = "Failed to open service";
        CloseServiceHandle(schSCManager);
        return false;
    }

    SERVICE_STATUS status;
    if (!ControlService(schService, SERVICE_CONTROL_STOP, &status)) {
        DWORD error = GetLastError();
        if (error != ERROR_SERVICE_NOT_ACTIVE) {
            m_lastError = QString("Failed to stop service (Error: %1)").arg(error);
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool ServiceManager::isServiceInstalledWindows(const QString& serviceName)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL) {
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(),
                                       SERVICE_QUERY_STATUS);
    bool installed = (schService != NULL);

    if (schService) {
        CloseServiceHandle(schService);
    }
    CloseServiceHandle(schSCManager);
    return installed;
}

bool ServiceManager::isServiceRunningWindows(const QString& serviceName)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL) {
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(),
                                       SERVICE_QUERY_STATUS);
    if (schService == NULL) {
        CloseServiceHandle(schSCManager);
        return false;
    }

    SERVICE_STATUS status;
    bool running = false;
    if (QueryServiceStatus(schService, &status)) {
        running = (status.dwCurrentState == SERVICE_RUNNING);
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return running;
}

bool ServiceManager::setAutoStartWindows(const QString& serviceName, bool enabled)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        m_lastError = "Failed to open Service Control Manager";
        return false;
    }

    SC_HANDLE schService = OpenServiceW(schSCManager, serviceName.toStdWString().c_str(),
                                       SERVICE_CHANGE_CONFIG);
    if (schService == NULL) {
        m_lastError = "Failed to open service";
        CloseServiceHandle(schSCManager);
        return false;
    }

    DWORD startType = enabled ? SERVICE_AUTO_START : SERVICE_DEMAND_START;
    if (!ChangeServiceConfig(schService, SERVICE_NO_CHANGE, startType,
                            SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
        m_lastError = "Failed to change service configuration";
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return false;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

#else
bool ServiceManager::installServiceLinux(const QString& serviceName, const QString& displayName,
                                        const QString& executablePath)
{
    QString serviceFile = QString("/etc/systemd/system/%1.service").arg(serviceName);
    
    QFile file(serviceFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Failed to create service file";
        return false;
    }

    QTextStream out(&file);
    out << "[Unit]\n";
    out << "Description=" << displayName << "\n";
    out << "After=network.target\n\n";
    out << "[Service]\n";
    out << "Type=simple\n";
    out << "ExecStart=" << executablePath << "\n";
    out << "Restart=on-failure\n";
    out << "RestartSec=5s\n\n";
    out << "[Install]\n";
    out << "WantedBy=multi-user.target\n";
    
    file.close();

    QProcess::execute("systemctl", QStringList() << "daemon-reload");
    return true;
}

bool ServiceManager::uninstallServiceLinux(const QString& serviceName)
{
    stopServiceLinux(serviceName);
    
    QString serviceFile = QString("/etc/systemd/system/%1.service").arg(serviceName);
    if (!QFile::remove(serviceFile)) {
        m_lastError = "Failed to remove service file";
        return false;
    }

    QProcess::execute("systemctl", QStringList() << "daemon-reload");
    return true;
}

bool ServiceManager::startServiceLinux(const QString& serviceName)
{
    int result = QProcess::execute("systemctl", QStringList() << "start" << serviceName);
    if (result != 0) {
        m_lastError = "Failed to start service";
        return false;
    }
    return true;
}

bool ServiceManager::stopServiceLinux(const QString& serviceName)
{
    int result = QProcess::execute("systemctl", QStringList() << "stop" << serviceName);
    if (result != 0) {
        m_lastError = "Failed to stop service";
        return false;
    }
    return true;
}

bool ServiceManager::isServiceInstalledLinux(const QString& serviceName)
{
    QString serviceFile = QString("/etc/systemd/system/%1.service").arg(serviceName);
    return QFile::exists(serviceFile);
}

bool ServiceManager::isServiceRunningLinux(const QString& serviceName)
{
    QProcess process;
    process.start("systemctl", QStringList() << "is-active" << serviceName);
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput().trimmed();
    return output == "active";
}

bool ServiceManager::setAutoStartLinux(const QString& serviceName, bool enabled)
{
    QStringList args;
    args << (enabled ? "enable" : "disable") << serviceName;
    
    int result = QProcess::execute("systemctl", args);
    if (result != 0) {
        m_lastError = "Failed to change autostart setting";
        return false;
    }
    return true;
}
#endif
