#pragma once

#include <tchar.h>

#include "Uninstall.cpp"

// Remove any existing updates of TW4
int RemovePreviousTW4Updates(const TCHAR * /*pszCriticalFile*/)
{
	int nResult = 0;

	// First check that TW4 is installed:
	LONG lResult;
	HKEY hKey = NULL;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Translator's Workplace 4"),
		NULL, KEY_ALL_ACCESS, &hKey);

	if (ERROR_SUCCESS != lResult)
		nResult = -2;
	else
	{
		DWORD cbData = 0;
		// Get requirement for buffer length:
		lResult = RegQueryValueEx(hKey, _T("TW4Folder"), NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS != lResult || cbData <= 0)
			nResult = -2;
	}
	if (nResult != 0)
	{
		MessageBox(NULL,
			_T("Translator's Workplace 4 cannot be located on this system, therefore the Update will not be installed."),
			g_pszTitle, MB_OK | MB_ICONSTOP);
		return nResult;
	}

	// List of all previous FW product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{3C9E2078-EBBD-43EF-837F-2AC26AF618BC}"), // TW4 Update 1
		_T("{D938E793-A00B-4849-BCA8-9C25B4A6C081}"), // TW4 Update 2
	};
	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write(_T("Uninstalling earlier TW update (Product code %s)."), pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], _T("Preparing for the new TW update."));
		if (dwResult == 0)
			g_Log.Write(_T("Uninstall succeeded."));
		else
		{
			g_Log.Write(_T("Uninstall failed with error code %d."), dwResult);
			nResult = (int)dwResult;
			return nResult;
		}
	}

	return nResult;
}
