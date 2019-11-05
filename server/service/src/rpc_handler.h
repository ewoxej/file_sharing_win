#pragma once
#include "socket_serve.h"
#include "jsonrp.hpp"

class RequestHandler
{
public:
   jsonrpcpp::response_ptr parseRequest( std::string str, SOCKET clSocket );
   void setFolderPath( std::string fPath );
   std::string getFolderPath();
private:
   SOCKET m_clientSocket;
   std::string m_folderPath;
   bool sendFile( std::string path );
   bool receiveFile( std::string path, bool isDirectory );
   jsonrpcpp::response_ptr filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr download( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr upload( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   jsonrpcpp::response_ptr isExist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params );
   void ulongToCharArray( unsigned long& number, char* arr );
   void charArrayToUlong( unsigned long& number, char* arr );
};
