#pragma once

#include <tchar.h>

// Tests for presence of Service Packs for Microsoft .NET 1.1
bool TestDotNet2point0SPPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
								 const TCHAR * /*pszCriticalFile*/)
{
	DWORD nMinVersion = 0;
	DWORD nMaxVersion = 0xFFFFFFFF;
	if (pszMinVersion)
		nMinVersion = (DWORD)_tstoi(pszMinVersion);
	if (pszMaxVersion)
		nMaxVersion = (DWORD)_tstoi(pszMaxVersion);

	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"), NULL, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD nSP;
		DWORD cbData = sizeof(nSP);

		lResult = RegQueryValueEx(hKey, _T("SP"), NULL, NULL, (LPBYTE)&nSP, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			if (nSP >= nMinVersion && nSP <= nMaxVersion)
                return true;
		}
	}
	return false;
}
