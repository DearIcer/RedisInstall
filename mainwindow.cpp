#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "servicemanager.h"
#include "redismanager.h"
#include "serviceconfig.h"
#include "portchecker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QProgressBar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isServiceRunning(false)
{
    ui->setupUi(this);
    
    m_serviceManager = new ServiceManager(this);
    m_redisManager = new RedisManager(this);
    m_statusTimer = new QTimer(this);
    
    setupUI();
    applyModernStyle();
    
    // Connect Redis manager signals
    connect(m_redisManager, &RedisManager::downloadProgress,
            this, &MainWindow::onRedisDownloadProgress);
    connect(m_redisManager, &RedisManager::downloadFinished,
            this, &MainWindow::onRedisDownloadFinished);
    connect(m_redisManager, &RedisManager::installationProgress,
            this, &MainWindow::onRedisInstallationProgress);
    connect(m_redisManager, &RedisManager::installationFinished,
            this, &MainWindow::onRedisInstallationFinished);
    
    // Update status every 2 seconds
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateServiceStatus);
    m_statusTimer->start(2000);
    
    // Immediately check service status on startup
    QTimer::singleShot(100, this, &MainWindow::updateServiceStatus);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("服务管理器");
    
    // Window will be maximized on startup
    // Disable resize but allow minimize and close
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setMinimumSize(800, 600);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);
    
    // Header
    QLabel* titleLabel = new QLabel("服务管理器");
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);
    
    // Status Group
    QGroupBox* statusGroup = new QGroupBox("服务状态");
    statusGroup->setObjectName("groupBox");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(10);
    statusLayout->setContentsMargins(12, 12, 12, 12);
    
    QWidget* statusWidget = new QWidget();
    QHBoxLayout* statusRowLayout = new QHBoxLayout(statusWidget);
    statusRowLayout->setContentsMargins(0, 0, 0, 0);
    
    m_statusIconLabel = new QLabel("⚪");
    m_statusIconLabel->setObjectName("statusIcon");
    
    m_statusLabel = new QLabel("正在检查...");
    m_statusLabel->setObjectName("statusLabel");
    
    statusRowLayout->addWidget(m_statusIconLabel);
    statusRowLayout->addWidget(m_statusLabel);
    statusRowLayout->addStretch();
    
    statusLayout->addWidget(statusWidget);
    
    // Control buttons
    QWidget* controlWidget = new QWidget();
    QHBoxLayout* controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(10);
    
    m_startButton = new QPushButton("▶ 启动服务");
    m_startButton->setObjectName("startButton");
    
    m_stopButton = new QPushButton("⏸ 停止服务");
    m_stopButton->setObjectName("stopButton");
    
    m_uninstallButton = new QPushButton("🗑 卸载");
    m_uninstallButton->setObjectName("uninstallButton");
    
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_uninstallButton);
    controlLayout->addStretch();
    
    statusLayout->addWidget(controlWidget);
    mainLayout->addWidget(statusGroup);
    
    // Configuration Group
    QGroupBox* configGroup = new QGroupBox("配置设置");
    configGroup->setObjectName("groupBox");
    QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
    configLayout->setSpacing(8);
    configLayout->setContentsMargins(12, 12, 12, 12);
    
    // IP Address
    QLabel* ipLabel = new QLabel("IP 地址:");
    ipLabel->setObjectName("fieldLabel");
    m_ipEdit = new QLineEdit();
    m_ipEdit->setObjectName("inputField");
    m_ipEdit->setText(ServiceConfig::instance().getIpAddress());
    
    configLayout->addWidget(ipLabel);
    configLayout->addWidget(m_ipEdit);
    
    // Port
    QLabel* portLabel = new QLabel("端口:");
    portLabel->setObjectName("fieldLabel");
    m_portEdit = new QLineEdit();
    m_portEdit->setObjectName("inputField");
    m_portEdit->setText(QString::number(ServiceConfig::instance().getPort()));
    m_portEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{1,5}"), this));
    
    m_portStatusLabel = new QLabel();
    m_portStatusLabel->setObjectName("portStatusLabel");
    m_portStatusLabel->setWordWrap(true);
    
    configLayout->addWidget(portLabel);
    configLayout->addWidget(m_portEdit);
    configLayout->addWidget(m_portStatusLabel);
    
    // Password
    QLabel* passwordLabel = new QLabel("密码:");
    passwordLabel->setObjectName("fieldLabel");
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setObjectName("inputField");
    m_passwordEdit->setText(ServiceConfig::instance().getPassword());
    m_passwordEdit->setPlaceholderText("留空表示不设置密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    
    QLabel* passwordHintLabel = new QLabel("💡 设置密码可提高安全性");
    passwordHintLabel->setObjectName("hintLabel");
    
    configLayout->addWidget(passwordLabel);
    configLayout->addWidget(m_passwordEdit);
    configLayout->addWidget(passwordHintLabel);
    
    // Apply button
    m_applyButton = new QPushButton("应用更改");
    m_applyButton->setObjectName("applyButton");
    
    configLayout->addWidget(m_applyButton);
    mainLayout->addWidget(configGroup);
    
    // Redis Information Group
    QGroupBox* redisGroup = new QGroupBox("Redis 信息");
    redisGroup->setObjectName("groupBox");
    QVBoxLayout* redisLayout = new QVBoxLayout(redisGroup);
    redisLayout->setSpacing(10);
    redisLayout->setContentsMargins(12, 12, 12, 12);
    
    // Redis version
    QWidget* versionWidget = new QWidget();
    QHBoxLayout* versionLayout = new QHBoxLayout(versionWidget);
    versionLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* versionTitleLabel = new QLabel("版本:");
    versionTitleLabel->setObjectName("fieldLabel");
    
    m_redisVersionLabel = new QLabel(m_redisManager->isRedisInstalled() ? 
                                     m_redisManager->getRedisVersion() : "未安装");
    m_redisVersionLabel->setObjectName("statusLabel");
    
    versionLayout->addWidget(versionTitleLabel);
    versionLayout->addWidget(m_redisVersionLabel);
    versionLayout->addStretch();
    
    redisLayout->addWidget(versionWidget);
    
    // Redis path
    QWidget* pathWidget = new QWidget();
    QVBoxLayout* pathLayout = new QVBoxLayout(pathWidget);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* pathTitleLabel = new QLabel("安装路径:");
    pathTitleLabel->setObjectName("fieldLabel");
    
    m_redisPathLabel = new QLabel(m_redisManager->getRedisPath());
    m_redisPathLabel->setObjectName("pathLabel");
    m_redisPathLabel->setWordWrap(true);
    
    pathLayout->addWidget(pathTitleLabel);
    pathLayout->addWidget(m_redisPathLabel);
    
    redisLayout->addWidget(pathWidget);
    
    // Download/Install section
    if (!m_redisManager->isRedisInstalled()) {
        m_downloadRedisButton = new QPushButton("📥 下载并安装 Redis");
        m_downloadRedisButton->setObjectName("downloadButton");
        redisLayout->addWidget(m_downloadRedisButton);
        
        m_downloadProgressBar = new QProgressBar();
        m_downloadProgressBar->setObjectName("progressBar");
        m_downloadProgressBar->setVisible(false);
        redisLayout->addWidget(m_downloadProgressBar);
        
        m_installStatusLabel = new QLabel();
        m_installStatusLabel->setObjectName("installStatusLabel");
        m_installStatusLabel->setWordWrap(true);
        m_installStatusLabel->setVisible(false);
        redisLayout->addWidget(m_installStatusLabel);
        
        connect(m_downloadRedisButton, &QPushButton::clicked, 
                this, &MainWindow::onDownloadRedisClicked);
    } else {
        m_downloadRedisButton = nullptr;
        m_downloadProgressBar = nullptr;
        m_installStatusLabel = nullptr;
    }
    
    mainLayout->addWidget(redisGroup);
    
    mainLayout->addStretch();
    
    // Connect signals
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartServiceClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopServiceClicked);
    connect(m_uninstallButton, &QPushButton::clicked, this, &MainWindow::onUninstallClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &MainWindow::onApplyConfigClicked);
    connect(m_portEdit, &QLineEdit::textChanged, this, &MainWindow::onPortTextChanged);
}

