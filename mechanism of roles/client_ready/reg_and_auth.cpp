#include "reg_and_auth.h"
#include "ui_reg_and_auth.h"
#include "mainwindow.h"
#include "client.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QCryptographicHash>


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
    //ui->groupAdmin->setVisible(true); // Всегда показываем кнопку админа

    // Подключаемся к сигналу от Client
    connect(Client::getInstance(), &Client::msgFromServer,
            this, &RegAndAuth::onServerResponse);

    // Подключаем кнопку администратора
    disconnect(ui->adminButton, &QPushButton::clicked, 0, 0); // Сначала отключаем все
    connect(ui->adminButton, &QPushButton::clicked,
            this, &RegAndAuth::on_adminButton_clicked);
}

RegAndAuth::~RegAndAuth()
{
    delete ui;
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
    Client::getInstance()->sendMsgToServer("stats&");
    ui->statsResultLabel->setText("Ожидаем ответ...");
}


// Добавим кнопку для входа администратора
void RegAndAuth::on_adminButton_clicked()
{
    bool ok;
    QString login = QInputDialog::getText(this, tr("Admin Authentication"),
                                          tr("Login:"), QLineEdit::Normal, "", &ok);
    if (!ok || login.isEmpty()) return;

    QString password = QInputDialog::getText(this, tr("Admin Authentication"),
                                             tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;

    // Отправляем пароль в чистом виде
    Client::getInstance()->sendMsgToServer(
        QString("admin_auth&%1,%2").arg(login.trimmed(), password.trimmed())
        );
}

void RegAndAuth::onServerResponse(const QString &response)
{
    if (response.startsWith("ADMIN_AUTH_OK")) {
        Client::getInstance()->setAdmin(true); // Устанавливаем статус администратора

        // Создаём и показываем главное окно
        if (!m_mainWindow) {
            m_mainWindow = new mainwindow();
        }
        this->hide();
        m_mainWindow->show();

        QMessageBox::information(this, tr("Admin"), tr("Admin authentication successful"));
    }
    else if (response.startsWith("ADMIN_AUTH_ERR")) {
        Client::getInstance()->setAdmin(false); // Сбрасываем статус при ошибке
        QMessageBox::warning(this, tr("Admin"), tr("Admin authentication failed"));
    }
    else if (response.startsWith("REGISTER")) {
        ui->regResultLabel->setText(response);
    }
    else if (response.startsWith("AUTH_OK")) {
        if (!m_mainWindow) {
            m_mainWindow = new mainwindow();
            m_mainWindow->updateAdminUI(Client::getInstance()->isAdmin()); // Обновляем интерфейс
        }
        this->hide();
        m_mainWindow->show();
    }
    else if (response.startsWith("AUTH_ERR")) {
        ui->logResultLabel->setText(response);
    }
    else if (response.startsWith("STATS")) {
        ui->statsResultLabel->setText(response);
    }
}
