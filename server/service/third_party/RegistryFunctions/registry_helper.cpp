#include "registry_helper.hpp"

#define TOTAL_BYTES_READ    1024
#define OFFSET_BYTES 1024

BOOL CreateRegistryKey(HKEY hKeyParent,PWCHAR subkey)
{
	DWORD dwDisposition; //It verify new key is created or open existing key
	HKEY  hKey;
	DWORD Ret;


	Ret =
		RegCreateKeyEx(
		hKeyParent,
		subkey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition);

	if (Ret != ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegCloseKey(hKey); //close the key
	return TRUE;
}

BOOL writeStringInRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR strData)
{
	DWORD Ret;
	HKEY hKey;

	//Check if the registry exists
	Ret = RegOpenKeyEx(
		hKeyParent,
		subkey,
		0,
		KEY_WRITE,
		&hKey
		);

	if (Ret == ERROR_SUCCESS)
	{
		if (ERROR_SUCCESS !=
			RegSetValueEx(
			hKey,
			valueName,
			0,
			REG_SZ,
			(LPBYTE)(strData),
			((((DWORD)lstrlen(strData) + 1)) * 2)))
		{
			RegCloseKey(hKey);
			return FALSE;
		}

		RegCloseKey(hKey);
		return TRUE;
	}

	return FALSE;
}

BOOL readDwordValueRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD *readData)
{

	HKEY hKey;
	DWORD Ret;

	//Check if the registry exists
	Ret = RegOpenKeyEx(
		hKeyParent,
		subkey,
		0,
		KEY_READ,
		&hKey
		);

	if (Ret == ERROR_SUCCESS)
	{

		DWORD data;
		DWORD len = sizeof(DWORD);//size of data

		Ret = RegQueryValueEx(
			hKey,
			valueName,
			NULL,
			NULL,
			(LPBYTE)(&data),
			&len
			);

		if (Ret == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			(*readData) = data;
			return TRUE;
		}
		
		RegCloseKey(hKey);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL readStringFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR *readData)
{
	HKEY hKey;
	DWORD len = TOTAL_BYTES_READ;

	DWORD readDataLen = len;

	PWCHAR readBuffer = (PWCHAR )malloc(sizeof(PWCHAR)* len);
	if (readBuffer == NULL)
		return FALSE;

	//Check if the registry exists
	DWORD Ret = RegOpenKeyEx(
		hKeyParent,
		subkey,
		0,
		KEY_READ,
		&hKey
		);

	if (Ret == ERROR_SUCCESS)
	{
	
		Ret = RegQueryValueEx(
			hKey,
			valueName,
			NULL,
			NULL,
			(BYTE*)readBuffer,
			&readDataLen
			);

		while (Ret == ERROR_MORE_DATA)
		{
			// Get a buffer that is big enough.

			len += OFFSET_BYTES;
			readBuffer = (PWCHAR)realloc(readBuffer, len);
			readDataLen = len;

			Ret = RegQueryValueEx(
				hKey,
				valueName,
				NULL,
				NULL,
				(BYTE*)readBuffer,
				&readDataLen
				);
		}

		if (Ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return false;;
		}

		*readData = readBuffer;

		RegCloseKey(hKey);
		return true;
	}
	else
	{
		return false;
	}
}