// mytcpserver.cpp
#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent),
    m_server(new QTcpServer(this))
{
    // Предполагается, что БД уже инициализирована до запуска сервера.
    connect(m_server, &QTcpServer::newConnection,
            this, &MyTcpServer::onNewConnection);

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
}

void MyTcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);
        m_buffers[client].clear();

        connect(client, &QTcpSocket::readyRead, this, &MyTcpServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &MyTcpServer::onClientDisconnected);

        qDebug() << "New connection from:" << client->peerAddress().toString();
        sendResponse(client, "CONNECTED: Ready for commands\r\n");
    }
}

void MyTcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_buffers[client] += client->readAll();

    int idx;
    while ((idx = m_buffers[client].indexOf('\n')) != -1) {
        QByteArray raw = m_buffers[client].left(idx).trimmed();
        m_buffers[client].remove(0, idx + 1);

        QString command = QString::fromUtf8(raw);
        qDebug() << "Received:" << command;

        // Команда формата "<action>&<payload>"
        QStringList parts = command.split('&');
        if (parts.isEmpty()) {
            sendResponse(client, "ERROR: Empty command\r\n");
            continue;
        }

        QString action = parts.takeFirst().toLower();
        QString payload = parts.join("&");  // всё, что осталось после первого '&'
        QString result;

        if (action == "reg") {
            // payload = "login,password"
            result = handleRegister(payload);
        }
        else if (action == "auth") {
            // payload = "login,password"
            result = handleAuth(payload);
        }
        else if (action == "stats") {
            // payload игнорируется
            result = handleStats(payload);
        }
        else if (action == "sha512") {
            // payload = текст
            result = handleSHA512(payload);
        }
        else if (action == "division") {
            // payload = число
            result = handleDivision(payload);
        }
        else if (action == "graph") {
            // payload = "s,t"
            result = handleShortest(payload);
        }
        else {
            // эхо на всё остальное
            result = "ECHO: " + command;
        }

        // Добавляем CRLF
        QByteArray response = result.toUtf8() + "\r\n";
        sendResponse(client, response);
    }
}

void MyTcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qDebug() << "Client disconnected:" << client->peerAddress().toString();
    m_clients.removeAll(client);
    m_buffers.remove(client);
    client->deleteLater();
}

void MyTcpServer::sendResponse(QTcpSocket *client, const QByteArray &response)
{
    if (client && client->state() == QTcpSocket::ConnectedState) {
        client->write(response);
        client->flush();
    }
}
