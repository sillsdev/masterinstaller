#pragma once

#include <tchar.h>

// Crude test of Lexique Pro installation - currently only recognizes Lexique Pro 2.6.6
bool TestLexiquePresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	HKEY hKey = NULL;
	const TCHAR * pszTemplateDisplayName = _T("Lexique Pro 2.6.6");

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Lexique Pro - Bambara_is1"),
		NULL, KEY_READ, &hKey))
	{
		DWORD cchAppName = 0;
		TCHAR * pszAppName = NULL;
		const TCHAR * pszAppValue = _T("DisplayName");

		if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszAppValue, NULL, NULL, NULL, &cchAppName))
		{
			pszAppName = new TCHAR [cchAppName];
			RegQueryValueEx(hKey, pszAppValue, NULL, NULL, (LPBYTE)pszAppName, &cchAppName);

			if (_tcscmp(pszAppName, pszTemplateDisplayName) == 0)
				fResult = true;

			delete[] pszAppName;
			pszAppName = NULL;
		}
		RegCloseKey(hKey);
	}
	return fResult;
}
