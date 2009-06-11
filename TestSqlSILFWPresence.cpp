#pragma once
// SQL Server 2005 Express originally had a version number of 9.00.2047.00 in the CurrentVersion reg value below.
// After at least one update, the version number reads 9.00.3042.00

#include <tchar.h>

// Internal method called indirectly:
bool TestSqlSILFWPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszCurrentVersion = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\MSSQLServer\\CurrentVersion"),
		_T("CurrentVersion"));

	if (!pszCurrentVersion)
		return false;

#if !defined NOLOGGING
	g_Log.Write(_T("Found SQL Server::SILFW version %s"), pszCurrentVersion);
#endif

	bool fResult =  VersionInRange(pszCurrentVersion, pszMinVersion, pszMaxVersion);

	delete[] pszCurrentVersion;
	pszCurrentVersion = NULL;

	return fResult;
}
