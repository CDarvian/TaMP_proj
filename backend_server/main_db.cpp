#include <QCoreApplication>
#include "databasemanager.h"
#include "dbtcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Инициализируем БД
    if (!DatabaseManager::instance().init("Test.db")) {
        return -1;
    }

    // Запускаем DB-сервер
    DbTcpServer server(33333);

    return a.exec();
}
