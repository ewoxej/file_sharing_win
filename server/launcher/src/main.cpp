#include "..\\res\resource.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <codecvt>
#include <shlobj.h>
#pragma comment(lib,"Shell32.lib")
#include <tchar.h>
#include <string>
#include <registry_helper.hpp>

#define SERVICE_NAME             L"FileSharingCustomCppService"
#define REGISTRY_FS_ROOT L"S-1-5-19\\Software\\FileSharingService"

HINSTANCE hInst;

struct FSServiceData
{
	DWORD longIP;
	UINT port = 800;
	char serviceName[MAX_PATH];
	char folderPath[MAX_PATH];
} g_serviceData;

struct TrayData
{
	NOTIFYICONDATA icon;
	HMENU popMenu;
} g_trayIconData;

void createTrayIcon( NOTIFYICONDATA& Icon, HMENU& popupMenu, HWND parent );
INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );
int GetServiceStatus( TCHAR* name );
void editboxEnabling( HWND dlg, BOOL enabled );
void updateServiceState( TCHAR* servName, HWND item, HWND runItemKey, HWND installItemKey );

LRESULT CALLBACK WndProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_USER+1:
	{
		POINT pt;
		GetCursorPos( &pt );
		if (lParam == WM_RBUTTONDOWN)
		{
			TrackPopupMenu( g_trayIconData.popMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, window, nullptr );
		}
	}
	break;
	case WM_COMMAND:
	{
		switch (static_cast<UINT>(LOWORD( wParam )))
		{
		case IDB_SETTING:
		{
			DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), window, DlgProc );
		}
		break;
		case IDB_HELP:
		{
			MessageBox( window, L"This is a file sharing server. Configure it before running", L"Help", MB_OK );
		}
		break;
		case IDB_EXIT:
		{
			PostQuitMessage( 0 );
		}
		break;
		}
	}
	case WM_DESTROY:
	{
		//PostQuitMessage( 0 );
		break;
	}
	default:
		return DefWindowProc( window, message, wParam, lParam );
	}
	return 0;
}

int APIENTRY _tWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPTSTR cmdLine, int showCmd )
{
	hInst = instance;
	WNDCLASSEX main = { 0 };
	main.cbSize = sizeof( WNDCLASSEX );
	main.hInstance = instance;
	main.lpszClassName = TEXT( "Main" );
	main.lpfnWndProc = WndProc;
	RegisterClassEx( &main );
	GetCurrentDirectoryA( MAX_PATH, g_serviceData.folderPath );
	HWND window = CreateWindowEx( 0, TEXT( "Main" ), nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr );
	createTrayIcon( g_trayIconData.icon, g_trayIconData.popMenu, window );
	g_serviceData.longIP = MAKEIPADDRESS( 127, 0, 0, 1 );
	MSG message;
	while (GetMessage( &message, nullptr, 0, 0 ))
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
	}
	Shell_NotifyIcon( NIM_DELETE, &g_trayIconData.icon );
	return 0;
}

void initialSet( HWND hwndDlg )
{
	LONG style = GetWindowLong( GetDlgItem( hwndDlg, IDC_EDIT3 ), GWL_STYLE );
	SetWindowLongPtr( GetDlgItem( hwndDlg, IDC_EDIT3 ), GWL_STYLE, style | ES_NUMBER );
	SendMessage( GetDlgItem( hwndDlg, IDC_EDIT3 ), EM_SETLIMITTEXT, 4, 0 );
	SendMessage( GetDlgItem( hwndDlg, IDC_EDIT3 ), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(std::to_wstring( g_serviceData.port ).c_str()) );
	SendMessage( GetDlgItem( hwndDlg, IDC_IPADDRESS1 ), IPM_SETADDRESS, 0, static_cast<LPARAM>(g_serviceData.longIP) );
	SendMessageA( GetDlgItem( hwndDlg, IDC_EDIT2 ), WM_SETTEXT, 0, reinterpret_cast<LPARAM>(g_serviceData.serviceName) );
	SetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT1 ), g_serviceData.folderPath );
}

