#pragma once
#include <Windows.h>
#include <codecvt>
#include <string>

typedef std::basic_string<TCHAR> tstring;

class CFileSharingServiceData
{
public:
	CFileSharingServiceData();
	void setIP( long value );
	void setIP( std::string value );
	void setIP( std::wstring value );
	void setPort( unsigned int port );
	void setPort( std::string port );
	void setPort( std::wstring port );
	void setServiceName( std::string serviceName );
	void setServiceName( std::wstring serviceName );
	void setFolderPath( std::string serviceName );
	void setFolderPath( std::wstring serviceName );
	long getIPLong();
	std::string getIPA();
	std::wstring getIPW();
	tstring getIP();
	unsigned int getPortInt();
	std::string getPortA();
	std::wstring getPortW();
	tstring getPort();
	std::string getNameA();
	std::wstring getNameW();
	tstring getName();
	std::string getPathA();
	std::wstring getPathW();
	tstring getPath();
	void restoreConfig( PWCHAR registryKeyPath );
private:
	long longIP;
	unsigned int port = 800;
	std::wstring serviceName;
	std::wstring folderPath;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
};

