#include "CFileSharingServiceData.h"
#include <stdexcept>
#include <iostream>
#include <registry_helper.hpp>
#include <CommCtrl.h>

CFileSharingServiceData::CFileSharingServiceData()
	: port(800)
	, longIP( MAKEIPADDRESS( 127, 0, 0, 1 ) )
{
	CHAR buffer[128];
	DWORD bufSize = 128;
	GetComputerNameA( buffer, &bufSize );
	serviceName = converter.from_bytes(buffer);
	GetCurrentDirectoryA( 128, buffer );
	folderPath = converter.from_bytes( buffer );
}

void CFileSharingServiceData::setIP( long value )
{
	longIP = value;
}

void CFileSharingServiceData::setIP( std::string value )
{
	try
	{
		longIP = std::stoll( value );
	}
	catch (std::invalid_argument& e)
	{
		std::cerr << e.what();
	}
}

void CFileSharingServiceData::setIP( std::wstring value )
{
	try
	{
		longIP = std::stoll( value );
	}
	catch (std::invalid_argument& e)
	{
		std::cerr << e.what();
	}
}

void CFileSharingServiceData::setPort( unsigned int port )
{
	this->port = port;
}

void CFileSharingServiceData::setPort( std::string port )
{
	try
	{
		this->port = std::stoi( port );
	}
	catch (std::invalid_argument& e)
	{
		std::cerr << e.what();
	}
}

void CFileSharingServiceData::setPort( std::wstring port )
{
	try
	{
		this->port = std::stoi( port );
	}
	catch (std::invalid_argument& e)
	{
		std::cerr << e.what();
	}
}

void CFileSharingServiceData::setServiceName( std::string serviceName )
{
	this->serviceName = converter.from_bytes(serviceName);
}

void CFileSharingServiceData::setServiceName( std::wstring serviceName )
{
	this->serviceName = serviceName;
}


void CFileSharingServiceData::setFolderPath( std::string serviceName )
{
	folderPath = converter.from_bytes( serviceName );
}

void CFileSharingServiceData::setFolderPath( std::wstring serviceName )
{
	folderPath = serviceName;
}

long CFileSharingServiceData::getIPLong()
{
	return longIP;
}

std::string CFileSharingServiceData::getIPA()
{
	return std::to_string( longIP );
}

std::wstring CFileSharingServiceData::getIPW()
{
	return std::to_wstring( longIP );
}

tstring CFileSharingServiceData::getIP()
{
#ifdef UNICODE
	return getIPW();
#else
	return getIPA();
#endif
}

unsigned int CFileSharingServiceData::getPortInt()
{
	return port;
}

std::string CFileSharingServiceData::getPortA()
{
	return std::to_string( port );
}

std::wstring CFileSharingServiceData::getPortW()
{
	return std::to_wstring( port );
}

tstring CFileSharingServiceData::getPort()
{
#ifdef UNICODE
	return getPortW();
#else
	return getPortA();
#endif
}

std::string CFileSharingServiceData::getNameA()
{
	return converter.to_bytes( serviceName );
}

std::wstring CFileSharingServiceData::getNameW()
{
	return serviceName;
}

tstring CFileSharingServiceData::getName()
{
#ifdef UNICODE
	return getNameW();
#else
	return getNameA();
#endif
}

std::string CFileSharingServiceData::getPathA()
{
	return converter.to_bytes( folderPath );
}

std::wstring CFileSharingServiceData::getPathW()
{
	return folderPath;
}

tstring CFileSharingServiceData::getPath()
{
#ifdef UNICODE
	return getPathW();
#else
	return getPathA();
#endif
}

void CFileSharingServiceData::restoreConfig(PWCHAR registryKeyPath)
{
	WCHAR wideCharArray[128];
	PWCHAR buffer = wideCharArray;
	if (readStringFromRegistry( HKEY_USERS, registryKeyPath, L"IP", &buffer ))
	{
		setIP( buffer );
	}
	if (readStringFromRegistry( HKEY_USERS, registryKeyPath, L"Port", &buffer ))
	{
		setPort( buffer );
	}
	if (readStringFromRegistry( HKEY_USERS, registryKeyPath, L"ServiceName", &buffer ))
	{
		setServiceName( buffer );
	}
	if (readStringFromRegistry( HKEY_USERS, registryKeyPath, L"Path", &buffer ))
	{
		setFolderPath( buffer );
	}
}
