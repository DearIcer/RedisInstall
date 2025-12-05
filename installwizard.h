#ifndef INSTALLWIZARD_H
#define INSTALLWIZARD_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QStackedWidget>

class InstallWizard : public QWidget
{
    Q_OBJECT

public:
    explicit InstallWizard(QWidget *parent = nullptr);
    ~InstallWizard();

signals:
    void installCompleted(const QString& ip, int port, bool autoStart);
    void installCancelled();

private slots:
    void onNextClicked();
    void onBackClicked();
    void onInstallClicked();
    void onCancelClicked();
    void onPortTextChanged(const QString& text);

private:
    void setupUI();
    void createWelcomePage();
    void createConfigPage();
    void createInstallPage();
    void createFinishPage();
    
    void applyModernStyle();
    bool validatePort(int port, QString& errorMsg);
    void updateNavigationButtons();

private:
    QStackedWidget* m_stackedWidget;
    
    // Welcome page
    QWidget* m_welcomePage;
    
    // Config page
    QWidget* m_configPage;
    QLineEdit* m_ipEdit;
    QLineEdit* m_portEdit;
    QLabel* m_portStatusLabel;
    QCheckBox* m_autoStartCheckBox;
    
    // Install page
    QWidget* m_installPage;
    QLabel* m_installStatusLabel;
    
    // Finish page
    QWidget* m_finishPage;
    
    // Navigation buttons
    QPushButton* m_backButton;
    QPushButton* m_nextButton;
    QPushButton* m_cancelButton;
    
    int m_currentPage;
};

#endif // INSTALLWIZARD_H
