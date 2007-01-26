#pragma once
// SQL Server 2005 Express has a version number of 9.00.2047.00 in the CurrentVersion reg value below.

#include <tchar.h>

// Internal method called indirectly:
bool TestSqlSILFWPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
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
#if !defined NOLOGGING
	g_Log.Write(_T("Found SQL Server::SILFW version %s"), szCurrentVersion);
#endif

	return VersionInRange(szCurrentVersion, pszMinVersion, pszMaxVersion);
}
