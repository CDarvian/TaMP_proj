#include "MainWindow.h"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_socket(new QTcpSocket(this)),
    m_connected(false)
{
    setupUI();

    connect(m_socket, &QTcpSocket::disconnected, [this]() {
        m_connected = false;
        m_statusLabel->setText("Status: Disconnected");
        m_connectButton->setText("Connect");
    });
}

MainWindow::~MainWindow() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void MainWindow::setupUI() {
    // Здесь создаём главный контейнер, который будет прокручиваться
    QWidget *container = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(container);

    // 1) Кнопка Connect + статус
    m_connectButton = new QPushButton("Connect", this);
    m_statusLabel   = new QLabel("Status: Disconnected", this);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_connectButton);
    topLayout->addWidget(m_statusLabel);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 2) Registration Group
    QGroupBox *regGroup = new QGroupBox("Registration", this);
    m_regLoginEdit   = new QLineEdit(this);
    m_regPassEdit    = new QLineEdit(this);
    m_regPassEdit->setEchoMode(QLineEdit::Password);
    m_regButton      = new QPushButton("Register", this);
    m_regResultLabel = new QLabel(this);

    QGridLayout *regLayout = new QGridLayout;
    regLayout->addWidget(new QLabel("Login:"), 0, 0);
    regLayout->addWidget(m_regLoginEdit, 0, 1);
    regLayout->addWidget(new QLabel("Password:"), 1, 0);
    regLayout->addWidget(m_regPassEdit, 1, 1);
    regLayout->addWidget(m_regButton, 2, 0, 1, 2);
    regLayout->addWidget(m_regResultLabel, 3, 0, 1, 2);
    regGroup->setLayout(regLayout);

    mainLayout->addWidget(regGroup);

    // 3) Login Group
    QGroupBox *logGroup = new QGroupBox("Login", this);
    m_logLoginEdit   = new QLineEdit(this);
    m_logPassEdit    = new QLineEdit(this);
    m_logPassEdit->setEchoMode(QLineEdit::Password);
    m_logButton      = new QPushButton("Login", this);
    m_logResultLabel = new QLabel(this);

    QGridLayout *logLayout = new QGridLayout;
    logLayout->addWidget(new QLabel("Login:"), 0, 0);
    logLayout->addWidget(m_logLoginEdit, 0, 1);
    logLayout->addWidget(new QLabel("Password:"), 1, 0);
    logLayout->addWidget(m_logPassEdit, 1, 1);
    logLayout->addWidget(m_logButton, 2, 0, 1, 2);
    logLayout->addWidget(m_logResultLabel, 3, 0, 1, 2);
    logGroup->setLayout(logLayout);

    mainLayout->addWidget(logGroup);

    // 4) Stats Group
    QGroupBox *statsGroup = new QGroupBox("Statistics", this);
    m_statsButton      = new QPushButton("Get Stats", this);
    m_statsResultLabel = new QLabel(this);

    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->addWidget(m_statsButton);
    statsLayout->addWidget(m_statsResultLabel);
    statsGroup->setLayout(statsLayout);

    mainLayout->addWidget(statsGroup);

    // 5) SHA-512 Group
    QGroupBox *shaGroup = new QGroupBox("SHA-512", this);
    m_shaInputEdit   = new QLineEdit(this);
    m_shaButton      = new QPushButton("Hash", this);
    m_shaResultLabel = new QLabel(this);

    QGridLayout *shaLayout = new QGridLayout;
    shaLayout->addWidget(new QLabel("Input:"), 0, 0);
    shaLayout->addWidget(m_shaInputEdit, 0, 1);
    shaLayout->addWidget(m_shaButton, 1, 0, 1, 2);
    shaLayout->addWidget(m_shaResultLabel, 2, 0, 1, 2);
    shaGroup->setLayout(shaLayout);

    mainLayout->addWidget(shaGroup);

    // 6) Division Group
    QGroupBox *divGroup = new QGroupBox("Division (sqrt)", this);
    m_divInputEdit   = new QLineEdit(this);
    m_divButton      = new QPushButton("Compute", this);
    m_divResultLabel = new QLabel(this);

    QGridLayout *divLayout = new QGridLayout;
    divLayout->addWidget(new QLabel("Value:"), 0, 0);
    divLayout->addWidget(m_divInputEdit, 0, 1);
    divLayout->addWidget(m_divButton, 1, 0, 1, 2);
    divLayout->addWidget(m_divResultLabel, 2, 0, 1, 2);
    divGroup->setLayout(divLayout);

    mainLayout->addWidget(divGroup);

    // 7) Graph Group
    QGroupBox *graphGroup = new QGroupBox("Graph Shortest Path", this);
    m_graphSEdit       = new QLineEdit(this);
    m_graphTEdit       = new QLineEdit(this);
    m_graphButton      = new QPushButton("Compute Path", this);
    m_graphResultLabel = new QLabel(this);

    QGridLayout *graphLayout = new QGridLayout;
    graphLayout->addWidget(new QLabel("Source:"), 0, 0);
    graphLayout->addWidget(m_graphSEdit, 0, 1);
    graphLayout->addWidget(new QLabel("Target:"), 1, 0);
    graphLayout->addWidget(m_graphTEdit, 1, 1);
    graphLayout->addWidget(m_graphButton, 2, 0, 1, 2);
    graphLayout->addWidget(m_graphResultLabel, 3, 0, 1, 2);
    graphGroup->setLayout(graphLayout);

    mainLayout->addWidget(graphGroup);

    mainLayout->addStretch(); // чтобы контент не слипался сверху

    // Оборачиваем container в QScrollArea
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setWidget(container);

    setCentralWidget(scroll);
    setWindowTitle("TCP Client GUI");
    resize(500, 700);

    // Сигналы/слоты
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_regButton,     &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(m_logButton,     &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_statsButton,   &QPushButton::clicked, this, &MainWindow::onStatsClicked);
    connect(m_shaButton,     &QPushButton::clicked, this, &MainWindow::onSHA512Clicked);
    connect(m_divButton,     &QPushButton::clicked, this, &MainWindow::onDivisionClicked);
    connect(m_graphButton,   &QPushButton::clicked, this, &MainWindow::onGraphClicked);
}

