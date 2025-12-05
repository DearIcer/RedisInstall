#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ServiceManager;

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

private:
    void setupUI();
    void applyModernStyle();
    void updateButtons();
    bool validatePort(int port, QString& errorMsg);

private:
    Ui::MainWindow *ui;
    
    ServiceManager* m_serviceManager;
    QTimer* m_statusTimer;
    
    // UI Components
    QLabel* m_statusLabel;
    QLabel* m_statusIconLabel;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_uninstallButton;
    
    QLineEdit* m_ipEdit;
    QLineEdit* m_portEdit;
    QLabel* m_portStatusLabel;
    QPushButton* m_applyButton;
    
    bool m_isServiceRunning;
};
#endif // MAINWINDOW_H
