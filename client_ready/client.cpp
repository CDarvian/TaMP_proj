#include "client.h"
#include <QDebug>

Client* Client::p_instance = nullptr;
Client::ClientDestroyer Client::destroyer;

Client::Client(QObject *parent)
    : QObject(parent)
{
    mTcpSocket = new QTcpSocket(this);
    // Сразу пробуем подключиться к серверу (порт 33333)
    mTcpSocket->connectToHost("127.0.0.1", 33333);
    connect(mTcpSocket, &QTcpSocket::readyRead,
            this, &Client::slotServerRead);
}

Client::~Client()
{
    if (mTcpSocket->state() == QAbstractSocket::ConnectedState)
        mTcpSocket->disconnectFromHost();
}

Client* Client::getInstance()
{
    if (!p_instance) {
        p_instance = new Client();
        destroyer.init(p_instance);
    }
    return p_instance;
}

void Client::sendMsgToServer(const QString &msg)
{
    if (mTcpSocket->state() != QAbstractSocket::ConnectedState) {
        // попытка переподключиться
        mTcpSocket->connectToHost("127.0.0.1", 33333);
        if (!mTcpSocket->waitForConnected(2000)) {
            emit msgFromServer("ERROR: Not connected to server");
            return;
        }
    }
    // отправляем с '\n'
    QByteArray data = msg.toUtf8() + "\n";
    mTcpSocket->write(data);
    mTcpSocket->flush();
}

void Client::slotServerRead()
{
    // читаем построчно (server шлёт “...\r\n”)
    while (mTcpSocket->canReadLine()) {
        QByteArray line = mTcpSocket->readLine().trimmed();
        QString msg = QString::fromUtf8(line);
        qDebug() << "Client got:" << msg;
        emit msgFromServer(msg);
    }
}
