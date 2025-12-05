#include "redismanager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QFileInfo>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#else
#include <signal.h>
#endif

RedisManager::RedisManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_downloadReply(nullptr)
    , m_redisProcess(nullptr)
    , m_isInstalled(false)
    , m_isRunning(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_redisProcess = new QProcess(this);
    
    connect(m_redisProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RedisManager::onRedisProcessFinished);
    connect(m_redisProcess, &QProcess::errorOccurred,
            this, &RedisManager::onRedisProcessError);
    connect(m_redisProcess, &QProcess::readyReadStandardOutput,
            this, [this]() {
                QString output = QString::fromUtf8(m_redisProcess->readAllStandardOutput());
                if (!output.isEmpty()) {
                    qDebug() << "[Redis Output]" << output.trimmed();
                }
            });
    connect(m_redisProcess, &QProcess::readyReadStandardError,
            this, [this]() {
                QString error = QString::fromUtf8(m_redisProcess->readAllStandardError());
                if (!error.isEmpty()) {
                    qDebug() << "[Redis Error]" << error.trimmed();
                }
            });
    
    // 检查 Redis 是否已安装
    m_redisPath = getDefaultInstallPath();
    m_redisConfigPath = m_redisPath + "/redis.conf";
    
#ifdef Q_OS_WIN
    QString redisExe = m_redisPath + "/redis-server.exe";
#else
    QString redisExe = m_redisPath + "/redis-server";
#endif
    
    m_isInstalled = QFile::exists(redisExe);
}

RedisManager::~RedisManager()
{
    if (m_isRunning) {
        stopRedis();
    }
    
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
    }
}

bool RedisManager::isRedisInstalled() const
{
    return m_isInstalled;
}

QString RedisManager::getDefaultInstallPath() const
{
#ifdef Q_OS_WIN
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return appData + "/Redis";
#else
    return QDir::homePath() + "/.local/share/RedisInstall/redis";
#endif
}

QString RedisManager::getRedisDownloadUrl() const
{
#ifdef Q_OS_WIN
    // Windows Redis download URL
    return "https://github.com/tporadowski/redis/releases/download/v5.0.14.1/Redis-x64-5.0.14.1.zip";
#else
    // Linux - compile from source or use package manager
    return "https://download.redis.io/redis-stable.tar.gz";
#endif
}

void RedisManager::downloadRedis(const QString& version)
{
    QString url = getRedisDownloadUrl();
    
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, 
                        QNetworkRequest::NoLessSafeRedirectPolicy);
    
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
    }
    
    m_downloadReply = m_networkManager->get(request);
    
    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &RedisManager::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &RedisManager::onDownloadFinished);
    
    emit installationProgress("正在下载 Redis...");
}

void RedisManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void RedisManager::onDownloadFinished()
{
    if (!m_downloadReply) {
        return;
    }
    
    if (m_downloadReply->error() != QNetworkReply::NoError) {
        m_lastError = "下载失败: " + m_downloadReply->errorString();
        emit errorOccurred(m_lastError);
        emit downloadFinished(false);
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
        return;
    }
    
    // 保存下载文件
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#ifdef Q_OS_WIN
    m_downloadedFilePath = tempDir + "/redis.zip";
#else
    m_downloadedFilePath = tempDir + "/redis.tar.gz";
#endif
    
    QFile file(m_downloadedFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "无法保存下载文件";
        emit errorOccurred(m_lastError);
        emit downloadFinished(false);
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
        return;
    }
    
    file.write(m_downloadReply->readAll());
    file.close();
    
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;
    
    emit downloadFinished(true);
    emit installationProgress("下载完成，开始安装...");
    
    // 下载后自动安装
    installRedis(m_redisPath);
}

void RedisManager::installRedis(const QString& installPath)
{
    if (m_downloadedFilePath.isEmpty() || !QFile::exists(m_downloadedFilePath)) {
        m_lastError = "未找到下载的 Redis 文件";
        emit errorOccurred(m_lastError);
        emit installationFinished(false);
        return;
    }
    
    // 创建安装目录
    QDir dir;
    if (!dir.mkpath(installPath)) {
        m_lastError = "无法创建安装目录";
        emit errorOccurred(m_lastError);
        emit installationFinished(false);
        return;
    }
    
    m_redisPath = installPath;
    
    emit installationProgress("正在解压文件...");
    
    // 解压压缩包
    if (!extractRedisArchive(m_downloadedFilePath, installPath)) {
        m_lastError = "解压失败";
        emit errorOccurred(m_lastError);
        emit installationFinished(false);
        return;
    }
    
    // 创建默认配置
    emit installationProgress("正在创建配置文件...");
    if (!createRedisConfig("0.0.0.0", 10833)) {
        m_lastError = "创建配置文件失败";
        emit errorOccurred(m_lastError);
        emit installationFinished(false);
        return;
    }
    
    m_isInstalled = true;
    emit installationProgress("安装完成！");
    emit installationFinished(true);
    
    // 清理下载文件
    QFile::remove(m_downloadedFilePath);
    m_downloadedFilePath.clear();
}

