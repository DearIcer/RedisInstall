#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "servicemanager.h"
#include "serviceconfig.h"
#include "portchecker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isServiceRunning(false)
{
    ui->setupUi(this);
    
    m_serviceManager = new ServiceManager(this);
    m_statusTimer = new QTimer(this);
    
    setupUI();
    applyModernStyle();
    
    // Update status every 2 seconds
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateServiceStatus);
    m_statusTimer->start(2000);
    
    updateServiceStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("服务管理器");
    setFixedSize(700, 500);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    
    // Header
    QLabel* titleLabel = new QLabel("服务管理器");
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);
    
    // Status Group
    QGroupBox* statusGroup = new QGroupBox("服务状态");
    statusGroup->setObjectName("groupBox");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(15);
    
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
    configLayout->setSpacing(15);
    
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
    
    // Apply button
    m_applyButton = new QPushButton("应用更改");
    m_applyButton->setObjectName("applyButton");
    
    configLayout->addWidget(m_applyButton);
    mainLayout->addWidget(configGroup);
    
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
            font-size: 28px;
            font-weight: bold;
            color: #2c3e50;
            padding: 10px 0;
        }
        
        #groupBox {
            font-size: 14px;
            font-weight: 600;
            color: #2c3e50;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 15px;
            padding-top: 20px;
            background-color: white;
        }
        
        QGroupBox {
            font-size: 14px;
            font-weight: 600;
            color: #2c3e50;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 15px;
            padding-top: 20px;
            background-color: white;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
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
            font-size: 16px;
            font-weight: 600;
            color: #2c3e50;
            margin-top: 5px;
            margin-bottom: 5px;
        }
        
        #inputField {
            padding: 12px 15px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            font-size: 20px;
            background-color: white;
            min-height: 20px;
        }
        
        #inputField:focus {
            border-color: #3498db;
        }
        
        #portStatusLabel {
            font-size: 14px;
            padding: 5px;
        }
        
        QPushButton {
            padding: 10px 20px;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 600;
            min-width: 120px;
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
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::onStartServiceClicked()
{
    if (m_serviceManager->startService("RedisInstall")) {
        QMessageBox::information(this, "成功", "服务启动成功！");
        updateServiceStatus();
    } else {
        QMessageBox::warning(this, "错误", "服务启动失败: " + m_serviceManager->getLastError());
    }
}

void MainWindow::onStopServiceClicked()
{
    if (m_serviceManager->stopService("RedisInstall")) {
        QMessageBox::information(this, "成功", "服务停止成功！");
        updateServiceStatus();
    } else {
        QMessageBox::warning(this, "错误", "服务停止失败: " + m_serviceManager->getLastError());
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
    
    ServiceConfig::instance().setIpAddress(m_ipEdit->text());
    ServiceConfig::instance().setPort(port);
    ServiceConfig::instance().save();
    
    QMessageBox::information(this, "成功", 
                            "配置更新成功！\n\n"
                            "请重启服务使更改生效。");
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
    bool isRunning = m_serviceManager->isServiceRunning("RedisInstall");
    
    if (isRunning != m_isServiceRunning) {
        m_isServiceRunning = isRunning;
        
        if (m_isServiceRunning) {
            m_statusIconLabel->setText("🟢");
            m_statusLabel->setText("服务运行中");
            m_statusLabel->setStyleSheet("color: #27ae60;");
        } else {
            m_statusIconLabel->setText("🔴");
            m_statusLabel->setText("服务已停止");
            m_statusLabel->setStyleSheet("color: #e74c3c;");
        }
        
        updateButtons();
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
