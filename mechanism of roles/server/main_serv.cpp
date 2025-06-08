#include <QCoreApplication>
#include "mytcpserver.h"
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!DatabaseManager::instance().init("Test.db")) {
        qCritical() << "Failed to initialize database";
        return -1;
    }

    MyTcpServer server(33333);        // стартует сразу при создании
    return a.exec();            // висим, пока не закроют
}
