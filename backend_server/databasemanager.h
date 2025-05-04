#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    // Получить единственный экземпляр
    static DatabaseManager& instance();

    // Открыть или создать базу по пути
    bool init(const QString& dbPath = "app.db");

    // Исполнить SQL и вернуть текстовый ответ
    QString execute(const QString& sql);

private:
    DatabaseManager();
    ~DatabaseManager();
    Q_DISABLE_COPY(DatabaseManager)

    bool ensureOpen();

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
