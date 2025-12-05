#include "installwizard.h"
#include "portchecker.h"
#include "serviceconfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QGraphicsDropShadowEffect>

InstallWizard::InstallWizard(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(0)
{
    setupUI();
    applyModernStyle();
}

InstallWizard::~InstallWizard()
{
}

void InstallWizard::setupUI()
{
    setWindowTitle("服务安装向导");
    setFixedSize(600, 450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Stacked widget for pages
    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);

    createWelcomePage();
    createConfigPage();
    createInstallPage();
    createFinishPage();

    // Navigation buttons
    QWidget* navigationWidget = new QWidget(this);
    navigationWidget->setObjectName("navigationWidget");
    QHBoxLayout* navLayout = new QHBoxLayout(navigationWidget);
    navLayout->setContentsMargins(20, 15, 20, 15);

    m_cancelButton = new QPushButton("取消", navigationWidget);
    m_backButton = new QPushButton("← 上一步", navigationWidget);
    m_nextButton = new QPushButton("下一步 →", navigationWidget);

    m_cancelButton->setObjectName("cancelButton");
    m_backButton->setObjectName("backButton");
    m_nextButton->setObjectName("nextButton");

    navLayout->addWidget(m_cancelButton);
    navLayout->addStretch();
    navLayout->addWidget(m_backButton);
    navLayout->addWidget(m_nextButton);

    mainLayout->addWidget(navigationWidget);

    connect(m_backButton, &QPushButton::clicked, this, &InstallWizard::onBackClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &InstallWizard::onNextClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &InstallWizard::onCancelClicked);

    updateNavigationButtons();
}

void InstallWizard::createWelcomePage()
{
    m_welcomePage = new QWidget();
    m_welcomePage->setObjectName("welcomePage");

    QVBoxLayout* layout = new QVBoxLayout(m_welcomePage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel* titleLabel = new QLabel("欢迎使用服务安装程序");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* iconLabel = new QLabel("🚀");
    iconLabel->setObjectName("iconLabel");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* descLabel = new QLabel(
        "此向导将引导您完成安装过程。\n\n"
        "您将能够：\n"
        "• 配置 IP 地址和端口\n"
        "• 设置开机自动启动\n"
        "• 无需管理员权限即可安装服务"
    );
    descLabel->setObjectName("descLabel");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignLeft);

    layout->addStretch();
    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addStretch();

    m_stackedWidget->addWidget(m_welcomePage);
}

void InstallWizard::createConfigPage()
{
    m_configPage = new QWidget();
    m_configPage->setObjectName("configPage");

    QVBoxLayout* layout = new QVBoxLayout(m_configPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel* titleLabel = new QLabel("配置设置");
    titleLabel->setObjectName("titleLabel");

    // IP Address
    QLabel* ipLabel = new QLabel("IP 地址:");
    ipLabel->setObjectName("fieldLabel");
    m_ipEdit = new QLineEdit();
    m_ipEdit->setObjectName("inputField");
    m_ipEdit->setText("0.0.0.0");
    m_ipEdit->setPlaceholderText("请输入 IP 地址 (例如: 0.0.0.0)");

    // Port
    QLabel* portLabel = new QLabel("端口:");
    portLabel->setObjectName("fieldLabel");
    m_portEdit = new QLineEdit();
    m_portEdit->setObjectName("inputField");
    m_portEdit->setText("10833");
    m_portEdit->setPlaceholderText("请输入端口号");
    m_portEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{1,5}"), this));

    m_portStatusLabel = new QLabel();
    m_portStatusLabel->setObjectName("portStatusLabel");
    m_portStatusLabel->setWordWrap(true);

    // Auto start checkbox
    m_autoStartCheckBox = new QCheckBox("开机自动启动");
    m_autoStartCheckBox->setObjectName("checkBox");
    m_autoStartCheckBox->setChecked(true);

    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(ipLabel);
    layout->addWidget(m_ipEdit);
    layout->addSpacing(10);
    layout->addWidget(portLabel);
    layout->addWidget(m_portEdit);
    layout->addWidget(m_portStatusLabel);
    layout->addSpacing(10);
    layout->addWidget(m_autoStartCheckBox);
    layout->addStretch();

    connect(m_portEdit, &QLineEdit::textChanged, this, &InstallWizard::onPortTextChanged);

    m_stackedWidget->addWidget(m_configPage);
}

void InstallWizard::createInstallPage()
{
    m_installPage = new QWidget();
    m_installPage->setObjectName("installPage");

    QVBoxLayout* layout = new QVBoxLayout(m_installPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel* titleLabel = new QLabel("正在安装...");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* iconLabel = new QLabel("⚙️");
    iconLabel->setObjectName("iconLabel");
    iconLabel->setAlignment(Qt::AlignCenter);

    m_installStatusLabel = new QLabel("正在安装服务，请稍候...");
    m_installStatusLabel->setObjectName("descLabel");
    m_installStatusLabel->setAlignment(Qt::AlignCenter);
    m_installStatusLabel->setWordWrap(true);

    layout->addStretch();
    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(m_installStatusLabel);
    layout->addStretch();

    m_stackedWidget->addWidget(m_installPage);
}

void InstallWizard::createFinishPage()
{
    m_finishPage = new QWidget();
    m_finishPage->setObjectName("finishPage");

    QVBoxLayout* layout = new QVBoxLayout(m_finishPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel* titleLabel = new QLabel("安装完成！");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* iconLabel = new QLabel("✅");
    iconLabel->setObjectName("iconLabel");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* descLabel = new QLabel(
        "服务已成功安装。\n\n"
        "您现在可以通过主界面管理服务。"
    );
    descLabel->setObjectName("descLabel");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);

    layout->addStretch();
    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addStretch();

    m_stackedWidget->addWidget(m_finishPage);
}

void InstallWizard::applyModernStyle()
{
    QString styleSheet = R"(
        QWidget {
            background-color: #f5f5f5;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        
        #welcomePage, #configPage, #installPage, #finishPage {
            background-color: white;
        }
        
        #navigationWidget {
            background-color: #f8f9fa;
            border-top: 1px solid #e0e0e0;
        }
        
        #titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
        }
        
        #iconLabel {
            font-size: 64px;
        }
        
        #descLabel {
            font-size: 14px;
            color: #555;
            line-height: 1.6;
        }
        
        #fieldLabel {
            font-size: 16px;
            font-weight: 600;
            color: #2c3e50;
            margin-top: 5px;
            margin-bottom: 8px;
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
            outline: none;
        }
        
        #portStatusLabel {
            font-size: 14px;
            color: #27ae60;
            padding: 5px;
        }
        
        QPushButton {
            padding: 8px 20px;
            border: none;
            border-radius: 6px;
            font-size: 13px;
            font-weight: 600;
            min-width: 80px;
        }
        
        #nextButton {
            background-color: #3498db;
            color: white;
        }
        
        #nextButton:hover {
            background-color: #2980b9;
        }
        
        #nextButton:pressed {
            background-color: #1f6ca1;
        }
        
        #nextButton:disabled {
            background-color: #bdc3c7;
        }
        
        #backButton {
            background-color: #ecf0f1;
            color: #2c3e50;
        }
        
        #backButton:hover {
            background-color: #d5dbdb;
        }
        
        #backButton:disabled {
            background-color: #ecf0f1;
            color: #95a5a6;
        }
        
        #cancelButton {
            background-color: transparent;
            color: #7f8c8d;
        }
        
        #cancelButton:hover {
            color: #e74c3c;
        }
        
        QCheckBox {
            font-size: 13px;
            color: #2c3e50;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #bdc3c7;
            border-radius: 4px;
            background-color: white;
        }
        
        QCheckBox::indicator:checked {
            background-color: #3498db;
            border-color: #3498db;
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTAiIGhlaWdodD0iOCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMSA0TDMuNSA2LjUgOSAxIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjIiIGZpbGw9Im5vbmUiLz48L3N2Zz4=);
        }
    )";
    
    setStyleSheet(styleSheet);
}

