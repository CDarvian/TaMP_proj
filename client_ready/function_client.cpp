#include "function_client.h"
#include "ui_function_client.h"
#include "client.h"
#include <QDebug>

function_client::function_client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::function_client)
{
    ui->setupUi(this);

    // По нажатию Back вернёмся назад (скроем это окно и покажем RegAndAuth,
    // который уже должен быть создан и спрятан в main.cpp).
    connect(ui->backButton, &QPushButton::clicked, this, [=]() {
        this->hide();
        // Покажем RegAndAuth (ищем его среди открытых окон)
        foreach (QWidget *w, QApplication::topLevelWidgets()) {
            if (w->inherits("RegAndAuth")) {
                w->show();
                break;
            }
        }
    });

    connect(Client::getInstance(), &Client::msgFromServer,
            this, &function_client::onServerResponse);
}

function_client::~function_client()
{
    delete ui;
}

void function_client::on_shaButton_clicked()
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

void function_client::on_divButton_clicked()
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

void function_client::on_graphButton_clicked()
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

void function_client::onServerResponse(const QString &response)
{
    if (response.startsWith("SHA512")) {
        ui->shaResultLabel->setText(response);
    }
    else if (response.startsWith("DIV_RESULT") || response.startsWith("DIV_ERR")) {
        ui->divResultLabel->setText(response);
    }
    else if (response.startsWith("GRAPH") || response.startsWith("GRAPH_ERR")) {
        ui->graphResultLabel->setText(response);
    }
    else {
        // Всё другое (например, эхо) в outputLabel
        ui->outputLabel->setText(response);
    }
}