void restoreConfig()
{
	WCHAR abc[128];
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	PWCHAR buffer = abc;
	std::wstring wstr;
	if (readStringFromRegistry( HKEY_USERS, REGISTRY_FS_ROOT, L"IP", &buffer ))
	{
		wstr = std::wstring( buffer );
		g_serviceData.longIP = std::stoll( wstr );
	}
	if (readStringFromRegistry( HKEY_USERS, REGISTRY_FS_ROOT, L"Port", &buffer ))
	{
		wstr = std::wstring( buffer );
		g_serviceData.port = std::stoi( wstr );
	}
	if (readStringFromRegistry( HKEY_USERS, REGISTRY_FS_ROOT, L"ServiceName", &buffer ))
	{
		wstr = std::wstring( buffer );
		strcpy( g_serviceData.serviceName, converter.to_bytes( wstr ).c_str() );
	}
	if (readStringFromRegistry( HKEY_USERS, REGISTRY_FS_ROOT, L"Path", &buffer ))
	{
		wstr = std::wstring( buffer );
		strcpy( g_serviceData.folderPath, converter.to_bytes( wstr ).c_str() );
	}
}

INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	TCHAR buffer[MAX_PATH];
	switch (message)
	{
	case WM_INITDIALOG:
	{
		updateServiceState( SERVICE_NAME, 
			GetDlgItem( hwndDlg, IDC_STATUS ),
			GetDlgItem( hwndDlg, IDB_RUN ),
			GetDlgItem( hwndDlg, IDB_INSTALL ) );
		DWORD bufSize = MAX_PATH;
		GetComputerNameA( g_serviceData.serviceName, &bufSize );
		restoreConfig();
		initialSet( hwndDlg );
		return true;
	}
	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDC_BUTTON1:
		{
			TCHAR current[MAX_PATH];
			BROWSEINFO bis;
			bis.hwndOwner = nullptr;
			bis.pidlRoot = nullptr;
			bis.pszDisplayName = buffer;
			bis.lpszTitle = L"HERE";
			bis.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
			bis.lpfn = nullptr;
			bis.lParam = reinterpret_cast<LPARAM>(current);
			LPITEMIDLIST lst = SHBrowseForFolder( &bis );
			SHGetPathFromIDListA( lst, g_serviceData.folderPath );
			SetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT1 ), g_serviceData.folderPath );
		}
		break;
		case IDB_INSTALL:
		{
			//install/unistall
			if (GetServiceStatus( SERVICE_NAME ) == SERVICE_RUNNING || GetServiceStatus( SERVICE_NAME ) == SERVICE_STOPPED)
				system( "fservice.exe -remove" );
			else
				system( "fservice.exe -install" );

			updateServiceState( SERVICE_NAME,
				GetDlgItem( hwndDlg, IDC_STATUS ),
				GetDlgItem( hwndDlg, IDB_RUN ),
				GetDlgItem( hwndDlg, IDB_INSTALL ) );

		}
		break;
		case IDB_RUN:
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			if (GetServiceStatus( SERVICE_NAME ) == SERVICE_RUNNING)
			{
				editboxEnabling( hwndDlg, true );
				std::string query = "sc stop " + converter.to_bytes( SERVICE_NAME );
				system( query.c_str() );
			}
			else
			{
				editboxEnabling( hwndDlg, false );
				const int portArraySize = 5;
				char cport[portArraySize];
				SendMessage( GetDlgItem( hwndDlg, IDC_IPADDRESS1 ),
					IPM_GETADDRESS, 0, LPARAM( &g_serviceData.longIP ) );
				GetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT2 ), g_serviceData.serviceName, MAX_PATH );
				GetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT3 ), cport, portArraySize );
				g_serviceData.port = std::stoi( cport );
				std::string text = 
					std::to_string( g_serviceData.longIP ) +
					" " + std::to_string( g_serviceData.port ) +
					" " + g_serviceData.serviceName +
					" " + g_serviceData.folderPath;
				std::string query = "sc start " + converter.to_bytes( SERVICE_NAME ) + " " + text;
				system( query.c_str() );
			}
			updateServiceState( SERVICE_NAME,
				GetDlgItem( hwndDlg, IDC_STATUS ),
				GetDlgItem( hwndDlg, IDB_RUN ),
				GetDlgItem( hwndDlg, IDB_INSTALL ) );
		}
		break;
		case WM_DESTROY:
			EndDialog( hwndDlg, 0 );
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void createTrayIcon(NOTIFYICONDATA& Icon,HMENU& popupMenu,HWND parent)
{
	Icon.cbSize = sizeof( NOTIFYICONDATA );
	Icon.hWnd = parent;
	Icon.uVersion = NOTIFYICON_VERSION;
	Icon.uCallbackMessage = WM_USER+1;
	Icon.hIcon = LoadIcon( nullptr, IDI_WINLOGO );
	Icon.uFlags = NIF_MESSAGE | NIF_ICON;
	Shell_NotifyIcon( NIM_ADD, &Icon );
	popupMenu = CreatePopupMenu();
	TCHAR strhelp[] = L"Help";
	TCHAR strset[] = L"Setting";
	TCHAR strexit[] = L"Exit";
	AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_SETTING, strset );
	AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_HELP, strhelp );
	AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_EXIT, strexit );
	MSG message;
}

