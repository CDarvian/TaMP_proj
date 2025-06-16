#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QSet>
#include <QList>
#include <QHash>

class MyTcpServer : public QObject {
    Q_OBJECT
public:
    explicit MyTcpServer(quint16 port = 33333, QObject *parent = nullptr);
    ~MyTcpServer();
    int clientCount() const;

signals:
    void clientConnected(const QString &clientAddress);

    void clientDisconnected(const QString &clientAddress);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QString processCommand(QTcpSocket *client, const QString &command); // Изменено на QString
    void sendResponse(QTcpSocket *client, const QByteArray &response);
    bool isAdminClient(QTcpSocket *client) const;
    void setAdminClient(QTcpSocket *client, bool isAdmin);

    QTcpServer *m_server;                      ///< Основной серверный сокет
    QList<QTcpSocket*> m_clients;              ///< Список активных клиентов
    QHash<QTcpSocket*, QByteArray> m_buffers;  ///< Буферы данных для каждого клиента
    QSet<QTcpSocket*> m_adminClients;          ///< Множество администраторов
    quint16 m_port;                            ///< Порт сервера
};

#endif // MYTCPSERVER_H