void MainWindow::applyModernStyle()
{
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        
        QWidget {
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        
        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
            padding: 5px 0;
            margin-bottom: 5px;
        }
        
        QGroupBox {
            font-size: 14px;
            font-weight: 600;
            color: #2c3e50;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 12px;
            padding: 12px 10px 10px 10px;
            background-color: white;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 5px 15px;
            left: 10px;
            background-color: white;
            color: #2c3e50;
        }
        
        #statusIcon {
            font-size: 24px;
        }
        
        #statusLabel {
            font-size: 16px;
            font-weight: 600;
            color: #2c3e50;
        }
        
        #fieldLabel {
            font-size: 15px;
            font-weight: 600;
            color: #2c3e50;
            margin-top: 5px;
            margin-bottom: 3px;
            min-height: 18px;
        }
        
        #inputField {
            padding: 8px 10px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            font-size: 16px;
            background-color: white;
            min-height: 14px;
        }
        
        #inputField:focus {
            border-color: #3498db;
        }
        
        #portStatusLabel {
            font-size: 14px;
            padding: 5px;
        }
        
        #hintLabel {
            font-size: 12px;
            color: #3498db;
            padding: 5px;
        }
        
        QPushButton {
            padding: 8px 16px;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 600;
            min-width: 100px;
            min-height: 30px;
        }
        
        #startButton {
            background-color: #27ae60;
            color: white;
        }
        
        #startButton:hover {
            background-color: #229954;
        }
        
        #startButton:disabled {
            background-color: #95a5a6;
        }
        
        #stopButton {
            background-color: #e67e22;
            color: white;
        }
        
        #stopButton:hover {
            background-color: #d35400;
        }
        
        #stopButton:disabled {
            background-color: #95a5a6;
        }
        
        #uninstallButton {
            background-color: #e74c3c;
            color: white;
        }
        
        #uninstallButton:hover {
            background-color: #c0392b;
        }
        
        #applyButton {
            background-color: #3498db;
            color: white;
        }
        
        #applyButton:hover {
            background-color: #2980b9;
        }
        
        #downloadButton {
            background-color: #9b59b6;
            color: white;
            padding: 12px 20px;
        }
        
        #downloadButton:hover {
            background-color: #8e44ad;
        }
        
        #downloadButton:disabled {
            background-color: #95a5a6;
        }
        
        #progressBar {
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            text-align: center;
        }
        
        #progressBar::chunk {
            background-color: #3498db;
            border-radius: 4px;
        }
        
        #pathLabel {
            font-size: 12px;
            color: #7f8c8d;
            padding: 5px;
        }
        
        #installStatusLabel {
            font-size: 13px;
            color: #27ae60;
            padding: 5px;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::onStartServiceClicked()
{
    if (!m_redisManager->isRedisInstalled()) {
        QMessageBox::warning(this, "错误", "Redis 尚未安装，请先下载并安装 Redis。");
        return;
    }
    
    QString ip = m_ipEdit->text();
    int port = m_portEdit->text().toInt();
    QString password = m_passwordEdit->text();
    
    if (m_redisManager->startRedis(ip, port, password)) {
        QMessageBox::information(this, "成功", "Redis 服务启动成功！");
        updateServiceStatus();
    } else {
        QMessageBox::warning(this, "错误", "Redis 启动失败: " + m_redisManager->getLastError());
    }
}

