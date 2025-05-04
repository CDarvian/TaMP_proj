#include "dbtcpserver.h"
#include "databasemanager.h"
#include <QDebug>

DbTcpServer::DbTcpServer(quint16 port, QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this, &DbTcpServer::onNewConnection);
    if (!m_server->listen(QHostAddress::Any, port)) {
        qCritical() << "Не удалось запустить DB-сервер на порту" << port;
    } else {
        qDebug() << "DB-сервер слушает порт" << port;
    }
}

DbTcpServer::~DbTcpServer() {
    m_server->close();
}

void DbTcpServer::onNewConnection() {
    m_socket = m_server->nextPendingConnection();
    qDebug() << "Клиент подключился:" << m_socket->peerAddress().toString();
    connect(m_socket, &QTcpSocket::readyRead, this, &DbTcpServer::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, m_socket, &QTcpSocket::deleteLater);
    m_buffer.clear();
}

void DbTcpServer::onReadyRead() {
    m_buffer += m_socket->readAll();
    int idx;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);

        QString sql = QString::fromUtf8(line);
        qDebug() << "Получен SQL:" << sql;

        QString result = DatabaseManager::instance().execute(sql);
        // Отправляем ответ + CRLF
        m_socket->write(result.toUtf8() + "\r\n");
    }
}
