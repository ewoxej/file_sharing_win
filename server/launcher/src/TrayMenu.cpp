#include "TrayMenu.h"

TrayMenu::TrayMenu( HWND parent, HICON icon, UINT flags,UINT id )
{
iconData = { 0 };
iconData.cbSize = sizeof( NOTIFYICONDATA );
iconData.hWnd = parent;
iconData.uVersion = NOTIFYICON_VERSION;
iconData.uCallbackMessage = id;
iconData.hIcon = LoadIcon( nullptr, IDI_WINLOGO );
iconData.uFlags = flags;
Shell_NotifyIcon( NIM_ADD, &iconData );
popMenu = CreatePopupMenu();
}

void TrayMenu::addMenuItem( UINT id, tstring name )
{
	AppendMenu( popMenu, MF_STRING | MF_MOUSESELECT, id, name.c_str() );
}

void TrayMenu::removeMenuItem( UINT id, bool byPosition )
{
	RemoveMenu( popMenu, id, (byPosition) ? MF_BYPOSITION : MF_BYCOMMAND );
}

void TrayMenu::eventHandler(LPARAM lParam)
{
	POINT pt;
	GetCursorPos( &pt );
	if (lParam == WM_RBUTTONDOWN)
	{
		TrackPopupMenu( popMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, iconData.hWnd, nullptr );
	}
}
