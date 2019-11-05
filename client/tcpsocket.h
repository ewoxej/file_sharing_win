#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QDir>
#include <iostream>

const int bufferSize = 1024;
class TCPSocket : public QObject
{
   Q_OBJECT
public:
   explicit TCPSocket( QObject *parent = nullptr );
   void connectTcp( QString ip, quint16 port );
   void setPath( QString newPath );
signals:

public slots:
   void readTcpData();
private:
   QTcpSocket* socket;
   QString path;
};

#endif // TCPSOCKET_H
