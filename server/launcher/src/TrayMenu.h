#pragma once
#include <Windows.h>
#include <string>
typedef std::basic_string<TCHAR> tstring;
class TrayMenu
{
public:
	TrayMenu(HWND parent,HICON icon,UINT flags,UINT id);
	void addMenuItem( UINT id, tstring name );
	void removeMenuItem( UINT id, bool byPosition );
	void eventHandler(LPARAM lParam);
private:
	NOTIFYICONDATA iconData;
	HMENU popMenu;
};

