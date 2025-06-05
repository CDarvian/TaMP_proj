#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

class QPushButton;
class QLineEdit;
class QLabel;
class QGroupBox;
class QScrollArea;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onRegisterClicked();
    void onLoginClicked();
    void onStatsClicked();
    void onSHA512Clicked();
    void onDivisionClicked();
    void onGraphClicked();

private:
    QString sendCommand(const QString &cmd);
    void setupUI();

    QTcpSocket *m_socket;
    bool m_connected;

    // Connection
    QPushButton *m_connectButton;
    QLabel      *m_statusLabel;

    // Registration
    QLineEdit   *m_regLoginEdit;
    QLineEdit   *m_regPassEdit;
    QPushButton *m_regButton;
    QLabel      *m_regResultLabel;

    // Login
    QLineEdit   *m_logLoginEdit;
    QLineEdit   *m_logPassEdit;
    QPushButton *m_logButton;
    QLabel      *m_logResultLabel;

    // Stats
    QPushButton *m_statsButton;
    QLabel      *m_statsResultLabel;

    // SHA-512
    QLineEdit   *m_shaInputEdit;
    QPushButton *m_shaButton;
    QLabel      *m_shaResultLabel;

    // Division
    QLineEdit   *m_divInputEdit;
    QPushButton *m_divButton;
    QLabel      *m_divResultLabel;

    // Graph
    QLineEdit   *m_graphSEdit;
    QLineEdit   *m_graphTEdit;
    QPushButton *m_graphButton;
    QLabel      *m_graphResultLabel;
};

#endif // MAINWINDOW_H
