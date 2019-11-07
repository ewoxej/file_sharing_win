#include <iostream>
#include "tcpclient.h"

int main( int argc, char* argv[] )
{
   QString serveraddr;
   quint16 port;
   if( argc < 3 )
   {
      std::cout << "Command line arguments should be next: ip port path" << std::endl;
      return -1;
   }
   std::cout <<
      "Enter \"sync\" to synchronize client folder with server folder.\n"
      "download(upload) filename - to download(upload) a file,\n"
      "\"filelist\" - to enumerate files in server folder" << std::endl;
   serveraddr = argv[1];
   port = std::stoi( argv[2] );
   TCPClient socket;
   if( argc == 4 )
      socket.setPath( argv[3] );
   socket.connectTcp( serveraddr, port );
   while( socket.isOpen() )
   {
      socket.readWriteData();
   }
   return 0;
}
