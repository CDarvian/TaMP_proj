#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "reg_and_auth.h"
#include <QMenu>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

mainwindow::mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainwindow)
{
    ui->setupUi(this);
    ui->adminButton->setVisible(Client::getInstance()->isAdmin());



    connect(ui->backButton, &QPushButton::clicked, this, &mainwindow::on_backButton_clicked);
    connect(Client::getInstance(), &Client::msgFromServer, this, &mainwindow::onServerResponse);
    connect(Client::getInstance(), &Client::adminStatusChanged, this, [this](bool isAdmin) {
        ui->adminButton->setVisible(isAdmin);
    });
}
mainwindow::~mainwindow()
{
    delete ui;
}

void mainwindow::on_backButton_clicked()
{
    this->hide();
    foreach (QWidget *w, QApplication::topLevelWidgets()) {
        // Делаем dynamic_cast к RegAndAuth*
        RegAndAuth *ra = qobject_cast<RegAndAuth*>(w);
        if (ra) {
            ra->show();
            break;
        }
    }
}

void mainwindow::on_shaButton_clicked()
{
    QString text = ui->shaInputEdit->text().trimmed();
    if (text.isEmpty()) {
        ui->shaResultLabel->setText("Enter text");
        return;
    }
    QString cmd = QString("sha512&%1").arg(text);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->shaResultLabel->setText("Waiting response...");
}

void mainwindow::on_divButton_clicked()
{
    QString val = ui->divInputEdit->text().trimmed();
    if (val.isEmpty()) {
        ui->divResultLabel->setText("Enter value");
        return;
    }
    QString cmd = QString("division&%1").arg(val);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->divResultLabel->setText("Waiting response...");
}

void mainwindow::on_graphButton_clicked()
{
    QString s = ui->graphSEdit->text().trimmed();
    QString t = ui->graphTEdit->text().trimmed();
    if (s.isEmpty() || t.isEmpty()) {
        ui->graphResultLabel->setText("Enter source & target");
        return;
    }
    QString cmd = QString("graph&%1,%2").arg(s, t);
    Client::getInstance()->sendMsgToServer(cmd);
    ui->graphResultLabel->setText("Waiting response...");
}

void mainwindow::onServerResponse(const QString &response) {
    qDebug() << "Server response:" << response; // Логирование для отладки

    if (response.startsWith("SHA512")) {
        ui->shaResultLabel->setText(response);
    }
    else if (response.startsWith("DIV_RESULT") || response.startsWith("DIV_ERR")) {
        ui->divResultLabel->setText(response);
    }
    else if (response.startsWith("GRAPH") || response.startsWith("GRAPH_ERR")) {
        ui->graphResultLabel->setText(response);
    }
    else if (response.startsWith("ADMIN_AUTH_OK")) {
        Client::getInstance()->setAdmin(true);
        QMessageBox::information(this, "Admin", "Admin authentication successful");
    }
    else if (response.startsWith("ADMIN_USERS")) {
        QString formatted = response.mid(12); // Пропускаем "ADMIN_USERS:"
        formatted.replace("\t", " | "); // Форматируем разделители
        formatted.replace("\r\n", "\n"); // Заменяем переносы строк
        QMessageBox::information(this, "Users List", formatted);
    }
    else if (response.startsWith("ADMIN_OK")) {
        QMessageBox::information(this, "Success", response.mid(9));
    }
    else if (response.startsWith("ADMIN_ERR")) {
        QMessageBox::warning(this, "Error", response.mid(10));
    }
    else if (response.startsWith("STATS")) {
        ui->outputLabel->setText(response);
    }
    else {
        ui->outputLabel->setText(response);
    }
}

// Добавим кнопку для администраторских функций
void mainwindow::on_adminButton_clicked()
{
    QMenu adminMenu;
    adminMenu.setTitle("Admin Panel");

    QAction *usersAction = adminMenu.addAction("List Users");
    QAction *statusAction = adminMenu.addAction("Change User Status");
    QAction *logoutAction = adminMenu.addAction("Logout as Admin");

    QAction *selected = adminMenu.exec(QCursor::pos());

    if (selected == usersAction) {
        Client::getInstance()->sendMsgToServer("admin_users&");
    }
    else if (selected == statusAction) {
        bool ok;
        QString login = QInputDialog::getText(this, "Change User Status",
                                              "Enter user login:", QLineEdit::Normal, "", &ok);
        if (!ok || login.isEmpty()) return;

        QStringList statuses = {"online", "offline", "banned", "admin"};
        QString status = QInputDialog::getItem(this, "Change Status",
                                               "Select new status:", statuses, 0, false, &ok);
        if (!ok || status.isEmpty()) return;

        Client::getInstance()->sendMsgToServer(QString("admin_setstatus&%1,%2").arg(login, status));
    }
    else if (selected == logoutAction) {
        Client::getInstance()->setAdmin(false);
        QMessageBox::information(this, "Admin", "Logged out from admin panel");
    }
}


void mainwindow::updateAdminUI(bool isAdmin) {
    ui->adminButton->setVisible(isAdmin);
}
