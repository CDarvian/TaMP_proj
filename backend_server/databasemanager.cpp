#include "databasemanager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlDriver>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() { }

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::init(const QString& dbPath) {
    if (QSqlDatabase::contains("db_server")) {
        m_db = QSqlDatabase::database("db_server");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "db_server");
        m_db.setDatabaseName(dbPath);
    }
    if (!m_db.open()) {
        qCritical() << "Не удалось открыть БД:" << m_db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::ensureOpen() {
    if (!m_db.isValid() || !m_db.isOpen()) {
        qCritical() << "БД не открыта!";
        return false;
    }
    return true;
}

QString DatabaseManager::execute(const QString& sql) {
    if (!ensureOpen())
        return "ERROR: DB not open";

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        return QString("ERROR: %1").arg(query.lastError().text());
    }

    QString cmd = sql.trimmed().toUpper();
    if (cmd.startsWith("SELECT")) {
        QSqlRecord rec = query.record();
        int cols = rec.count();
        QStringList lines;

        // Заголовки
        QStringList headers;
        for (int i = 0; i < cols; ++i)
            headers << rec.fieldName(i);
        lines << headers.join("\t");

        // Данные
        while (query.next()) {
            QStringList row;
            for (int i = 0; i < cols; ++i)
                row << query.value(i).toString();
            lines << row.join("\t");
        }
        return lines.join("\r\n");
    } else {
        int changes = query.numRowsAffected();
        return QString("OK: Rows affected = %1").arg(changes);
    }
}
