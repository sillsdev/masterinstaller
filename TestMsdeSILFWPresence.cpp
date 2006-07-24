#pragma once

#include <tchar.h>

// Global instantiation:
bool g_fExistingMsdeNeedsUpgrade = false;

// Internal method called indirectly:
bool TestMsdeSILFWPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						   const _TCHAR * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\MSSQLServer\\CurrentVersion"), NULL,
		KEY_READ, &hKey);

	// We don't proceed unless the call above succeeds:
	if (ERROR_SUCCESS != lResult)
		return false;

	const int knCurrentVersionLen = 128;
	_TCHAR szCurrentVersion[knCurrentVersionLen];
	DWORD cbData = knCurrentVersionLen;

	lResult = RegQueryValueEx(hKey, _T("CurrentVersion"), NULL, NULL, (LPBYTE)szCurrentVersion,
		&cbData);

	if (ERROR_SUCCESS != lResult)
	{
		RegCloseKey(hKey);
		return false;
	}

	g_Log.Write(_T("Found MSDE::SILFW version %s"), szCurrentVersion);

	// See if Service Pack 4 is installed:
	g_fExistingMsdeNeedsUpgrade = true; // Assume not, initially
	DWORD nSpVersion;
	cbData = sizeof(nSpVersion);

	lResult = RegQueryValueEx(hKey, _T("CSDVersionNumber"), NULL, NULL, (LPBYTE)(&nSpVersion),
		&cbData);
	RegCloseKey(hKey);

	if (ERROR_SUCCESS == lResult)
	{
		g_Log.Write(_T("MSDE service pack level = %X"), nSpVersion);
		if (nSpVersion < 0x00000400)
			return false;
		else
			g_fExistingMsdeNeedsUpgrade = false;
	}

	return VersionInRange(szCurrentVersion, pszMinVersion, pszMaxVersion);
}
