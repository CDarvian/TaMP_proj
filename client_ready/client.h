#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT

    // Singleton‐реализация
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

signals:
    // Эмитится при получении полной строки (без "\r\n") от сервера
    void msgFromServer(const QString &msg);

private slots:
    void slotServerRead();

private:
    QTcpSocket *mTcpSocket;
};

#endif // CLIENT_H
