#include "reg_and_auth.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RegAndAuth w;
    w.show();

    return a.exec();
}
