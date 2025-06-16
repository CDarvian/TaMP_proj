#include "dbtcpserver.h"
#include "databasemanager.h"
#include <QDebug>

DbTcpServer::DbTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this, &DbTcpServer::onNewConnection);
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start DB server on port" << port;
    } else {
        qDebug() << "DB server listening on port" << port;
    }
}

DbTcpServer::~DbTcpServer() {
    m_server->close();
    m_server->deleteLater();
}

void DbTcpServer::onNewConnection() {
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;

    qDebug() << "Client connected:" << socket->peerAddress().toString();

    connect(socket, &QTcpSocket::readyRead, this, &DbTcpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &DbTcpServer::onClientDisconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    m_buffers[socket].clear();
}

void DbTcpServer::onClientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_buffers.remove(socket);
        qDebug() << "Client disconnected:" << socket->peerAddress().toString();
    }
}

void DbTcpServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray newData = socket->readAll();
    qDebug() << "Received data from:" << socket->peerAddress() << "content:" << newData;

    try {
        m_buffers[socket] += newData;

        int idx;
        while ((idx = m_buffers[socket].indexOf('\n')) != -1) {
            QByteArray line = m_buffers[socket].left(idx).trimmed();
            m_buffers[socket].remove(0, idx + 1);

            QString sql = QString::fromUtf8(line);
            qDebug() << "Executing SQL:" << sql;

            QString result = DatabaseManager::instance().execute(sql);

            if (socket->state() == QTcpSocket::ConnectedState) {
                QByteArray response = result.toUtf8() + "\n";
                socket->write(response);
                if (!socket->waitForBytesWritten(2000)) {
                    qWarning() << "Failed to write response to DB client";
                }
            }
        }
    } catch (const std::exception& e) {
        qCritical() << "DB server error:" << e.what();
        socket->disconnectFromHost();
    }
}
