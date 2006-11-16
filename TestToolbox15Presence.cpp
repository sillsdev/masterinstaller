#pragma once

#include <tchar.h>

// Crude test of Toolbox 1.5 installation
bool TestToolbox15Presence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;
	const TCHAR * pszTemplateDisplayName = _T("Toolbox 1.5");

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Toolbox_is1"),
		NULL, KEY_READ, &hKey);

	// Read display name:
	if (ERROR_SUCCESS == lResult)
	{
		_TCHAR * pszDisplayName = NULL;
		DWORD cbData = 0;

		// Find out how much space we need to hold display name:
		lResult = RegQueryValueEx(hKey, _T("DisplayName"), NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			pszDisplayName = new TCHAR [cbData];
			RegQueryValueEx(hKey, _T("DisplayName"), NULL, NULL, (LPBYTE)pszDisplayName, &cbData);

			if (ERROR_SUCCESS == lResult)
			{
				if (_tcsnicmp(pszDisplayName, pszTemplateDisplayName, _tcslen(pszTemplateDisplayName)) == 0)
					fResult = true;
			}

			delete[] pszDisplayName;
			pszDisplayName = NULL;
		}
	}

	RegCloseKey(hKey);

	return fResult;
}