void MainWindow::onStopServiceClicked()
{
    if (m_redisManager->stopRedis()) {
        QMessageBox::information(this, "成功", "Redis 服务停止成功！");
        updateServiceStatus();
    } else {
        QMessageBox::warning(this, "错误", "Redis 停止失败: " + m_redisManager->getLastError());
    }
}

void MainWindow::onUninstallClicked()
{
    auto reply = QMessageBox::question(this, "确认卸载",
                                      "您确定要卸载服务吗？",
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_serviceManager->isServiceRunning("RedisInstall")) {
            m_serviceManager->stopService("RedisInstall");
        }
        
        if (m_serviceManager->uninstallService("RedisInstall")) {
            ServiceConfig::instance().setServiceInstalled(false);
            ServiceConfig::instance().save();
            
            QMessageBox::information(this, "成功", "服务卸载成功！");
            close();
        } else {
            QMessageBox::warning(this, "错误", "服务卸载失败: " + m_serviceManager->getLastError());
        }
    }
}

void MainWindow::onApplyConfigClicked()
{
    QString errorMsg;
    int port = m_portEdit->text().toInt();
    
    if (!validatePort(port, errorMsg)) {
        QMessageBox::warning(this, "端口无效", errorMsg);
        return;
    }
    
    QString ip = m_ipEdit->text();
    QString password = m_passwordEdit->text();
    
    ServiceConfig::instance().setIpAddress(ip);
    ServiceConfig::instance().setPort(port);
    ServiceConfig::instance().setPassword(password);
    ServiceConfig::instance().save();
    
    // Update Redis config
    if (m_redisManager->isRedisInstalled()) {
        m_redisManager->updateRedisConfig(ip, port, password);
    }
    
    QMessageBox::information(this, "成功", 
                            "配置更新成功！\n\n"
                            "请重启 Redis 服务使更改生效。");
}

