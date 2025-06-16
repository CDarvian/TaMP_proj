#ifndef DBTCPSERVER_H
#define DBTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>

class DbTcpServer : public QObject {
    Q_OBJECT
public:
    explicit DbTcpServer(quint16 port = 33334, QObject *parent = nullptr);
    ~DbTcpServer();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
};

#endif // DBTCPSERVER_H
