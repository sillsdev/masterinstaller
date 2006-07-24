#pragma once

#include <tchar.h>

// Crude test of Toolbox 1.4 installation
bool TestToolbox14Presence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		_TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Toolbox_is1"),
		NULL, KEY_READ, &hKey);

	RegCloseKey(hKey);

	return (ERROR_SUCCESS == lResult);
}