void InstallWizard::onNextClicked()
{
    if (m_currentPage == 0) {
        // Welcome -> Config
        m_currentPage = 1;
        m_stackedWidget->setCurrentIndex(m_currentPage);
    } else if (m_currentPage == 1) {
        // Config -> Install
        QString errorMsg;
        int port = m_portEdit->text().toInt();
        if (!validatePort(port, errorMsg)) {
            QMessageBox::warning(this, "端口无效", errorMsg);
            return;
        }
        
        m_currentPage = 2;
        m_stackedWidget->setCurrentIndex(m_currentPage);
        onInstallClicked();
    } else if (m_currentPage == 3) {
        // Finish -> Close
        emit installCompleted(m_ipEdit->text(), m_portEdit->text().toInt(), 
                            m_autoStartCheckBox->isChecked());
        close();
    }
    
    updateNavigationButtons();
}

void InstallWizard::onBackClicked()
{
    if (m_currentPage > 0 && m_currentPage != 2) {
        m_currentPage--;
        m_stackedWidget->setCurrentIndex(m_currentPage);
        updateNavigationButtons();
    }
}

void InstallWizard::onInstallClicked()
{
    // Save configuration
    ServiceConfig::instance().setIpAddress(m_ipEdit->text());
    ServiceConfig::instance().setPort(m_portEdit->text().toInt());
    ServiceConfig::instance().setAutoStart(m_autoStartCheckBox->isChecked());
    ServiceConfig::instance().setServiceInstalled(true);
    ServiceConfig::instance().save();

    // Simulate installation process
    m_installStatusLabel->setText("安装成功完成！");
    
    // Move to finish page
    m_currentPage = 3;
    m_stackedWidget->setCurrentIndex(m_currentPage);
    updateNavigationButtons();
}

void InstallWizard::onCancelClicked()
{
    emit installCancelled();
    close();
}

void InstallWizard::onPortTextChanged(const QString& text)
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

bool InstallWizard::validatePort(int port, QString& errorMsg)
{
    if (port < 1 || port > 65535) {
        errorMsg = "端口必须在 1 到 65535 之间";
        return false;
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

void InstallWizard::updateNavigationButtons()
{
    m_backButton->setEnabled(m_currentPage > 0 && m_currentPage != 2);
    
    if (m_currentPage == 3) {
        m_nextButton->setText("完成");
        m_cancelButton->setEnabled(false);
    } else if (m_currentPage == 2) {
        m_nextButton->setEnabled(false);
        m_backButton->setEnabled(false);
        m_cancelButton->setEnabled(false);
    } else {
        m_nextButton->setText("下一步 →");
        m_nextButton->setEnabled(true);
        m_cancelButton->setEnabled(true);
    }
}
