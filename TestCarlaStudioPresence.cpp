#pragma once

#include <tchar.h>

// Crude test of Carla Studio installation
bool TestCarlaStudioPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
							 const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CarlaStudio"),
		NULL, KEY_READ, &hKey);

	RegCloseKey(hKey);

	return (ERROR_SUCCESS == lResult);
}
