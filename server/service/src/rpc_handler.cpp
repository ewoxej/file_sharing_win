#define _WINSOCK2API_
#include <Windows.h>
#include <string>
#include "rpc_handler.h"
#pragma comment(lib,"Mswsock.lib")

bool RequestHandler::sendFile( std::string path )
{
   const int bufferSize = 1024;
   CHAR buffer[bufferSize];
   DWORD bytesRecv = 0;
   HANDLE file = CreateFileA( path.c_str(), FILE_READ_DATA, GENERIC_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
   DWORD size = GetFileSize( file, nullptr );
   if( file == INVALID_HANDLE_VALUE )
      size = 0;
   char arr[4];
   ulongToCharArray( size, arr );
   send( m_clientSocket, (char*)arr, 4, 0 );
   if ( file != INVALID_HANDLE_VALUE )
   TransmitFile( m_clientSocket, file, 0, 0, nullptr, nullptr, TF_WRITE_BEHIND );
   CloseHandle( file );
   bytesRecv = recv( m_clientSocket, &buffer[0], sizeof( buffer ), 0 );
   if( strcmp( buffer, "ok" ) )
      return true;
   else
      return false;
}

bool RequestHandler::receiveFile( std::string path, bool isDirectory )
{
   path += ".";
   const int bufferSize = 1024;
   CHAR buffer[bufferSize];
   char arr[4]{ 0 };
   unsigned long sz = 0;
   if( !isDirectory )
   {
      recv( m_clientSocket, &arr[0], 4, 0 );
      charArrayToUlong( sz, arr );
   }
   if( isDirectory )
   {
      CreateDirectoryA( path.c_str(), nullptr );
   }
   else
   {
      HANDLE file = CreateFileA( path.c_str(), FILE_ALL_ACCESS, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
      unsigned long receivedBytes = 0;
      int rBytes = 0;
      while( receivedBytes < sz )
      {
         rBytes = recv( m_clientSocket, &buffer[0], sizeof( buffer ), 0 );
         WriteFile( file, buffer, rBytes, nullptr, nullptr );
         receivedBytes += rBytes;
      }
      CloseHandle( file );
   }
   return true;
}

void enumerateFilesInFolder( std::string path, Json* files )
{
   WIN32_FIND_DATAA data;
   Json nestedFiles;
   HANDLE hfile = FindFirstFileA( ( path + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         nestedFiles.push_back( data.cFileName );
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            enumerateFilesInFolder( path + "\\" + data.cFileName, &nestedFiles );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   files->push_back( nestedFiles );
}

jsonrpcpp::response_ptr RequestHandler::filelist( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string fPath = params.get( 0 );
   Json files;
   WIN32_FIND_DATAA data;

   HANDLE hfile = FindFirstFileA( ( std::string( fPath ) + "\\*.*" ).c_str(), &data );
   bool res = true;
   while( res )
   {
      if( strcmp( data.cFileName, "." ) != 0 && strcmp( data.cFileName, ".." ) != 0 )
      {
         files.push_back( data.cFileName );
         if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            enumerateFilesInFolder( fPath + "\\" + data.cFileName, &files );
      }
      res = FindNextFileA( hfile, &data );
   }
   FindClose( hfile );
   return std::make_shared<jsonrpcpp::Response>( id, files );
}

jsonrpcpp::response_ptr RequestHandler::download( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   bool res = sendFile( pathSrc );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}

jsonrpcpp::response_ptr RequestHandler::upload( const jsonrpcpp::Id& id, const jsonrpcpp::Parameter& params )
{
   std::string pathSrc = params.get( 0 );
   bool isDir = params.get( 1 );
   send( m_clientSocket, "ok", 2, 0 );
   bool res = receiveFile( pathSrc, isDir );
   return std::make_shared<jsonrpcpp::Response>( id, res );
}

jsonrpcpp::response_ptr RequestHandler::isExist( const jsonrpcpp::Id & id, const jsonrpcpp::Parameter & params )
{
   std::string pathSrc = params.get( 0 );
   std::string res = "false";
   DWORD dwAttr = GetFileAttributesA( pathSrc.c_str() );
   if( dwAttr != INVALID_FILE_ATTRIBUTES )
      res = "true";
   if( dwAttr != 0xffffffff && ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
      res = "dir";
   return std::make_shared<jsonrpcpp::Response>( id, res );
}

void RequestHandler::ulongToCharArray( unsigned long & number, char * arr )
{
   arr[0] = ( number >> 24 );
   arr[1] = ( number >> 16 );
   arr[2] = ( number >> 8 );
   arr[3] = number;
}

void RequestHandler::charArrayToUlong( unsigned long & number, char * arr )
{
   number = 0;
   number |= ( arr[0] << 24 ) & 0xff000000;
   number |= ( arr[1] << 16 ) & 0x00ff0000;
   number |= ( arr[2] << 8 ) & 0x0000ff00;
   number |= arr[3] & 0x000000ff;
}

jsonrpcpp::response_ptr RequestHandler::parseRequest( std::string str, SOCKET clSocket )
{
   m_clientSocket = clSocket;
   jsonrpcpp::Parser parser;
   jsonrpcpp::entity_ptr entity;
   try
   {
      entity = parser.parse( str );
   }
   catch( jsonrpcpp::ParseErrorException )
   {
      return nullptr;
   }
   if( entity->is_request() )
   {
      jsonrpcpp::request_ptr request = std::dynamic_pointer_cast<jsonrpcpp::Request>( entity );
      if( request->method == "filelist" )
      {
         request->params.param_array.push_back( m_folderPath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( filelist( 0, request->params ) );
      }
      else if( request->method == "exist" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( m_folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( isExist( 0, request->params ) );
      }
      else if( request->method == "download" )
      {
         std::string srcpath = request->params.get( 0 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( m_folderPath ) + "\\" + srcpath );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( download( 0, request->params ) );
      }
      else if( request->method == "upload" )
      {
         std::string srcpath = request->params.get( 0 );
         bool isDir = request->params.get( 1 );
         request->params.param_array.clear();
         request->params.param_array.push_back( std::string( m_folderPath ) + "\\" + srcpath );
         request->params.param_array.push_back( isDir );
         return std::dynamic_pointer_cast<jsonrpcpp::Response>( upload( 0, request->params ) );
      }
      else
      {
         return nullptr;
      }
   }
   return nullptr;
}

void RequestHandler::setFolderPath( std::string fPath )
{
   m_folderPath = fPath;
}

std::string RequestHandler::getFolderPath()
{
   return m_folderPath;
}
