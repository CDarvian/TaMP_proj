#include "functions_to_server.h"
#include <QTcpSocket>
#include <QStringList>
#include <QAbstractSocket>

namespace {
// Простенький клиент для общения с DB-сервером
class DbClient {
public:
    static DbClient& instance() {
        static DbClient inst;
        return inst;
    }

    // Отправляет SQL-запрос и возвращает весь ответ (UTF-8)
    QString sendQuery(const QString& sql) {
        if (socket.state() != QAbstractSocket::ConnectedState) {
            socket.connectToHost("127.0.0.1", 33334);
            if (!socket.waitForConnected(2000))
                return "ERROR: Cannot connect to DB server";
        }

        QByteArray req = sql.toUtf8() + '\n';
        socket.write(req);
        if (!socket.waitForBytesWritten(1000))
            return "ERROR: Write timeout";

        QByteArray resp;
        // Ждем данных
        if (!socket.waitForReadyRead(2000))
            return "ERROR: Read timeout";
        resp += socket.readAll();
        // Считываем оставшиеся порции (короткие паузы)
        while (socket.waitForReadyRead(100)) {
            resp += socket.readAll();
        }

        return QString::fromUtf8(resp).trimmed();
    }

private:
    QTcpSocket socket;
    DbClient() {}
    ~DbClient() { socket.disconnectFromHost(); }
    Q_DISABLE_COPY(DbClient)
};

// Экранирование одиночных кавычек для SQL
static QString escapeSql(const QString& s) {
    QString r = s;
    r.replace("'", "''");
    return r;
}

// Разбиение ответа на строки по CRLF
static QStringList splitLines(const QString& resp) {
    return resp.split("\r\n", Qt::KeepEmptyParts);
}
}

QString handleRegister(const QString& payload) {
    auto parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return "REGISTER_ERR: Bad format";

    QString user = escapeSql(parts[0].trimmed());
    QString pass = escapeSql(parts[1].trimmed());

    // Проверяем дубликат
    QString checkSql = QString("SELECT COUNT(*) FROM User WHERE login='%1';").arg(user);
    QString checkResp = DbClient::instance().sendQuery(checkSql);
    if (checkResp.startsWith("ERROR:"))
        return "REGISTER_ERR: DB error";

    auto lines = splitLines(checkResp);
    int count = (lines.size() > 1) ? lines[1].toInt() : 0;
    if (count > 0)
        return "REGISTER_ERR: User already exists";

    // Создаем нового пользователя
    QString insSql = QString(
                         "INSERT INTO User(login,password,status) VALUES('%1','%2','offline');"
                         ).arg(user, pass);
    QString insResp = DbClient::instance().sendQuery(insSql);
    if (insResp.startsWith("ERROR:"))
        return "REGISTER_ERR: DB error";

    return "REGISTER_OK: Registration successful";
}

QString handleLogin(const QString& payload) {
    auto parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return "LOGIN_ERR: Bad format";

    QString user = escapeSql(parts[0].trimmed());
    QString pass = escapeSql(parts[1].trimmed());

    QString sql = QString("SELECT password FROM User WHERE login='%1';").arg(user);
    QString resp = DbClient::instance().sendQuery(sql);
    if (resp.startsWith("ERROR:"))
        return "LOGIN_ERR: DB error";

    auto lines = splitLines(resp);
    if (lines.size() < 2)
        return "LOGIN_ERR: Invalid credentials";

    if (lines[1] != pass)
        return "LOGIN_ERR: Invalid credentials";

    return QString("LOGIN_OK: Welcome, %1!").arg(user);
}

QString handleStats(const QString&) {
    QString resp = DbClient::instance().sendQuery("SELECT COUNT(*) FROM User;");
    if (resp.startsWith("ERROR:"))
        return "STATS_ERR: DB error";

    auto lines = splitLines(resp);
    QString cnt = (lines.size() > 1) ? lines[1] : "0";
    return QString("STATS: Total users = %1").arg(cnt);
}
