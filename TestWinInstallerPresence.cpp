#pragma once

#include <tchar.h>

// Tests for the presence of Windows Installer. Here, pszMinVersion is treated as the
// minimum acceptable version number, and pszMaxVersion is ignored.
bool TestWinInstallerPresence(const TCHAR * pszMinVersion, const TCHAR * /*pszMaxVersion*/,
							  const TCHAR * /*pszCriticalFile*/)
{
	// See if we can detect where the Windows Installer .exe file is:
	TCHAR * szLoc = GetInstallerLocation();
	if (!szLoc)
		return false; // Couldn't find it.

	bool fResult = false;

	// Now add file name to path:
	TCHAR * szLocation = new_sprintf(_T("%s\\msiexec.exe"), szLoc);
	delete[] szLoc;
	szLoc = NULL;

	__int64 nVersion;
	if (GetFileVersion(szLocation, nVersion))
	{
#if !defined NOLOGGING
		TCHAR * pszVersion = GenVersionText(nVersion);
		g_Log.Write(_T("Found msiexec.exe version %s"), pszVersion);
		delete[] pszVersion;
		pszVersion = NULL;
#endif

		fResult = VersionInRange(nVersion, pszMinVersion, NULL);
	}

	delete[] szLocation;
	szLocation = NULL;

	return fResult;
}
