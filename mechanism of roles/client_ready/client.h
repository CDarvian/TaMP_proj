#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Client : public QObject
{
    Q_OBJECT

    // Singleton-реализация
    static Client* p_instance;
    class ClientDestroyer {
        Client *p;
    public:
        ClientDestroyer() : p(nullptr) {}
        ~ClientDestroyer() { delete p; }
        void init(Client *ptr) { p = ptr; }
    };
    static ClientDestroyer destroyer;

    explicit Client(QObject *parent = nullptr);
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    friend class ClientDestroyer;

public:
    static Client* getInstance();
    ~Client() override;

    // Отправка команды на сервер (добавляет "\n")
    void sendMsgToServer(const QString &msg);

    // Проверка состояния подключения
    bool isConnected() const;

    void setAdmin(bool isAdmin);
    bool isAdmin() const;
    bool hasAdminPrivileges() const;
    bool isReallyConnected() const {
        return mTcpSocket->state() == QAbstractSocket::ConnectedState;
    }
    bool waitForConnection(int msecs = 3000);

signals:
    // Эмитится при получении полной строки (без "\r\n") от сервера
    void msgFromServer(const QString &msg);
    // Сигнал об изменении состояния подключения
    void connectionStateChanged(bool connected);
    void adminStatusChanged(bool isAdmin);
private slots:
    void slotServerRead();
    void slotConnected();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError error);

private:
    bool m_isAdmin = false;
    QTcpSocket *mTcpSocket;
    QTimer *m_reconnectTimer;
    int m_reconnectAttempts = 0;
};
#endif // CLIENT_H
