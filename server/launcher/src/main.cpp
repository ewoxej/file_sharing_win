#include "..\\res\resource.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <codecvt>
#include <shlobj.h>
#pragma comment(lib,"Shell32.lib")
#include <tchar.h>
#include <string>
#include <registry_helper.hpp>
#include "TrayMenu.h"
#include "CFileSharingServiceData.h"
#define SERVICE_NAME             L"FileSharingCustomCppService"
#define REGISTRY_FS_ROOT L"S-1-5-19\\Software\\FileSharingService"

HINSTANCE hInst;

INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );
int GetServiceStatus( TCHAR* name );
void setOnlyNumbers( HWND hwndControl, BOOL value );
void editboxEnabling( HWND dlg, BOOL enabled );
void updateServiceState( TCHAR* servName, HWND item, HWND runItemKey, HWND installItemKey );

LRESULT CALLBACK WndProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
	static TrayMenu tMenu( window, LoadIcon( nullptr, IDI_WINLOGO ), NIF_MESSAGE | NIF_ICON, WM_USER + 1 );
	switch (message)
	{
	case WM_CREATE:
	{
		tMenu.addMenuItem( IDB_SETTING, TEXT("Setting") );
		tMenu.addMenuItem( IDB_HELP, TEXT("Help") );
		tMenu.addMenuItem( IDB_EXIT, TEXT("Exit") );
	}
	break;
	case WM_USER+1:
	{
		tMenu.eventHandler( lParam );
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
	HWND window = CreateWindowEx( 0, TEXT( "Main" ), nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr );
	MSG message;
	while (GetMessage( &message, nullptr, 0, 0 ))
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
	}
	return 0;
}

INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static CFileSharingServiceData data;
	HWND editPathControl = GetDlgItem( hwndDlg, IDC_EDIT1 );
	HWND editNameControl = GetDlgItem( hwndDlg, IDC_EDIT2 );
	HWND editPortControl = GetDlgItem( hwndDlg, IDC_EDIT3 );
	HWND editIpControl = GetDlgItem( hwndDlg, IDC_IPADDRESS1 );
	TCHAR buffer[MAX_PATH];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		updateServiceState( SERVICE_NAME, 
			GetDlgItem( hwndDlg, IDC_STATUS ),
			GetDlgItem( hwndDlg, IDB_RUN ),
			GetDlgItem( hwndDlg, IDB_INSTALL ) );
		data.restoreConfig(REGISTRY_FS_ROOT);
		SetWindowText( editPathControl, data.getPath().c_str() );
		SetWindowText( editNameControl, data.getName().c_str() );
		SetWindowText( editPortControl, data.getPort().c_str() );
		setOnlyNumbers( editPortControl, true );
		SendMessage( editPortControl, EM_SETLIMITTEXT, 5, 0 );
		SendMessage( editIpControl, IPM_SETADDRESS, 0, static_cast<LPARAM>(data.getIPLong()) );
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
			SHGetPathFromIDList( lst, buffer );
			data.setFolderPath( buffer );
			SetWindowText( editPathControl, data.getPath().c_str() );
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
				long tempIp = 0;
				SendMessage( editIpControl,IPM_GETADDRESS, 0, LPARAM( &tempIp ) );
				data.setIP( tempIp );
				GetWindowText( editNameControl, buffer, MAX_PATH );
				data.setServiceName( buffer );
				GetWindowText( editPortControl, buffer, MAX_PATH );
				data.setPort( buffer );
				std::string text = 
					data.getIPA() +
					" " + data.getPortA() +
					" " + data.getNameA() +
					" " + data.getPathA();
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
	tstring msgStat = TEXT("Not installed");
	tstring runItemText = TEXT("Run");
	tstring installItemText = TEXT("Unistall");
	EnableWindow( runItemKey, true );
	if (servStat == SERVICE_STOPPED)
	{
		msgStat = TEXT("Stopped");
	}
	else if (servStat == SERVICE_RUNNING)
	{
		msgStat = TEXT("Running");
		runItemText = TEXT("Stop");
	}
	else
	{
		EnableWindow( runItemKey, false );
		installItemText = TEXT("Install");
	}
	SetWindowText( item, msgStat.c_str() );
	SetWindowText( runItemKey, runItemText.c_str() );
	SetWindowText( installItemKey, installItemText.c_str() );
}

void setOnlyNumbers( HWND hwndControl, BOOL value )
{
	LONG style = GetWindowLong( hwndControl, GWL_STYLE );
	style = (value) ? (style | ES_NUMBER) : (style & ~(ES_NUMBER));
	SetWindowLongPtr( GetDlgItem( hwndControl, IDC_EDIT3 ), GWL_STYLE, style );
}


