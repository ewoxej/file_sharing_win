#include "tcpsocket.h"

TCPSocket::TCPSocket( QObject *parent ) : QObject( parent ), path( QDir::currentPath() )
{

}

void TCPSocket::connectTcp( QString ip, quint16 port )
{
   socket = new QTcpSocket( this );
   connect( socket, SIGNAL( readyRead() ), SLOT( readTcpData() ) );
   socket->connectToHost( ip, port );
   socket->waitForConnected();
   socket->waitForReadyRead();
}

void TCPSocket::setPath( QString newPath )
{
   path = newPath;
}

void TCPSocket::readTcpData()
{
   char buff[bufferSize];
   int recv_bytes;
   while( socket->isOpen() )
   {
      recv_bytes = socket->read( buff, bufferSize );
      buff[recv_bytes] = 0;
      std::cout << buff << std::endl;
      std::cin >> buff;
      if( !strcmp( &buff[0], "sync" ) )
      {
         QJsonObject jsonreq;
         jsonreq["jsonrpc"] = "2.0";
         jsonreq["method"] = "sync";
         QJsonArray jarray;
         jarray.push_back( path );
         jsonreq["params"] = jarray;
         jsonreq["id"] = 0;
         QJsonDocument doc( jsonreq );
         strcpy_s( buff, doc.toJson( QJsonDocument::Compact ) );
         std::cout << buff << std::endl;
      }
      if( !strcmp( &buff[0], "filelist" ) )
      {
         QJsonObject jsonreq;
         jsonreq["jsonrpc"] = "2.0";
         jsonreq["method"] = "filelist";
         QJsonArray jarray;
         jsonreq["params"] = jarray;
         jsonreq["id"] = 0;
         QJsonDocument doc( jsonreq );
         strcpy_s( buff, doc.toJson( QJsonDocument::Compact ) );
         std::cout << buff << std::endl;
      }
      socket->write( std::string( buff ).c_str(), strlen( buff ) );
      socket->waitForReadyRead();
   }
}
