#include <iostream>
#include "tcpsocket.h"

int main( int argc, char* argv[] )
{
   QString serveraddr;
   quint16 port;
   if( argc < 3 )
   {
      std::cout << "Command line arguments should be next: ip port path";
      return -1;
   }
   serveraddr = argv[1];
   port = std::stoi( argv[2] );
   TCPSocket socket;
   if( argc == 4 )
      socket.setPath( argv[3] );
   socket.connectTcp( serveraddr, port );
   return 0;
}
