#pragma region Includes
#include <WinSock2.h>
#include "fs_service.h"
#include "ThreadPool.h"
#include <codecvt>
#include "rpc_handler.h"
#include <registry_helper.hpp>
#pragma endregion

void CFileSharingService::serveClient( SOCKET clSocket )
{
	const int bufferSize = 1024;
	const int indent = 3;
	SOCKET clientSocket;
	RequestHandler handler;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	handler.setFolderPath( converter.to_bytes( m_filePath ) );
	clientSocket = clSocket;
	char buff[bufferSize];
	send( clientSocket, converter.to_bytes( m_serviceName ).c_str(), sizeof( m_serviceName ), 0 );
	int bytesRecv = 0;
	while (bytesRecv != SOCKET_ERROR)
	{
		buff[0] = 0;
		bytesRecv = recv( clientSocket, &buff[0], sizeof( buff ) - 1, 0 );
		if (bytesRecv > 0)
		{
			buff[bytesRecv] = 0;
		}

		jsonrpcpp::response_ptr resp = handler.parseRequest( buff, clSocket );
		if (!resp)
		{
			strcpy( buff, "error: incorrect request" );
			send( clientSocket, buff, strlen( buff ), 0 );
		}
		else
		{
			std::string strRes = resp->result.dump( indent );
			send( clientSocket, strRes.c_str(), strRes.length(), 0 );
		}
	}

	closesocket( clientSocket );
}

CFileSharingService::CFileSharingService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}


CFileSharingService::~CFileSharingService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}

bool dirExists( const std::wstring& dirName_in )
{
	DWORD ftyp = GetFileAttributes( dirName_in.c_str() );
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool writeRegistryInfo( wchar_t* argv[] )
{
	bool ok = CreateRegistryKey( HKEY_CURRENT_USER, L"Software\\FileSharingService" ); //create key
	if (ok != TRUE) return FALSE;
	ok = writeStringInRegistry( HKEY_CURRENT_USER, L"Software\\FileSharingService", L"IP", argv[1] ); //write string
	if (ok != TRUE) return FALSE;
	ok = writeStringInRegistry( HKEY_CURRENT_USER, L"Software\\FileSharingService", L"Port", argv[2] ); //write string
	if (ok != TRUE) return FALSE;
	ok = writeStringInRegistry( HKEY_CURRENT_USER, L"Software\\FileSharingService", L"ServiceName", argv[3] ); //write string
	if (ok != TRUE) return FALSE;
	ok = writeStringInRegistry( HKEY_CURRENT_USER, L"Software\\FileSharingService", L"Path", argv[4] ); //write string
	if (ok != TRUE) return FALSE;
}

void CFileSharingService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);
	writeRegistryInfo( lpszArgv );
	parseString( lpszArgv );
	m_server.connectTo( m_ipAddress, m_port );

    CThreadPool::QueueUserWorkItem(&CFileSharingService::ServiceWorkerThread, this);
}

void CFileSharingService::ServiceWorkerThread(void)
{
    // Periodically check if the service is stopping.
    while (!m_fStopping)
    {
		m_server.accept( [this]( SOCKET sk ) {serveClient( sk ); } );
    }

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}

void CFileSharingService::parseString( LPWSTR* inputString )
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	m_ipAddress = htonl(inet_addr( converter.to_bytes(inputString[1]).c_str() ));
	m_port = std::stoi( inputString[2] );
	m_serviceName = inputString[3];
	m_filePath = inputString[4];
	if (!dirExists( m_filePath )) throw std::invalid_argument( "Invalid path was provided" );
}

void CFileSharingService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    m_fStopping = TRUE;
	m_server.setWorking( false );
	shutdown( m_server.getActiveSocket(), SD_BOTH );
	closesocket( m_server.getActiveSocket() );
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}