#pragma once

#include <tchar.h>

// Test for DirectX
bool TestDirectXPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						 SoftwareProduct * /*Product*/)
{
	bool fResult = false;

	// Test for presence of a version number within range:
	_TCHAR * pszVer = NewRegString(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\DirectX"),
		_T("Version"));

	if (pszVer)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Found DirectX version %s"), pszVer);
#endif
		if (VersionInRange(pszVer, pszMinVersion, pszMaxVersion))
			fResult = true;
	}
	delete[] pszVer;
	pszVer = NULL;

	return fResult;
}