bool RedisManager::extractRedisArchive(const QString& archivePath, const QString& destPath)
{
#ifdef Q_OS_WIN
    QProcess process;
    QString command = QString("powershell -command \"Expand-Archive -Path '%1' -DestinationPath '%2' -Force\"")
                         .arg(archivePath)
                         .arg(destPath);
    
    process.start(command);
    process.waitForFinished(60000);
    
    return process.exitCode() == 0;
#else
    // Linux: 解压并编译 Redis
    QProcess process;
    process.setWorkingDirectory(destPath);
    
    process.start("tar", QStringList() << "-xzf" << archivePath);
    if (!process.waitForFinished(120000)) {
        qDebug() << "Tar extraction timeout";
        return false;
    }
    
    if (process.exitCode() != 0) {
        qDebug() << "Tar extraction failed:" << process.readAllStandardError();
        return false;
    }
    
    // 查找 redis 目录
    QDir dir(destPath);
    QStringList redisDirs = dir.entryList(QStringList() << "redis-*", QDir::Dirs);
    if (redisDirs.isEmpty()) {
        qDebug() << "Redis directory not found";
        return false;
    }
    
    QString redisSourceDir = destPath + "/" + redisDirs.first();
    qDebug() << "Redis source directory:" << redisSourceDir;
    
    // 编译 Redis
    emit installationProgress("正在编译 Redis...");
    process.setWorkingDirectory(redisSourceDir);
    process.start("make", QStringList());
    
    if (!process.waitForFinished(300000)) {
        qDebug() << "Make compilation timeout";
        return false;
    }
    
    if (process.exitCode() != 0) {
        qDebug() << "Make compilation failed:" << process.readAllStandardError();
        return false;
    }
    
    // 复制二进制文件
    QString srcDir = redisSourceDir + "/src";
    QFile::copy(srcDir + "/redis-server", destPath + "/redis-server");
    QFile::copy(srcDir + "/redis-cli", destPath + "/redis-cli");
    
    // 设置执行权限
    QFile::setPermissions(destPath + "/redis-server", 
                          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                          QFile::ReadGroup | QFile::ExeGroup |
                          QFile::ReadOther | QFile::ExeOther);
    QFile::setPermissions(destPath + "/redis-cli",
                          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                          QFile::ReadGroup | QFile::ExeGroup |
                          QFile::ReadOther | QFile::ExeOther);
    
    // 清理源代码目录
    QDir(redisSourceDir).removeRecursively();
    
    return true;
#endif
}

