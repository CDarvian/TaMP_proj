#include <QCoreApplication>
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyTcpServer server;         // стартует сразу при создании
    return a.exec();            // висим, пока не закроют
}
