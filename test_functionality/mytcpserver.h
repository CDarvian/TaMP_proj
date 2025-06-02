#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(quint16 port = 33333, QObject *parent = nullptr);
    ~MyTcpServer();

    bool initDatabase(const QString& dbName = "server.db");

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QSqlDatabase m_db;
    QList<QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QByteArray> m_buffers;

    // Обработчики команд
    QByteArray processCommand(const QString& command);
    QByteArray handleAuth(const QStringList& args);
    QByteArray handleReg(const QStringList& args);
    QByteArray handleRSA(const QStringList& args);
    QByteArray handleSHA512(const QStringList& args);
    QByteArray handleDivision(const QStringList& args);
    QByteArray handleGraph(const QStringList& args);

    // Вспомогательные функции
    QString hashPassword(const QString& password);
    void sendResponse(QTcpSocket* client, const QByteArray& response);
};

#endif // MYTCPSERVER_H
