#pragma once

#include <tchar.h>

// Tests for the presence of MDAC.
bool TestMDACPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
					  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszVersion = NewRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DataAccess"),
		_T("FullInstallVer"));

	if (!pszVersion)
		return false;

#if !defined NOLOGGING
	g_Log.Write(_T("Found MDAC version %s"), pszVersion);
#endif
	bool fResult = VersionInRange(pszVersion, pszMinVersion, pszMaxVersion);
	
	delete[] pszVersion;
	pszVersion = NULL;

	return fResult;
}
