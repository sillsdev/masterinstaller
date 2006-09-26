#pragma once

#include <tchar.h>

// Crude test of Lexique Pro installation
bool TestLexiquePresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Lexique Pro - Bambara_is1"),
		NULL, KEY_READ, &hKey);

	RegCloseKey(hKey);

	return (ERROR_SUCCESS == lResult);
}