void MainWindow::onPortTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        m_portStatusLabel->setText("");
        return;
    }

    int port = text.toInt();
    QString errorMsg;
    
    if (validatePort(port, errorMsg)) {
        m_portStatusLabel->setStyleSheet("color: #27ae60;");
        m_portStatusLabel->setText("✓ 端口可用");
    } else {
        m_portStatusLabel->setStyleSheet("color: #e74c3c;");
        m_portStatusLabel->setText("✗ " + errorMsg);
    }
}

void MainWindow::updateServiceStatus()
{
    bool isRunning = m_redisManager->isRedisRunning();
    
    qDebug() << "[MainWindow] Checking service status:" << (isRunning ? "Running" : "Stopped");
    
    // Always update UI, not just when status changes
    m_isServiceRunning = isRunning;
    
    if (m_isServiceRunning) {
        m_statusIconLabel->setText("🟢");
        m_statusLabel->setText("Redis 运行中");
        m_statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
        qDebug() << "[MainWindow] Service status: RUNNING";
    } else {
        m_statusIconLabel->setText("🔴");
        m_statusLabel->setText("Redis 已停止");
        m_statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
        qDebug() << "[MainWindow] Service status: STOPPED";
    }
    
    updateButtons();
}

void MainWindow::onDownloadRedisClicked()
{
    if (m_downloadRedisButton) {
        m_downloadRedisButton->setEnabled(false);
        m_downloadProgressBar->setVisible(true);
        m_installStatusLabel->setVisible(true);
    }
    
    m_redisManager->downloadRedis();
}

void MainWindow::onRedisDownloadProgress(qint64 received, qint64 total)
{
    if (m_downloadProgressBar && total > 0) {
        int progress = (int)((received * 100) / total);
        m_downloadProgressBar->setValue(progress);
    }
}

void MainWindow::onRedisDownloadFinished(bool success)
{
    if (!success) {
        if (m_downloadRedisButton) {
            m_downloadRedisButton->setEnabled(true);
            m_downloadProgressBar->setVisible(false);
            m_installStatusLabel->setVisible(false);
        }
        QMessageBox::warning(this, "错误", "Redis 下载失败: " + m_redisManager->getLastError());
    }
}

void MainWindow::onRedisInstallationProgress(const QString& message)
{
    if (m_installStatusLabel) {
        m_installStatusLabel->setText(message);
    }
}

void MainWindow::onRedisInstallationFinished(bool success)
{
    if (success) {
        QMessageBox::information(this, "成功", "Redis 安装成功！\n\n现在可以启动 Redis 服务了。");
        
        // Refresh UI
        if (m_redisVersionLabel) {
            m_redisVersionLabel->setText(m_redisManager->getRedisVersion());
        }
        if (m_redisPathLabel) {
            m_redisPathLabel->setText(m_redisManager->getRedisPath());
        }
        
        // Hide download section
        if (m_downloadRedisButton) {
            m_downloadRedisButton->setVisible(false);
        }
        if (m_downloadProgressBar) {
            m_downloadProgressBar->setVisible(false);
        }
        if (m_installStatusLabel) {
            m_installStatusLabel->setVisible(false);
        }
    } else {
        QMessageBox::warning(this, "错误", "Redis 安装失败: " + m_redisManager->getLastError());
        
        if (m_downloadRedisButton) {
            m_downloadRedisButton->setEnabled(true);
            m_downloadProgressBar->setVisible(false);
            m_installStatusLabel->setVisible(false);
        }
    }
}

void MainWindow::updateButtons()
{
    m_startButton->setEnabled(!m_isServiceRunning);
    m_stopButton->setEnabled(m_isServiceRunning);
}

bool MainWindow::validatePort(int port, QString& errorMsg)
{
    if (port < 1 || port > 65535) {
        errorMsg = "端口必须在 1 到 65535 之间";
        return false;
    }

    int currentPort = ServiceConfig::instance().getPort();
    if (port == currentPort) {
        return true; // Same port as current, no need to check
    }

    PortInfo info = PortChecker::checkPort(port);
    if (info.isInUse) {
        errorMsg = QString("端口已被占用，占用进程: %1 (PID: %2)")
                      .arg(info.processName)
                      .arg(info.processId);
        return false;
    }

    return true;
}
