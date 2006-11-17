#pragma once

#include <tchar.h>

// Test for Carla Studio installation. Uses the path in the registry for product uninstallation
// to get the version of the main .exe file
bool TestCarlaStudioPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
							 const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;
	HKEY hKey = NULL;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CarlaStudio"),
		NULL, KEY_READ, &hKey))
	{
		// Get the uninstall string:
		DWORD cchUninstallString = 0;
		TCHAR * pszUninstallString = NULL;
		const TCHAR * pszUninstallStringValue = _T("UninstallString");

		if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszUninstallStringValue, NULL, NULL, NULL,
			&cchUninstallString))
		{
			pszUninstallString = new TCHAR [cchUninstallString];
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszUninstallStringValue, NULL, NULL,
				(LPBYTE)pszUninstallString, &cchUninstallString))
			{
				// Parse the UninstallString to get the installation path. It typically looks like this:
				// C:\WINDOWS\IsUninst.exe -f"C:\Program Files\SIL\CarlaStudio\Uninst.isu"
				const TCHAR * pszFlag = _T("-f");
				TCHAR * pchFlagStart = _tcsstr(pszUninstallString, pszFlag);
				if (pchFlagStart)
				{
					TCHAR * pszInstallPath = pchFlagStart + _tcslen(pszFlag);
					// Check for a quoted string - bypass quote marks if present:
					if (_tcsnccmp(pszInstallPath, _T("\""), 1) == 0)
					{
						pszInstallPath++;
						size_t cch = _tcslen(pszInstallPath) - 1;
						if (_tcscmp(&pszInstallPath[cch], _T("\"")) == 0)
							pszInstallPath[cch] = 0;
					}
					// Now find main .exe file and get its version:
					TCHAR * pchLastBackslash = _tcsrchr(pszInstallPath, '\\');
					if (pchLastBackslash)
					{
						*pchLastBackslash = 0;
						TCHAR * pszMainExePath = new_sprintf(_T("%s\\CStudio.exe"), pszInstallPath);

						// See if the file exists and what version it is:
						__int64 nVersion;
						bool fFound = GetFileVersion(pszMainExePath, nVersion);
						delete[] pszMainExePath;
						pszMainExePath = NULL;

						if (fFound)
						{
#if !defined NOLOGGING
							TCHAR * pszVersion = GenVersionText(nVersion);
							g_Log.Write(_T("Found Carla Studio version %s"), pszVersion);
							delete[] pszVersion;
							pszVersion = NULL;
#endif

							fResult = VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
						}
					}
				}
			}
			delete[] pszUninstallString;
			pszUninstallString = NULL;
		}
		RegCloseKey(hKey);
	}

	return (fResult);
}
