#include "tcpclient.h"
#include <QDataStream>
#include <QDir>
#include <QTcpSocket>
#include <iostream>
#include <string>

void ulongToCharArray( unsigned long & number, char * arr )
{
   arr[0] = ( number >> 24 );
   arr[1] = ( number >> 16 );
   arr[2] = ( number >> 8 );
   arr[3] = number;
}

void charArrayToUlong( unsigned long & number, char * arr )
{
   number = 0;
   number |= ( arr[0] << 24 ) & 0xff000000;
   number |= ( arr[1] << 16 ) & 0x00ff0000;
   number |= ( arr[2] << 8 ) & 0x0000ff00;
   number |= arr[3] & 0x000000ff;
}

const int TCPClient::bufferSize = 1024;

TCPClient::TCPClient( QObject *parent ) : QObject( parent ), m_path( QDir::currentPath() )
{

}

void TCPClient::connectTcp( QString ip, quint16 port )
{
   m_socket = new QTcpSocket( this );
   m_socket->connectToHost( ip, port );
   m_socket->waitForConnected();
   m_socket->waitForReadyRead();
}

void TCPClient::readWriteData()
{
   QTextStream cin( stdin );
   QString buffer;
   buffer = m_socket->readAll();
   std::cout << buffer.toStdString() << "\n";
   buffer = cin.readLine();
   QStringList arg = buffer.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
   if( arg.size() == 2 && arg[0] == "download" )
   {
      buffer = downloadFile( arg[1] );
   }
   else if( arg.size() == 2 && arg[0] == "upload" )
   {
      buffer = uploadFile( arg[1] );
   }
   else if( arg.size() == 2 && arg[0] == "exist" )
   {
      buffer = isExist( arg[1] );
   }
   else if( arg.size() == 1 && arg[0] == "sync" )
   {
      buffer = syncFiles();
   }
   else if( arg.size() == 1 && arg[0] == "filelist" )
   {
      buffer = fileList();
   }
   else
   {
      buffer = "unknown command\n";
   }
   std::cout << buffer.toStdString();
}

bool TCPClient::isOpen()
{
   return m_socket->isOpen();
}

void TCPClient::setPath( QString newPath )
{
   m_path = newPath;
}

QString TCPClient::downloadFile( QString filename )
{
   m_socket->readAll();
   QString strBuff;
   QByteArray byteArray;
filename += ".";
   strBuff = makeRpcCallString( "exist", filename );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   if( strBuff == "\"false\"" )
   {
      return "error: file is not exist";
   }
   if( strBuff == "\"dir\"" )
   {
      QDir dir;
      dir.mkdir( m_path + "//" + filename );
      return "true";
   }
   strBuff = makeRpcCallString( "download", filename );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   QFile file( m_path + "//" + filename );
   file.open( QIODevice::ReadWrite );
   if( !file.isOpen() )
   {
      return "error: can`t create file";
   }
   m_socket->waitForReadyRead();
   const int arraySize = 4;
   char arr[arraySize];
   unsigned long sz = 0;
   m_socket->read( arr, arraySize );
   charArrayToUlong( sz, arr );
   unsigned long receivedBytes = 0;
   while( receivedBytes < sz )
   {
      byteArray.clear();
      m_socket->waitForReadyRead();
      byteArray = m_socket->readAll();
      receivedBytes += byteArray.size();
      file.write( byteArray );
   }
   strBuff = "ok";
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   return strBuff;
}

QString TCPClient::uploadFile( QString filename )
{
   m_socket->readAll();
   QString strBuff;
   char buffer[bufferSize];
   QFile file( m_path + "//" + filename );
   file.open( QIODevice::ReadOnly );
   const int arraySize = 4;
   char arr[4]{ arraySize };
   if( !file.isOpen() && !QFileInfo( file ).isDir() )
   {
      m_socket->write( arr, arraySize );
      return "error: file does not exist";
   }
   strBuff = makeRpcCallString( "upload", filename, QFileInfo( file ).isDir() );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   if( strBuff != "ok" ) return "false";
   if( !QFileInfo( file ).isDir() )
   {
      unsigned long filesize = QFileInfo( file ).size();
      ulongToCharArray( filesize, arr );
      m_socket->write( arr, 4 );
      unsigned long sendedBytes = 0;
      int readedBytes;
      m_socket->waitForBytesWritten();
      while( sendedBytes < filesize )
      {
         readedBytes = file.read( buffer, bufferSize );
         m_socket->write( buffer, readedBytes );
         sendedBytes += readedBytes;
      }
   }
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   return strBuff;
}

QString TCPClient::isExist( QString filename )
{
   QString strBuff;
   strBuff = makeRpcCallString( "exist", filename );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   return strBuff;
}

QString TCPClient::syncFiles()
{
    bool downloadIsOk,uploadIsOk;
   QJsonArray jarr;
   QString strBuff;
   strBuff = makeRpcCallString( "filelist" );
   uploadIsOk = uploadFiles( "" );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   QByteArray bar;
   bar.append( strBuff );
   QJsonDocument doc( QJsonDocument::fromJson( bar ) );
   jarr = doc.array();
   downloadIsOk = downloadFiles( "", jarr );
if (downloadIsOk && uploadIsOk) return "true";
else return "false";
}

QString TCPClient::fileList()
{
   QString strBuff;
   strBuff = makeRpcCallString( "filelist" );
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   m_socket->waitForReadyRead();
   strBuff = m_socket->readAll();
   return strBuff;
}


bool TCPClient::uploadFiles( QString folderPath )
{
    bool success = true;
    bool nestedSuccess = true;
   QDir dir( m_path + "\\" + folderPath );
   QStringList nameFilter;
   QFileInfoList clientFolderList = dir.entryInfoList( nameFilter, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
   QString strBuff;
   for( auto i : clientFolderList )
   {
      strBuff = uploadFile( folderPath + "\\" + i.fileName() );
      if(strBuff != "true") success = false;
      if( i.isDir() )
      {
         QString fpath = i.path() + "\\" + i.fileName();
         if(!uploadFiles( folderPath + "\\" + i.fileName() )) nestedSuccess = false;
      }
   }
   return (nestedSuccess && success);
}

bool TCPClient::downloadFiles( QString path, QJsonArray jarr )
{
    bool success = true;
    bool nestedSuccess = true;
   QString strBuff;
   QString absPath = m_path + "\\" + path;
   for( int i = 0; i < jarr.size(); i++ )
   {
      if( jarr[i].isArray() )
      {
         if(!downloadFiles( path + "\\" + jarr[i - 1].toString(), jarr[i].toArray() )) nestedSuccess = false;
      }
      else
      {
         strBuff = downloadFile( path + "\\" + jarr[i].toString() );
         if(strBuff != "true") success = false;
      }
   }
return (nestedSuccess && success);
}


