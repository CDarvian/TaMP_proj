#include "reg_and_auth.h"
#include "ui_reg_and_auth.h"
#include "client.h"
#include "mainwindow.h"

RegAndAuth::RegAndAuth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegAndAuth),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);

    // Изначально показываем только форму входа, скрываем регистрацию
    ui->groupRegister->setVisible(false);
    ui->groupLogin->setVisible(true);
    ui->showLoginButton->setVisible(false);

    // Подключаемся к сигналу от Client, чтобы получать ответы от сервера
    connect(Client::getInstance(), &Client::msgFromServer,
            this, &RegAndAuth::onServerResponse);
}

RegAndAuth::~RegAndAuth()
{
    delete ui;
}

void RegAndAuth::setmainwindow(mainwindow *w)
{
    m_mainWindow = w;
}

// Показать форму регистрации, скрыть форму входа
void RegAndAuth::on_showRegisterButton_clicked()
{
    ui->groupLogin->setVisible(false);
    ui->groupRegister->setVisible(true);
    ui->showRegisterButton->setVisible(false);
    ui->showLoginButton->setVisible(true);
    // Очистим прошлые результаты
    ui->regResultLabel->clear();
    ui->logResultLabel->clear();
    ui->statsResultLabel->clear();
}

// Показать форму входа, скрыть форму регистрации
void RegAndAuth::on_showLoginButton_clicked()
{
    ui->groupRegister->setVisible(false);
    ui->groupLogin->setVisible(true);
    ui->showLoginButton->setVisible(false);
    ui->showRegisterButton->setVisible(true);
    // Очистим прошлые результаты
    ui->regResultLabel->clear();
    ui->logResultLabel->clear();
    ui->statsResultLabel->clear();
}

// Обработчик кнопки «Зарегистрироваться»
void RegAndAuth::on_regButton_clicked()
{
    QString login = ui->regLoginEdit->text().trimmed();
    QString pass  = ui->regPassEdit->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        ui->regResultLabel->setText("Заполните login и password");
        return;
    }
    // Формируем команду: reg&login,password
    QString cmd = QString("reg&%1,%2").arg(login, pass);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->regResultLabel->setText("Ожидаем ответ...");
}

// Обработчик кнопки «Войти»
void RegAndAuth::on_logButton_clicked()
{
    QString login = ui->logLoginEdit->text().trimmed();
    QString pass  = ui->logPassEdit->text().trimmed();
    if (login.isEmpty() || pass.isEmpty()) {
        ui->logResultLabel->setText("Заполните login и password");
        return;
    }
    // Формируем команду: auth&login,password
    QString cmd = QString("auth&%1,%2").arg(login, pass);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->logResultLabel->setText("Ожидаем ответ...");
}

// Обработчик кнопки «Статистика»
void RegAndAuth::on_statsButton_clicked()
{
    Client::getInstance()->sendMsgToServer("stats");
    ui->statsResultLabel->setText("Ожидаем ответ...");
}

// Приём ответов от сервера и вывод в соответствующие QLabel
void RegAndAuth::onServerResponse(const QString &response)
{
    // Ответы для регистрации
    if (response.startsWith("REGISTER_OK") || response.startsWith("REGISTER_ERR")) {
        ui->regResultLabel->setText(response);
    }
    // Ответы для авторизации
    else if (response.startsWith("AUTH_OK")) {
        ui->logResultLabel->setText(response);
        if (m_mainWindow) {
            this->hide();
            m_mainWindow->show();
        }
    }
    else if (response.startsWith("AUTH_ERR")) {
        ui->logResultLabel->setText(response);
    }
    // Ответы для статистики
    else if (response.startsWith("STATS")) {
        ui->statsResultLabel->setText(response);
    }
    else {
        // Любые другие ответы (эхо и т.д.)
    }
}
