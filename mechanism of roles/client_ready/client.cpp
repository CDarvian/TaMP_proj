#include "client.h"
#include <QDebug>
#include <QTimer>

Client* Client::p_instance = nullptr;
Client::ClientDestroyer Client::destroyer;

Client::Client(QObject *parent) : QObject(parent), m_isAdmin(false)
{
    mTcpSocket = new QTcpSocket(this);
    connect(mTcpSocket, &QTcpSocket::readyRead, this, &Client::slotServerRead);
    connect(mTcpSocket, &QTcpSocket::connected, this, &Client::slotConnected);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &Client::slotDisconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(mTcpSocket, &QAbstractSocket::errorOccurred, this, &Client::slotError);
#else
    connect(mTcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &Client::slotError);
#endif
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if(++m_reconnectAttempts > 5) {
            m_reconnectTimer->stop();
            emit msgFromServer("NETWORK: Max reconnect attempts reached");
            return;
        }
        mTcpSocket->connectToHost("127.0.0.1", 33333);
    });
    // Сразу пробуем подключиться к серверу
    mTcpSocket->connectToHost("127.0.0.1", 33333);
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

bool Client::isConnected() const
{
    return mTcpSocket->state() == QAbstractSocket::ConnectedState;
}

void Client::sendMsgToServer(const QString &msg)
{
    if (!isConnected()) {
        mTcpSocket->connectToHost("127.0.0.1", 33333);
        if (!mTcpSocket->waitForConnected(2000)) {
            emit msgFromServer("ERROR: Connection failed. Please check if server is running");
            return;
        }
    }

    QByteArray data = msg.toUtf8() + "\n";
    qint64 bytesWritten = mTcpSocket->write(data);

    if (bytesWritten == -1) {
        emit msgFromServer("ERROR: Failed to send data: " + mTcpSocket->errorString());
        return;
    }

    if (!mTcpSocket->waitForBytesWritten(2000)) {
        emit msgFromServer("ERROR: Write timeout");
    }
}

bool Client::waitForConnection(int msecs)
{
    if (isReallyConnected()) return true;

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &Client::connectionStateChanged, &loop, &QEventLoop::quit);

    timeoutTimer.start(msecs);
    loop.exec();

    return isReallyConnected();
}

void Client::slotServerRead() {
    while (mTcpSocket->canReadLine()) {
        QByteArray line = mTcpSocket->readLine().trimmed();
        QString msg = QString::fromUtf8(line);
        qDebug() << "Received from server:" << msg; // Логирование полученных данных

        if (msg.startsWith("ADMIN_AUTH_OK")) {
            setAdmin(true);
            emit msgFromServer("ADMIN: Authentication successful");
        }
        else if (msg.startsWith("ADMIN_AUTH_ERR")) {
            setAdmin(false);
            emit msgFromServer("ADMIN: Authentication failed");
        }
        else {
            emit msgFromServer(msg);
        }
    }
}

void Client::slotConnected()
{
    qDebug() << "Connected to server";
    m_reconnectAttempts = 0; // Сбрасываем счетчик попыток
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
    emit connectionStateChanged(true);
}

void Client::slotDisconnected()
{
    qDebug() << "Disconnected from server, attempting to reconnect...";
    m_reconnectTimer->start();
    emit connectionStateChanged(false);
}

void Client::slotError(QAbstractSocket::SocketError error)
{
    QString errorMsg;
    switch(error) {
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Connection refused - server may not be running";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg = "Remote host closed the connection";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Host not found - check address and port";
        break;
    default:
        errorMsg = mTcpSocket->errorString();
    }

    qDebug() << "Connection error:" << errorMsg;
    emit msgFromServer("NETWORK ERROR: " + errorMsg);
    emit connectionStateChanged(false);

    // Автоматическое переподключение через 5 сек
    QTimer::singleShot(5000, this, [this]() {
        mTcpSocket->connectToHost("127.0.0.1", 33333);
    });
}


void Client::setAdmin(bool isAdmin) {
    if (m_isAdmin != isAdmin) {
        m_isAdmin = isAdmin;
        emit adminStatusChanged(isAdmin);
        qDebug() << "Admin status changed to:" << isAdmin; // Для отладки
    }
}

bool Client::isAdmin() const
{
    return m_isAdmin;
}

bool Client::hasAdminPrivileges() const
{
    return m_isAdmin;
}


