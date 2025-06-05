
#include "function_client.h"
#include "mainwindow.h"
#include "reg_and_auth.h"
#include "client.h"

function_client::function_client()
{
    client = Client::getInstance();

    connect(client, &Client::msgFromServer, this, &function_client::msgHandler);

    // Симулируем успешную авторизацию как "guest"
    client->sendMsgToServer("auth&guest&guest");
}

void function_client::openMainWindow()
{
    ui_main = new MainWindow;
    ui_main->show();

    connect(ui_main, &MainWindow::RSA, this, &function_client::DecisionRSA);
    connect(ui_main, &MainWindow::hash, this, &function_client::DecisionHash);
    connect(ui_main, &MainWindow::Division, this, &function_client::DecisionDivision);
    connect(ui_main, &MainWindow::Graf, this, &function_client::DecisionGraf);
    connect(this, &function_client::Decision, ui_main, &MainWindow::Decision);
}

void function_client::msgHandler(QString msg)
{
    if (msg == "Successfully auth\r\n" || msg == "Successfully reg\r\n")
    {
        openMainWindow();
    }
    else
    {
        emit Decision(msg);
    }
}

void function_client::DecisionRSA(QString text)
{
    client->sendMsgToServer("rsa&" + text );
}

void function_client::DecisionHash(QString text)
{
    client->sendMsgToServer("sha_512&" + text);
}

void function_client::DecisionDivision(QString  arr, QString num)
{
    client->sendMsgToServer("mtd_dln_ppl&" + arr + "&" + num);
}

void function_client::DecisionGraf(QString ctop, QString top, QString start_end)
{
    client->sendMsgToServer("graf&" + ctop + "&" + top + "&" + start_end);
}
