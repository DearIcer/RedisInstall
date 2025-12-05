#ifndef REDISMANAGER_H
#define REDISMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RedisManager : public QObject
{
    Q_OBJECT
    
public:
    explicit RedisManager(QObject *parent = nullptr);
    ~RedisManager();
    
    // Redis installation
    bool isRedisInstalled() const;
    void downloadRedis(const QString& version = "latest");
    void installRedis(const QString& installPath);
    void uninstallRedis();
    
    // Redis service control
    bool startRedis(const QString& ip, int port, const QString& password = "");
    bool stopRedis();
    bool restartRedis(const QString& ip, int port, const QString& password = "");
    bool isRedisRunning() const;
    
    // Configuration
    void updateRedisConfig(const QString& ip, int port, const QString& password = "");
    QString getRedisVersion() const;
    QString getRedisPath() const;
    QString getLastError() const { return m_lastError; }
    
signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(bool success);
    void installationProgress(const QString& message);
    void installationFinished(bool success);
    void redisStarted();
    void redisStopped();
    void errorOccurred(const QString& error);
    
private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onRedisProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRedisProcessError(QProcess::ProcessError error);
    
private:
    bool extractRedisArchive(const QString& archivePath, const QString& destPath);
    bool createRedisConfig(const QString& ip, int port);
    bool createRedisConfigWithPassword(const QString& ip, int port, const QString& password);
    QString getRedisDownloadUrl() const;
    QString getDefaultInstallPath() const;
    bool killRedisProcess();
    
private:
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_downloadReply;
    QProcess* m_redisProcess;
    
    QString m_redisPath;
    QString m_redisConfigPath;
    QString m_downloadedFilePath;
    QString m_lastError;
    
    bool m_isInstalled;
    bool m_isRunning;
};

#endif // REDISMANAGER_H
