#pragma once

#include <tchar.h>

// Tests for the presence of MDAC.
bool TestMDACPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
					  const TCHAR * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DataAccess"), NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionLen = 256;
		TCHAR szVersion[knVersionLen];
		DWORD cbData = knVersionLen;

		lResult = RegQueryValueEx(hKey, _T("FullInstallVer"), NULL, NULL, (LPBYTE)szVersion,
			&cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS != lResult)
			return false;

		g_Log.Write(_T("Found MDAC version %s"), szVersion);
		return VersionInRange(szVersion, pszMinVersion, pszMaxVersion);
	}
	return false;
}
