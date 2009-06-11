#pragma once

#include <tchar.h>

// Tests for the presence of Windows Installer. Here, pszMinVersion is treated as the
// minimum acceptable version number, and pszMaxVersion is ignored.
bool TestWinInstallerPresence(const _TCHAR * pszMinVersion, const _TCHAR * /*pszMaxVersion*/,
							  SoftwareProduct * /*Product*/)
{
	// See if we can detect where the Windows Installer .exe file is:
	_TCHAR * pszLoc = GetInstallerLocation();
	if (!pszLoc)
		return false; // Couldn't find it.

	bool fResult = false;

	// Now add file name to path:
	_TCHAR * pszLocation = MakePath(pszLoc, _T("msiexec.exe"));
	delete[] pszLoc;
	pszLoc = NULL;

	__int64 nVersion;
	if (GetFileVersion(pszLocation, nVersion))
	{
#if !defined NOLOGGING
		TCHAR * pszVersion = GenVersionText(nVersion);
		g_Log.Write(_T("Found msiexec.exe version %s"), pszVersion);
		delete[] pszVersion;
		pszVersion = NULL;
#endif

		fResult = VersionInRange(nVersion, pszMinVersion, NULL);
	}

	delete[] pszLocation;
	pszLocation = NULL;

	return fResult;
}
