#ifndef SERVICECONFIG_H
#define SERVICECONFIG_H

#include <QString>
#include <QSettings>

class ServiceConfig
{
public:
    static ServiceConfig& instance();
    
    QString getIpAddress() const;
    void setIpAddress(const QString& ip);
    
    int getPort() const;
    void setPort(int port);
    
    bool isAutoStart() const;
    void setAutoStart(bool enabled);
    
    bool isServiceInstalled() const;
    void setServiceInstalled(bool installed);
    
    void save();
    void load();
    
private:
    ServiceConfig();
    ~ServiceConfig() = default;
    ServiceConfig(const ServiceConfig&) = delete;
    ServiceConfig& operator=(const ServiceConfig&) = delete;
    
    QString m_ipAddress;
    int m_port;
    bool m_autoStart;
    bool m_serviceInstalled;
    
    QSettings* m_settings;
};

#endif // SERVICECONFIG_H