QString MainWindow::sendCommand(const QString &cmd) {
    if (!m_connected) return "ERROR: Not connected";
    QString message = cmd + "\n";
    m_socket->write(message.toUtf8());
    m_socket->flush();

    if (!m_socket->waitForReadyRead(2000)) {
        return "ERROR: No response";
    }
    QByteArray response = m_socket->readAll().trimmed();
    return QString::fromUtf8(response);
}

void MainWindow::onConnectClicked() {
    if (m_connected) {
        m_socket->disconnectFromHost();
        return;
    }
    m_socket->connectToHost(QHostAddress::LocalHost, 33333);
    if (!m_socket->waitForConnected(2000)) {
        QMessageBox::warning(this, "Connection", "Failed to connect: " + m_socket->errorString());
        return;
    }
    m_connected = true;
    m_statusLabel->setText("Status: Connected");
    m_connectButton->setText("Disconnect");
}

void MainWindow::onRegisterClicked() {
    QString login = m_regLoginEdit->text().trimmed();
    QString pass  = m_regPassEdit->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        m_regResultLabel->setText("Fill login and password");
        return;
    }
    QString cmd = "reg&" + login + "," + pass;
    QString resp = sendCommand(cmd);
    m_regResultLabel->setText(resp);
}

void MainWindow::onLoginClicked() {
    QString login = m_logLoginEdit->text().trimmed();
    QString pass  = m_logPassEdit->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        m_logResultLabel->setText("Fill login and password");
        return;
    }
    QString cmd = "auth&" + login + "," + pass;
    QString resp = sendCommand(cmd);
    m_logResultLabel->setText(resp);
}

void MainWindow::onStatsClicked() {
    QString resp = sendCommand("stats");
    m_statsResultLabel->setText(resp);
}

void MainWindow::onSHA512Clicked() {
    QString text = m_shaInputEdit->text();
    if (text.isEmpty()) {
        m_shaResultLabel->setText("Enter text");
        return;
    }
    QString cmd = "sha512&" + text;
    QString resp = sendCommand(cmd);
    m_shaResultLabel->setText(resp);
}

void MainWindow::onDivisionClicked() {
    QString val = m_divInputEdit->text().trimmed();
    if (val.isEmpty()) {
        m_divResultLabel->setText("Enter a value");
        return;
    }
    QString cmd = "division&" + val;
    QString resp = sendCommand(cmd);
    m_divResultLabel->setText(resp);
}

void MainWindow::onGraphClicked() {
    QString s = m_graphSEdit->text().trimmed();
    QString t = m_graphTEdit->text().trimmed();
    if (s.isEmpty() || t.isEmpty()) {
        m_graphResultLabel->setText("Enter source and target");
        return;
    }
    QString cmd = "graph&" + s + "," + t;
    QString resp = sendCommand(cmd);
    m_graphResultLabel->setText(resp);
}
