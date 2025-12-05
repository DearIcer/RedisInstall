#include "serviceconfig.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

ServiceConfig& ServiceConfig::instance()
{
    static ServiceConfig config;
    return config;
}

ServiceConfig::ServiceConfig()
    : m_ipAddress("0.0.0.0")
    , m_port(10833)
    , m_password("")
    , m_autoStart(true)
    , m_serviceInstalled(false)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir;
    dir.mkpath(configPath);
    
    m_settings = new QSettings(configPath + "/config.ini", QSettings::IniFormat);
    load();
}

QString ServiceConfig::getIpAddress() const
{
    return m_ipAddress;
}

void ServiceConfig::setIpAddress(const QString& ip)
{
    m_ipAddress = ip;
}

int ServiceConfig::getPort() const
{
    return m_port;
}

void ServiceConfig::setPort(int port)
{
    m_port = port;
}

QString ServiceConfig::getPassword() const
{
    return m_password;
}

void ServiceConfig::setPassword(const QString& password)
{
    m_password = password;
}

bool ServiceConfig::isAutoStart() const
{
    return m_autoStart;
}

void ServiceConfig::setAutoStart(bool enabled)
{
    m_autoStart = enabled;
}

bool ServiceConfig::isServiceInstalled() const
{
    return m_serviceInstalled;
}

void ServiceConfig::setServiceInstalled(bool installed)
{
    m_serviceInstalled = installed;
}

void ServiceConfig::save()
{
    m_settings->beginGroup("Service");
    m_settings->setValue("IpAddress", m_ipAddress);
    m_settings->setValue("Port", m_port);
    m_settings->setValue("Password", m_password);
    m_settings->setValue("AutoStart", m_autoStart);
    m_settings->setValue("Installed", m_serviceInstalled);
    m_settings->endGroup();
    m_settings->sync();
}

void ServiceConfig::load()
{
    m_settings->beginGroup("Service");
    m_ipAddress = m_settings->value("IpAddress", "0.0.0.0").toString();
    m_port = m_settings->value("Port", 10833).toInt();
    m_password = m_settings->value("Password", "").toString();
    m_autoStart = m_settings->value("AutoStart", true).toBool();
    m_serviceInstalled = m_settings->value("Installed", false).toBool();
    m_settings->endGroup();
}
