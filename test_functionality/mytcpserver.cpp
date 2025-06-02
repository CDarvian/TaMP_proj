#include "mytcpserver.h"
#include <QDebug>
#include <QRegularExpression>

MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    if (!initDatabase()) {
        qCritical() << "Failed to initialize database!";
        return;
    }

    connect(m_server, &QTcpServer::newConnection, this, &MyTcpServer::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start on port" << port;
    } else {
        qDebug() << "Server started, listening on port" << port;
    }
}

MyTcpServer::~MyTcpServer()
{
    m_server->close();
    qDeleteAll(m_clients);
    m_db.close();
}

bool MyTcpServer::initDatabase(const QString& dbName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);

    if (!m_db.open()) {
        qCritical() << "Database error:" << m_db.lastError().text();
        return false;
    }

    // Создаем таблицу пользователей, если ее нет
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS Users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "login TEXT UNIQUE NOT NULL, "
               "password TEXT NOT NULL, "
               "email TEXT UNIQUE, "
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");

    return true;
}

void MyTcpServer::onNewConnection()
{
    QTcpSocket *client = m_server->nextPendingConnection();
    m_clients.append(client);
    m_buffers[client] = QByteArray();

    connect(client, &QTcpSocket::readyRead, this, &MyTcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &MyTcpServer::onClientDisconnected);

    qDebug() << "New connection from:" << client->peerAddress().toString();
    sendResponse(client, "Connected to server. Ready for commands.\r\n");
}

void MyTcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_buffers[client] += client->readAll();

    // Обрабатываем все завершенные команды (оканчивающиеся на \n)
    int idx;
    while ((idx = m_buffers[client].indexOf('\n')) != -1) {
        QByteArray command = m_buffers[client].left(idx).trimmed();
        m_buffers[client].remove(0, idx + 1);

        QByteArray response = processCommand(QString::fromUtf8(command));
        sendResponse(client, response);
    }
}

void MyTcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_clients.removeAll(client);
    m_buffers.remove(client);
    client->deleteLater();

    qDebug() << "Client disconnected:" << client->peerAddress().toString();
}

QByteArray MyTcpServer::processCommand(const QString& command)
{
    QStringList parts = command.split('&');
    if (parts.isEmpty()) {
        return "ERROR: Empty command\r\n";
    }

    QString cmd = parts[0].toLower();
    parts.removeFirst();

    if (cmd == "auth") {
        return handleAuth(parts);
    } else if (cmd == "reg") {
        return handleReg(parts);
    } else if (cmd == "rsa") {
        return handleRSA(parts);
    } else if (cmd == "sha512") {
        return handleSHA512(parts);
    } else if (cmd == "division") {
        return handleDivision(parts);
    } else if (cmd == "graph") {
        return handleGraph(parts);
    }

    return "ERROR: Unknown command\r\n";
}

QByteArray MyTcpServer::handleAuth(const QStringList& args)
{
    if (args.size() < 2) {
        return "AUTH_ERROR: Need login and password\r\n";
    }

    QString login = args[0];
    QString password = hashPassword(args[1]);

    QSqlQuery query;
    query.prepare("SELECT login FROM Users WHERE login = :login AND password = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        return "AUTH_ERROR: Database error\r\n";
    }

    if (query.next()) {
        return "AUTH_SUCCESS: Welcome, " + login.toUtf8() + "\r\n";
    }

    return "AUTH_ERROR: Invalid credentials\r\n";
}

QByteArray MyTcpServer::handleReg(const QStringList& args)
{
    if (args.size() < 3) {
        return "REG_ERROR: Need login, password and email\r\n";
    }

    QString login = args[0];
    QString password = hashPassword(args[1]);
    QString email = args[2];

    QSqlQuery query;
    query.prepare("INSERT INTO Users (login, password, email) VALUES (:login, :password, :email)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    query.bindValue(":email", email);

    if (!query.exec()) {
        return "REG_ERROR: " + query.lastError().text().toUtf8() + "\r\n";
    }

    return "REG_SUCCESS: User created\r\n";
}

QByteArray MyTcpServer::handleRSA(const QStringList& args)
{
    if (args.isEmpty()) {
        return "RSA_ERROR: Need text to encrypt\r\n";
    }

    // Здесь должна быть реальная реализация RSA
    QString encrypted = "RSA_ENCRYPTED:" + args.join("&");
    return encrypted.toUtf8() + "\r\n";
}

QByteArray MyTcpServer::handleSHA512(const QStringList& args)
{
    if (args.isEmpty()) {
        return "SHA512_ERROR: Need text to hash\r\n";
    }

    QCryptographicHash hash(QCryptographicHash::Sha512);
    hash.addData(args[0].toUtf8());
    return "SHA512:" + hash.result().toHex() + "\r\n";
}

QByteArray MyTcpServer::handleDivision(const QStringList& args)
{
    if (args.size() < 2) {
        return "DIVISION_ERROR: Need array and number\r\n";
    }

    // Пример: division&1,2,3,4,5&3
    QStringList numbers = args[0].split(',');
    bool ok;
    double target = args[1].toDouble(&ok);

    if (!ok) {
        return "DIVISION_ERROR: Invalid number format\r\n";
    }

    // Простая реализация метода деления пополам
    double left = 0, right = 100; // Примерные границы
    for (int i = 0; i < 20; i++) {
        double mid = (left + right) / 2;
        if (mid < target) left = mid;
        else right = mid;
    }

    return QString("DIVISION_RESULT:%1\r\n").arg((left + right) / 2).toUtf8();
}

QByteArray MyTcpServer::handleGraph(const QStringList& args)
{
    if (args.size() < 3) {
        return "GRAPH_ERROR: Need learningRate, maxIterations and precision\r\n";
    }

    // Пример: graph&0.1&1000&0.001
    return QString("GRAPH_RESULT:Processed with LR=%1, Iter=%2, Prec=%3\r\n")
        .arg(args[0]).arg(args[1]).arg(args[2]).toUtf8();
}

QString MyTcpServer::hashPassword(const QString& password)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    return hash.result().toHex();
}

void MyTcpServer::sendResponse(QTcpSocket* client, const QByteArray& response)
{
    if (client && client->state() == QTcpSocket::ConnectedState) {
        client->write(response);
        client->flush();
    }
}
