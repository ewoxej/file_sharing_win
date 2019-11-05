#include "socket_serve.h"
#pragma comment(lib,"ws2_32.lib")

const int CSocketServer::bufferSize = 1024;

CSocketServer::CSocketServer() :
   m_isWorking( true )
{
   char buff[bufferSize];
   WSAStartup( 0x0202, reinterpret_cast<WSADATA*>( &buff[0] ) );
}

bool CSocketServer::connectTo( ULONG ip, ULONG port )
{
   if( ( activeSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      return false;
      WSACleanup();
   }
   sockaddr_in localAdress;
   localAdress.sin_family = AF_INET;
   localAdress.sin_port = htons( port );
   localAdress.sin_addr.s_addr = htonl( ip );

   if( bind( activeSocket, reinterpret_cast<sockaddr*>( &localAdress ),
      sizeof( localAdress ) ) )
   {
      closesocket( activeSocket );
      WSACleanup();
      return false;
   }

   if( listen( activeSocket, 0x100 ) )
   {
      closesocket( activeSocket );
      WSACleanup();
      return false;
   }
   return true;
}

void CSocketServer::setActiveSocket( SOCKET newSocket )
{
   activeSocket = newSocket;
}

SOCKET CSocketServer::getActiveSocket()
{
   return activeSocket;
}

bool CSocketServer::isWorking()
{
   return m_isWorking;
}

void CSocketServer::setWorking( bool working )
{
   m_isWorking = working;
}

void CSocketServer::accept( std::function<void( SOCKET )> serverFunc )
{
   SOCKET clientSocket;
   sockaddr_in clientAdress;
   int clientAdressSize = sizeof( clientAdress );
   while( clientSocket = ::accept( activeSocket, reinterpret_cast<sockaddr*>( &clientAdress ), &clientAdressSize ) )
   {
      if( !m_isWorking ) break;
      threads.push_back( new std::thread( serverFunc, clientSocket ) );
   }
}

CSocketServer::~CSocketServer()
{
   for( const auto i : threads )
   {
      i->join();
      delete i;
   }
}
