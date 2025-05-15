#include "mytcpserver.h"
#include "functions_to_server.h"
#include <QDebug>

MyTcpServer::MyTcpServer(quint16 port, QObject *parent)
    : QObject(parent),
    m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this, &MyTcpServer::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Не удалось запустить сервер на порту" << port;
    } else {
        qDebug() << "Сервер запущен на порту" << port;
    }
}

MyTcpServer::~MyTcpServer() {
    m_server->close();
    // сокеты удалятся в onClientDisconnected
}

void MyTcpServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *client = m_server->nextPendingConnection();
        m_clients.append(client);
        m_buffers[client].clear();
        qDebug() << "Клиент подключился:"
                 << client->peerAddress().toString()
                 << ":" << client->peerPort();

        connect(client, &QTcpSocket::readyRead,
                this, &MyTcpServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected,
                this, &MyTcpServer::onClientDisconnected);
    }
}

void MyTcpServer::onReadyRead() {
    // Узнаём, какой клиент прислал данные
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    // Накопим в буфер
    m_buffers[client] += client->readAll();

    int idx;
    // Разбираем по строкам (\n)
    while ((idx = m_buffers[client].indexOf('\n')) != -1) {
        QByteArray raw = m_buffers[client].left(idx).trimmed();
        m_buffers[client].remove(0, idx + 1);

        QString line = QString::fromUtf8(raw);
        qDebug() << "Получено от" << client->peerAddress().toString()
                 << ":" << line;

        // Обработка команд
        QString response;
        if (line.startsWith("LOGIN:")) {
            response = handleLogin(line.mid(QString("LOGIN:").length()));
        }
        else if (line.startsWith("REGISTER:")) {
            response = handleRegister(line.mid(QString("REGISTER:").length()));
        }
        else if (line.startsWith("STATS:")) {
            response = handleStats(line.mid(QString("STATS:").length()));
        }
        else {
            // Обычное эхо
            response = "ECHO: " + line;
        }

        // Отправляем ответ с CRLF
        client->write(response.toUtf8() + "\r\n");
        client->flush();
    }
}

void MyTcpServer::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qDebug() << "Клиент отключился:"
             << client->peerAddress().toString();
    m_clients.removeAll(client);
    m_buffers.remove(client);
    client->deleteLater();
}
