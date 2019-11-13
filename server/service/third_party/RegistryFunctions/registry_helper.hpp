#pragma once
#include <Windows.h>
BOOL CreateRegistryKey(HKEY hKeyParent,PWCHAR subkey);
BOOL writeStringInRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR strData);
BOOL readDwordValueRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, DWORD *readData);
BOOL readStringFromRegistry(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR *readData);