int GetServiceStatus( TCHAR* name )
{
	SC_HANDLE theService, scm;
	SERVICE_STATUS m_SERVICE_STATUS;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwBytesNeeded;

	scm = OpenSCManager( nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE );
	if (!scm) {
		return 0;
	}

	theService = OpenService( scm, name, SERVICE_QUERY_STATUS );
	if (!theService) {
		CloseServiceHandle( scm );
		return 0;
	}

	auto result = QueryServiceStatusEx( theService, SC_STATUS_PROCESS_INFO,
		reinterpret_cast<LPBYTE>(&ssStatus), sizeof( SERVICE_STATUS_PROCESS ),
		&dwBytesNeeded );

	CloseServiceHandle( theService );
	CloseServiceHandle( scm );

	if (result == 0) {
		return 0;
	}

	return ssStatus.dwCurrentState;
}

void editboxEnabling( HWND dlg, BOOL enabled )
{
	EnableWindow( GetDlgItem( dlg, IDC_EDIT1 ), enabled );
	EnableWindow( GetDlgItem( dlg, IDC_EDIT2 ), enabled );
	EnableWindow( GetDlgItem( dlg, IDC_EDIT3 ), enabled );
	EnableWindow( GetDlgItem( dlg, IDC_IPADDRESS1 ), enabled );
	EnableWindow( GetDlgItem( dlg, IDC_BUTTON1 ), enabled );
}

void updateServiceState( TCHAR* servName, HWND item, HWND runItemKey, HWND installItemKey )
{
	int servStat = GetServiceStatus( servName );
	std::string msgStat = "Not installed";
	std::string runItemText = "Run";
	std::string installItemText = "Unistall";
	EnableWindow( runItemKey, true );
	if (servStat == SERVICE_STOPPED)
	{
		msgStat = "Stopped";
	}
	else if (servStat == SERVICE_RUNNING)
	{
		msgStat = "Running";
		runItemText = "Stop";
	}
	else
	{
		EnableWindow( runItemKey, false );
		installItemText = "Install";
	}
	SetWindowTextA( item, msgStat.c_str() );
	SetWindowTextA( runItemKey, runItemText.c_str() );
	SetWindowTextA( installItemKey, installItemText.c_str() );
}


