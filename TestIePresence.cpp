#pragma once

#include <tchar.h>

// Tests for the presence of Internet Explorer.
bool TestIePresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
					const TCHAR * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer"), NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionLen = 256;
		TCHAR szVersion[knVersionLen];
		DWORD cbData = knVersionLen;

		lResult = RegQueryValueEx(hKey, _T("Version"), NULL, NULL, (LPBYTE)szVersion, &cbData);
		RegCloseKey(hKey);

#if !defined NOLOGGING
		g_Log.Write(_T("Found Internet Explorer version %s"), szVersion);
#endif

		if (ERROR_SUCCESS == lResult)
			return VersionInRange(szVersion, pszMinVersion, pszMaxVersion);
	}
	return false;
}
