#include <QCoreApplication>
#include <QDebug>
#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Попытка инициализации БД
    if (!DatabaseManager::instance().init("Test.db")) {
        qCritical() << "❌ Не удалось открыть или создать БД";
        return -1;
    }
    qDebug() << "✅ БД инициализирована";

    // Создадим тестовую таблицу (если ещё нет)
    QString result = DatabaseManager::instance().execute(
        "CREATE TABLE IF NOT EXISTS TestTable("
        "id INTEGER PRIMARY KEY, "
        "name TEXT"
        ");"
        );
    qDebug() << "CREATE TABLE result:" << result;

    // Выведем все таблицы в БД
    result = DatabaseManager::instance().execute(
        "SELECT name FROM sqlite_master WHERE type='table';"
        );
    qDebug() << "Список таблиц:\n" << result;

    return 0;
}
