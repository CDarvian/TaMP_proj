#include "reg_and_auth.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RegAndAuth w;
    mainwindow mainWin;
    w.setmainwindow(&mainWin);
    w.show();

    // Главное окно задач покажем после успешной авторизации

    return a.exec();
}
