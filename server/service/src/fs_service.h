#pragma once

#include "ServiceBase.h"
#include "socket_serve.h"
#include <string>

class CFileSharingService : public CServiceBase
{
public:
	CFileSharingService( PWSTR pszServiceName,
		BOOL fCanStop = TRUE,
		BOOL fCanShutdown = TRUE,
		BOOL fCanPauseContinue = FALSE );
	virtual ~CFileSharingService( void );
	virtual void OnStart( DWORD dwArgc, PWSTR* pszArgv );

protected:
	virtual void OnStop();
	void ServiceWorkerThread( void );
private:
	void parseString( LPWSTR* inputString );
	void serveClient( SOCKET clSocket );
	CSocketServer m_server;
	ULONG m_ipAddress;
	ULONG m_port;
	std::wstring m_filePath;
	std::wstring m_serviceName;
	BOOL m_fStopping;
	HANDLE m_hStoppedEvent;
};