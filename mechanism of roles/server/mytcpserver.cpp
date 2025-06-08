#include "mytcpserver.h"
#include "functions_to_server.h"
#include <QDebug>
#include <utility>

MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_port(port)
{
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
    for (QTcpSocket *client : std::as_const(m_clients)) {
        client->disconnectFromHost();
        if (client->state() != QAbstractSocket::UnconnectedState) {
            client->waitForDisconnected(1000);
        }
        client->deleteLater();
    }
    m_clients.clear();
    m_buffers.clear();
    m_adminClients.clear();
}

void MyTcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        if (!client) continue;

        connect(client, &QTcpSocket::errorOccurred, this, [this, client](QAbstractSocket::SocketError error) {
            qWarning() << "Socket error:" << client->errorString();
            client->deleteLater();
        });

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
        qDebug() << "Processing command:" << command;

        QString result = processCommand(client, command);
        sendResponse(client, result.toUtf8() + "\n");
    }
}

QString MyTcpServer::processCommand(QTcpSocket *client, const QString &command)
{
    int separatorPos = command.indexOf('&');
    if (separatorPos == -1) {
        return "ERROR: Invalid command format";
    }

    QString action = command.left(separatorPos).toLower();
    QString payload = command.mid(separatorPos + 1);
    QString result; // Объявляем переменную result перед использованием

    if (action == "admin_auth") {
        result = handleAdminAuth(payload);
        if (result.startsWith("ADMIN_AUTH_OK")) {
            m_adminClients.insert(client);
        } else {
            m_adminClients.remove(client);
        }
    }
    else if (action == "admin_users") {
        if (isAdminClient(client)) {
            result = handleAdminGetUsers(payload);
        } else {
            result = "ADMIN_ERR: Not authenticated";
        }
    }
    else if (action == "admin_setstatus") {
        if (isAdminClient(client)) {
            result = handleAdminSetStatus(payload);
        } else {
            result = "ADMIN_ERR: Not authenticated";
        }
    }
    else if (action == "reg") {
        result = handleRegister(payload);
    }
    else if (action == "auth") {
        result = handleAuth(payload);
    }
    else if (action == "stats") {
        result = handleStats(payload);
    }
    else if (action == "sha512") {
        result = handleSHA512(payload);
    }
    else if (action == "division") {
        result = handleDivision(payload);
    }
    else if (action == "graph") {
        result = handleShortest(payload);
    }
    else {
        result = "ERROR: Unknown command";
    }

    qDebug() << "Sending response:" << result;
    return result;
}

void MyTcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qDebug() << "Client disconnected:" << client->peerAddress().toString();
    m_clients.removeAll(client);
    m_buffers.remove(client);
    m_adminClients.remove(client);
    client->deleteLater();
}

void MyTcpServer::sendResponse(QTcpSocket *client, const QByteArray &response)
{
    if (client && client->state() == QTcpSocket::ConnectedState) {
        client->write(response);
        if (!client->waitForBytesWritten(3000)) {
            qWarning() << "Failed to write response to client";
        }
    }
}

bool MyTcpServer::isAdminClient(QTcpSocket *client) const
{
    return m_adminClients.contains(client);
}

void MyTcpServer::setAdminClient(QTcpSocket *client, bool isAdmin)
{
    if (isAdmin) {
        m_adminClients.insert(client);
    } else {
        m_adminClients.remove(client);
    }
}
