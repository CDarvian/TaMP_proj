#include "mytcpserver.h"
#include "functions_to_server.h"
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent) {
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Server is not started";
    } else {
        qDebug() << "Server is started on port 33333";
    }
}

MyTcpServer::~MyTcpServer() {
    mTcpServer->close();
}

void MyTcpServer::slotNewConnection() {
    mTcpSocket = mTcpServer->nextPendingConnection();
    qDebug() << "Client connected from" << mTcpSocket->peerAddress().toString();
    connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead() {
    // Читаем всё, копим до разделителя '\n'
    while (mTcpSocket->bytesAvailable() > 0) {
        mBuffer += QString::fromUtf8(mTcpSocket->readAll());
        int idx;
        while ((idx = mBuffer.indexOf('\n')) != -1) {
            QString line = mBuffer.left(idx).trimmed();
            mBuffer.remove(0, idx + 1);

            // Примитивный парсер: команды вида "CMD:payload"
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
                // эхо на всё остальное
                response = "ECHO: " + line;
            }

            // Отправляем с '\r\n'
            mTcpSocket->write(response.toUtf8() + "\r\n");
        }
    }
}

void MyTcpServer::slotClientDisconnected() {
    qDebug() << "Client disconnected";
    mTcpSocket->close();
}
