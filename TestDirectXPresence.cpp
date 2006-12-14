#pragma once

#include <tchar.h>

// Test for DirectX
bool TestDirectXPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						 const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;

	// Test for presence of a version number within range:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\DirectX"), NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVerSize = 128;
		TCHAR szVer[knVerSize];
		DWORD cbData = knVerSize;

		lResult = RegQueryValueEx(hKey, _T("Version"), NULL, NULL, (LPBYTE)szVer, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
#if !defined NOLOGGING
			g_Log.Write(_T("Found DirectX version %s"), szVer);
#endif
			if (VersionInRange(szVer, pszMinVersion, pszMaxVersion))
				fResult = true;
		}
	}
	return fResult;
}