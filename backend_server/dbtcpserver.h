#ifndef DBTCPSERVER_H
#define DBTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class DbTcpServer : public QObject {
    Q_OBJECT
public:
    explicit DbTcpServer(quint16 port = 33333, QObject *parent = nullptr);
    ~DbTcpServer();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer *m_server;
    QTcpSocket *m_socket = nullptr;
    QByteArray   m_buffer;
};

#endif // DBTCPSERVER_H
