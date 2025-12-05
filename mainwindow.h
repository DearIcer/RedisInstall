#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QProgressBar>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ServiceManager;
class RedisManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartServiceClicked();
    void onStopServiceClicked();
    void onUninstallClicked();
    void onApplyConfigClicked();
    void onPortTextChanged(const QString& text);
    void updateServiceStatus();
    
    // Redis management slots
    void onDownloadRedisClicked();
    void onRedisDownloadProgress(qint64 received, qint64 total);
    void onRedisDownloadFinished(bool success);
    void onRedisInstallationProgress(const QString& message);
    void onRedisInstallationFinished(bool success);

private:
    void setupUI();
    void applyModernStyle();
    void updateButtons();
    bool validatePort(int port, QString& errorMsg);

private:
    Ui::MainWindow *ui;
    
    ServiceManager* m_serviceManager;
    RedisManager* m_redisManager;
    QTimer* m_statusTimer;
    
    // UI Components
    QLabel* m_statusLabel;
    QLabel* m_statusIconLabel;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_uninstallButton;
    QPushButton* m_downloadRedisButton;
    
    QLineEdit* m_ipEdit;
    QLineEdit* m_portEdit;
    QLineEdit* m_passwordEdit;
    QLabel* m_portStatusLabel;
    QPushButton* m_applyButton;
    
    QLabel* m_redisVersionLabel;
    QLabel* m_redisPathLabel;
    QProgressBar* m_downloadProgressBar;
    QLabel* m_installStatusLabel;
    
    bool m_isServiceRunning;
};
#endif // MAINWINDOW_H
