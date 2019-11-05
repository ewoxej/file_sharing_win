#pragma once
#include <Winsock2.h>
#include <minwindef.h>
#include <thread>
#include <vector>
#include <functional>

class CSocketServer
{
public:
   CSocketServer();
   ~CSocketServer();
   bool connectTo( ULONG ip, ULONG port );
   void setActiveSocket( SOCKET newSocket );
   SOCKET getActiveSocket();
   bool isWorking();
   void setWorking( bool working );
   void accept( std::function<void( SOCKET )> serverFunc );
private:
   static const int bufferSize;
   std::vector<std::thread*> threads;
   SOCKET activeSocket;
   bool m_isWorking;
};
