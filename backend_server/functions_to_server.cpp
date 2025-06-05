// functions_to_server.cpp
#include "functions_to_server.h"
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QStringList>
#include <QAbstractSocket>

// Простенький клиент для общения с удалённым DB-сервером на порту 33334
namespace {
class DbClient {
public:
    static DbClient& instance() {
        static DbClient inst;
        return inst;
    }

    // Отправляет SQL-запрос (UTF-8 + '\n') и возвращает весь ответ (CRLF-terminated)
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
        // Ждём первой порции данных
        if (!socket.waitForReadyRead(2000))
            return "ERROR: Read timeout";
        resp += socket.readAll();

        // Читаем всё, что пришло с небольшими паузами
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

// Разбить ответ DB-сервера на строки по "\r\n"
static QStringList splitLines(const QString& resp) {
    return resp.split("\r\n", Qt::SkipEmptyParts);
}

// SHA-512 хэширование (hex)
static QString sha512Hex(const QString& text) {
    QByteArray hash = QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha512);
    return hash.toHex();
}

// Метод «деление пополам» для приближённого вычисления √value
static double binarySqrt(double value) {
    double left = 0.0;
    double right = (value < 1.0 ? 1.0 : value);
    for (int i = 0; i < 60; ++i) {
        double mid = (left + right) / 2.0;
        if (mid * mid < value) {
            left = mid;
        } else {
            right = mid;
        }
    }
    return (left + right) / 2.0;
}

// Простой неориентированный граф (жёстко заданный пример)
struct Graph {
    QHash<int, QVector<int>> adj;
    void addEdge(int u, int v) {
        adj[u].append(v);
        adj[v].append(u);
    }
    bool hasPathWithin(int s, int t, int limit) {
        if (s == t) return true;
        QHash<int,int> dist;
        QVector<int> curr;
        curr.append(s);
        dist[s] = 0;
        while (!curr.isEmpty()) {
            QVector<int> next;
            for (int u : curr) {
                int d = dist[u];
                if (d >= limit) continue;
                for (int v : adj.value(u)) {
                    if (dist.contains(v)) continue;
                    dist[v] = d + 1;
                    if (v == t) return true;
                    next.append(v);
                }
            }
            curr = next;
        }
        return false;
    }
    int shortestPath(int s, int t) {
        QList<int> keys = adj.keys();
        int n = keys.size();
        int lo = 0, hi = n, ans = -1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (hasPathWithin(s, t, mid)) {
                ans = mid;
                hi = mid - 1;
            } else {
                lo = mid + 1;
            }
        }
        return ans;
    }
};

static Graph graph;
static bool graphInited = false;
static void initGraph() {
    if (graphInited) return;
    // Пример связных рёбер: 1–2, 2–3, 3–4, 1–4
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);
    graph.addEdge(3, 4);
    graph.addEdge(1, 4);
    graphInited = true;
}
}

// Регистрация: payload = "login,password"
QString handleRegister(const QString &payload) {
    QStringList parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return "REGISTER_ERR: Bad format";

    QString login = parts[0].trimmed();
    QString password = parts[1].trimmed();
    QString passHash = sha512Hex(password);

    // 1) Проверяем, есть ли уже пользователь с данным логином
    QString checkSql = QString("SELECT COUNT(*) FROM User WHERE login='%1';").arg(login);
    QString checkResp = DbClient::instance().sendQuery(checkSql);
    if (checkResp.startsWith("ERROR:"))
        return "REGISTER_ERR: DB error";

    auto checkLines = splitLines(checkResp);
    int count = (checkLines.size() > 1) ? checkLines[1].toInt() : 0;
    if (count > 0)
        return "REGISTER_ERR: User exists";

    // 2) Вставляем нового пользователя (status = 'offline')
    QString insSql = QString(
                         "INSERT INTO User(login,password,status) VALUES('%1','%2','offline');"
                         ).arg(login, passHash);

    QString insResp = DbClient::instance().sendQuery(insSql);
    if (insResp.startsWith("ERROR:"))
        return "REGISTER_ERR: DB error";

    return "REGISTER_OK: User created";
}

// Авторизация: payload = "login,password"
QString handleAuth(const QString &payload) {
    QStringList parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return "AUTH_ERR: Bad format";

    QString login = parts[0].trimmed();
    QString password = parts[1].trimmed();
    QString passHash = sha512Hex(password);

    QString sql = QString(
                      "SELECT login FROM User WHERE login='%1' AND password='%2';"
                      ).arg(login, passHash);

    QString resp = DbClient::instance().sendQuery(sql);
    if (resp.startsWith("ERROR:"))
        return "AUTH_ERR: DB error";

    auto lines = splitLines(resp);
    if (lines.size() < 2)
        return "AUTH_ERR: Invalid credentials";

    return QString("AUTH_OK: Welcome %1").arg(login);
}

// Статистика: payload игнорируется
QString handleStats(const QString &) {
    QString resp = DbClient::instance().sendQuery("SELECT COUNT(*) FROM User;");
    if (resp.startsWith("ERROR:"))
        return "STATS_ERR: DB error";

    auto lines = splitLines(resp);
    QString total = (lines.size() > 1) ? lines[1] : "0";
    return QString("STATS: Total users = %1").arg(total);
}

// SHA-512-хэширование любого текста: payload = текст
QString handleSHA512(const QString &payload) {
    if (payload.isEmpty())
        return "SHA512_ERR: No input";

    QString hash = sha512Hex(payload);
    return QString("SHA512: %1").arg(hash);
}

// Деление пополам (√value): payload = число (строка)
QString handleDivision(const QString &payload) {
    bool ok = false;
    double val = payload.toDouble(&ok);
    if (!ok)
        return "DIV_ERR: Bad format";

    double result = binarySqrt(val);
    return QString("DIV_RESULT: %1").arg(result);
}

// Поиск кратчайшего пути: payload = "s,t"
QString handleShortest(const QString &payload) {
    QStringList parts = payload.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return "GRAPH_ERR: Bad format";

    bool ok1 = false, ok2 = false;
    int s = parts[0].trimmed().toInt(&ok1);
    int t = parts[1].trimmed().toInt(&ok2);
    if (!ok1 || !ok2)
        return "GRAPH_ERR: Invalid vertices";

    initGraph();
    int dist = graph.shortestPath(s, t);
    if (dist < 0)
        return "GRAPH: No path";

    return QString("GRAPH: %1").arg(dist);
}
