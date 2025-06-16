#include "reg_and_auth.h"
#include "client.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Client* client = Client::getInstance();


    bool connected = client->waitForConnection(3000);
    // Проверяем подключение к серверу
    if (!connected) {
        QMessageBox::warning(nullptr, "Warning",
                             "Could not connect to server. Some functionality will be limited.");
    }

    RegAndAuth w;
    w.show();

    return a.exec();
}