bool RedisManager::createRedisConfig(const QString& ip, int port)
{
    m_redisConfigPath = m_redisPath + "/redis.conf";
    
    QFile file(m_redisConfigPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "# Redis Configuration File\n";
    out << "# Generated by RedisInstall\n\n";
    out << "bind " << ip << "\n";
    out << "port " << port << "\n";
    out << "protected-mode yes\n";
    out << "daemonize no\n";
    out << "pidfile redis.pid\n";
    out << "loglevel notice\n";
    out << "logfile \"redis.log\"\n";
    out << "databases 16\n";
    out << "save 900 1\n";
    out << "save 300 10\n";
    out << "save 60 10000\n";
    out << "stop-writes-on-bgsave-error yes\n";
    out << "rdbcompression yes\n";
    out << "rdbchecksum yes\n";
    out << "dbfilename dump.rdb\n";
    out << "dir ./\n";
    out << "maxmemory 256mb\n";
    out << "maxmemory-policy allkeys-lru\n";
    
    file.close();
    return true;
}

bool RedisManager::createRedisConfigWithPassword(const QString& ip, int port, const QString& password)
{
    m_redisConfigPath = m_redisPath + "/redis.conf";
    
    QFile file(m_redisConfigPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "# Redis Configuration File\n";
    out << "# Generated by RedisInstall\n\n";
    out << "bind " << ip << "\n";
    out << "port " << port << "\n";
    out << "protected-mode yes\n";
    
    // 如果提供了密码，则添加
    if (!password.isEmpty()) {
        out << "requirepass " << password << "\n";
    }
    
    out << "daemonize no\n";
    out << "pidfile redis.pid\n";
    out << "loglevel notice\n";
    out << "logfile \"redis.log\"\n";
    out << "databases 16\n";
    out << "save 900 1\n";
    out << "save 300 10\n";
    out << "save 60 10000\n";
    out << "stop-writes-on-bgsave-error yes\n";
    out << "rdbcompression yes\n";
    out << "rdbchecksum yes\n";
    out << "dbfilename dump.rdb\n";
    out << "dir ./\n";
    out << "maxmemory 256mb\n";
    out << "maxmemory-policy allkeys-lru\n";
    
    file.close();
    return true;
}

void RedisManager::updateRedisConfig(const QString& ip, int port, const QString& password)
{
    createRedisConfigWithPassword(ip, port, password);
}

bool RedisManager::startRedis(const QString& ip, int port, const QString& password)
{
    if (m_isRunning) {
        m_lastError = "Redis 已经在运行";
        return false;
    }
    
    if (!m_isInstalled) {
        m_lastError = "Redis 未安装";
        return false;
    }
    
    // 更新配置
    updateRedisConfig(ip, port, password);
    
#ifdef Q_OS_WIN
    QString redisExe = m_redisPath + "/redis-server.exe";
#else
    QString redisExe = m_redisPath + "/redis-server";
#endif
    
    qDebug() << "[RedisManager] Attempting to start Redis:";
    qDebug() << "  - Executable:" << redisExe;
    qDebug() << "  - Config:" << m_redisConfigPath;
    qDebug() << "  - Working Dir:" << m_redisPath;
    
    if (!QFile::exists(redisExe)) {
        m_lastError = "未找到 Redis 可执行文件: " + redisExe;
        qDebug() << "[RedisManager] Error:" << m_lastError;
        return false;
    }
    
    // Linux: 检查文件是否可执行
#ifndef Q_OS_WIN
    QFileInfo fileInfo(redisExe);
    if (!fileInfo.isExecutable()) {
        qDebug() << "[RedisManager] Warning: redis-server is not executable, attempting to set permissions";
        QFile::setPermissions(redisExe, 
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                              QFile::ReadGroup | QFile::ExeGroup |
                              QFile::ReadOther | QFile::ExeOther);
    }
#endif
    
    m_redisProcess->setWorkingDirectory(m_redisPath);
    m_redisProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_redisProcess->start(redisExe, QStringList() << m_redisConfigPath);
    
    if (!m_redisProcess->waitForStarted(5000)) {
        m_lastError = "Redis 启动失败: " + m_redisProcess->errorString();
        qDebug() << "[RedisManager] Error:" << m_lastError;
        qDebug() << "  - Process state:" << m_redisProcess->state();
        qDebug() << "  - Process error:" << m_redisProcess->error();
        qDebug() << "  - Stderr:" << m_redisProcess->readAllStandardError();
        return false;
    }
    
    m_isRunning = true;
    qDebug() << "[RedisManager] Redis started successfully!";
    emit redisStarted();
    return true;
}

bool RedisManager::stopRedis()
{
    if (!m_isRunning) {
        return true;
    }
    
    if (m_redisProcess->state() == QProcess::Running) {
        m_redisProcess->terminate();
        
        if (!m_redisProcess->waitForFinished(5000)) {
            m_redisProcess->kill();
            m_redisProcess->waitForFinished(2000);
        }
    }
    
    // 杀死孤儿进程
    killRedisProcess();
    
    m_isRunning = false;
    emit redisStopped();
    return true;
}

bool RedisManager::killRedisProcess()
{
#ifdef Q_OS_WIN
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            QString processName = QString::fromWCharArray(pe32.szExeFile);
            if (processName.contains("redis-server", Qt::CaseInsensitive)) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    
    CloseHandle(hProcessSnap);
    return true;
#else
    QProcess::execute("pkill", QStringList() << "-9" << "redis-server");
    return true;
#endif
}

bool RedisManager::restartRedis(const QString& ip, int port, const QString& password)
{
    stopRedis();
    QThread::msleep(500);
    return startRedis(ip, port, password);
}

bool RedisManager::isRedisRunning() const
{
    return m_isRunning && m_redisProcess->state() == QProcess::Running;
}

void RedisManager::uninstallRedis()
{
    stopRedis();
    
    QDir dir(m_redisPath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    
    m_isInstalled = false;
}

QString RedisManager::getRedisVersion() const
{
    if (!m_isInstalled) {
        return "未安装";
    }
    
#ifdef Q_OS_WIN
    QString redisExe = m_redisPath + "/redis-server.exe";
#else
    QString redisExe = m_redisPath + "/redis-server";
#endif
    
    QProcess process;
    process.start(redisExe, QStringList() << "--version");
    process.waitForFinished(3000);
    
    QString output = process.readAllStandardOutput();
    return output.trimmed();
}

QString RedisManager::getRedisPath() const
{
    return m_redisPath;
}

void RedisManager::onRedisProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isRunning = false;
    
    if (exitStatus == QProcess::CrashExit) {
        m_lastError = "Redis 进程崩溃";
        emit errorOccurred(m_lastError);
    }
    
    emit redisStopped();
}

void RedisManager::onRedisProcessError(QProcess::ProcessError error)
{
    m_isRunning = false;
    
    switch (error) {
    case QProcess::FailedToStart:
        m_lastError = "Redis 启动失败";
        break;
    case QProcess::Crashed:
        m_lastError = "Redis 进程崩溃";
        break;
    case QProcess::Timedout:
        m_lastError = "Redis 进程超时";
        break;
    default:
        m_lastError = "Redis 进程错误";
        break;
    }
    
    emit errorOccurred(m_lastError);
}
