#pragma once

#include <tchar.h>

// Test for Carla Studio installation. Uses the path in the registry for product uninstallation
// to get the version of the main .exe file
bool TestCarlaStudioPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
							 SoftwareProduct * /*Product*/)
{
	bool fResult = false;
	HKEY hKey = NULL;

	// Get the uninstall string:
	_TCHAR * pszUninstallString = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CarlaStudioUnicode"),
		_T("UninstallString"));

	if (pszUninstallString)
	{
		// Parse the UninstallString to get the installation path. It typically looks like this:
		// C:\WINDOWS\IsUninst.exe -f"C:\Program Files\SIL\CarlaStudioUnicode\Uninst.isu"
		const _TCHAR * pszFlag = _T("-f");
		_TCHAR * pchFlagStart = _tcsstr(pszUninstallString, pszFlag);
		if (pchFlagStart)
		{
			_TCHAR * pszInstallPath = pchFlagStart + _tcslen(pszFlag);
			// Check for a quoted string - bypass quote marks if present:
			if (_tcsnccmp(pszInstallPath, _T("\""), 1) == 0)
			{
				pszInstallPath++;
				size_t cch = _tcslen(pszInstallPath) - 1;
				if (_tcscmp(&pszInstallPath[cch], _T("\"")) == 0)
					pszInstallPath[cch] = 0;
			}
			// Now find main .exe file and get its version:
			RemoveLastPathSection(pszInstallPath);
			_TCHAR * pszMainExePath = MakePath(pszInstallPath, _T("CStudioU.exe"));

			// See if the file exists and what version it is:
			__int64 nVersion;
			bool fFound = GetFileVersion(pszMainExePath, nVersion);
			delete[] pszMainExePath;
			pszMainExePath = NULL;

			if (fFound)
			{
#if !defined NOLOGGING
				_TCHAR * pszVersion = GenVersionText(nVersion);
				g_Log.Write(_T("Found Carla Studio version %s"), pszVersion);
				delete[] pszVersion;
				pszVersion = NULL;
#endif
				fResult = VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
			}
		}
		delete[] pszUninstallString;
		pszUninstallString = NULL;
	}

	return (fResult);
}
