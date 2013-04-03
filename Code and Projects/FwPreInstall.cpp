#pragma once

#include <tchar.h>

#include "RemovePreviousFW7s.cpp"

_TCHAR * _FormerFw7ProjectsFolder = NULL;

// Deal with stuff prior to installing FieldWorks:
int FwPreInstall(SoftwareProduct * Product)
{
	g_Log.Write(_T("FieldWorks pre-install function..."));
	g_Log.Indent();

	// See if we can find where an existing version of FW (from 7.0 onwards) stores its projects
	// (so we can default to that in the new version).

	_TCHAR * pszHighestExistingFwVersion = my_strdup(_T("0.0.0.0"));
	const _TCHAR * kpszFwKey = _T("SOFTWARE\\SIL\\FieldWorks");

	// Enumerate registry keys in HKEY_LOCAL_MACHINE\SOFTWARE\SIL\FieldWorks
	// to find the highest version number:
	HKEY hKey;
	LONG lResult;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, kpszFwKey, NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS != lResult)
		g_Log.Write(_T("Could not open HKLM\\%s"), kpszFwKey);
	else
	{
		const int knVersionNumBufSize = 100;
		_TCHAR szVersionNum[knVersionNumBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionNumBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szVersionNum, &cbData, NULL, NULL, NULL, NULL))
		{
			if (GetHugeVersion(pszHighestExistingFwVersion) < GetHugeVersion(szVersionNum))
			{
				delete[] pszHighestExistingFwVersion;
				pszHighestExistingFwVersion = my_strdup(szVersionNum);
			}

			// Reset data for next iteration:
			cbData = knVersionNumBufSize;
		} // Next key in enumeration

		g_Log.Write(_T("Highest FW version found was %s"), pszHighestExistingFwVersion);

		// If we found any FW version numbers, use the highest one to read the path of the Projects directory:
		if (GetHugeVersion(pszHighestExistingFwVersion) > 0)
		{
			// Open FW versioned reg key:
			HKEY hKeyFwVersion;
			LONG lResultVer = RegOpenKeyEx(hKey, pszHighestExistingFwVersion, 0, KEY_READ, &hKeyFwVersion);
			if (ERROR_SUCCESS == lResultVer)
			{
				// Get ProjectsDir value:
				const int knProjectsDirLen = 1024;
				_TCHAR szProjectsDir[knProjectsDirLen];
				DWORD cbData = knProjectsDirLen;
				lResultVer = RegQueryValueEx(hKeyFwVersion, _T("ProjectsDir"), NULL, NULL, (LPBYTE)szProjectsDir, &cbData);
				RegCloseKey(hKeyFwVersion);

				if (ERROR_SUCCESS == lResultVer)
				{
					g_Log.Write(_T("FW version %s Projects directory was %s."), pszHighestExistingFwVersion, szProjectsDir);
					// We now have the path to the FW projects for the latest installed version.
					// If the latest version was 7.0 (which includes any of the version 7 family)
					// and the project directory is the default (C:\ProgramData\SIL\FieldWorks 7\Projects)
					// Then we will rename that "FieldWorks 7" folder to omit the " 7" and set the
					// corresponding Projects folder as the default:
					if (GetHugeVersion(pszHighestExistingFwVersion) == 0x0007000000000000)
					{
						// Get the CommonAppDataFolder string:
						_TCHAR * pszCommonAppDataFolder = GetFolderPathNew(CSIDL_COMMON_APPDATA);
						// Form default data directory for FW 7:
						_TCHAR * pszFw7DefaultFolder = MakePath(pszCommonAppDataFolder, _T("SIL\\FieldWorks 7"));
						_TCHAR * pszFw7DefaultProjectsFolder = MakePath(pszFw7DefaultFolder, _T("Projects"));
						_TCHAR * pszFw7DefaultProjectsFolderWithExtraSlash = new_sprintf(_T("%s\\"), pszFw7DefaultProjectsFolder);
						delete[] pszFw7DefaultFolder;

						if (_tcsicmp(pszFw7DefaultProjectsFolder, szProjectsDir) == 0
							|| _tcsicmp(pszFw7DefaultProjectsFolderWithExtraSlash, szProjectsDir) == 0)
						{
							g_Log.Write(_T("Existing FW projects folder is default for FW 7 [%s]. Will store value for later."), pszFw7DefaultProjectsFolder);

							_FormerFw7ProjectsFolder = my_strdup(pszFw7DefaultProjectsFolder);

							_TCHAR * pszNewFwDefaultFolder = MakePath(pszCommonAppDataFolder, _T("SIL\\FieldWorks"));
							_TCHAR * pszNewFwDefaultProjectsFolder = MakePath(pszNewFwDefaultFolder, _T("Projects\\"));
							delete[] pszCommonAppDataFolder;

							g_Log.Write(_T("Adjusting default Projects folder for FW installer from %s to %s."), szProjectsDir, pszNewFwDefaultProjectsFolder);
							_tcscpy_s(szProjectsDir, knProjectsDirLen, pszNewFwDefaultProjectsFolder);

							delete[] pszNewFwDefaultFolder;
							delete[] pszNewFwDefaultProjectsFolder;
						}
						else 
							g_Log.Write(_T("Existing FW projects folder [%s] is not the default for FW 7. No need to remember - not contemplating moving projects."), pszFw7DefaultProjectsFolder);
						delete[] pszFw7DefaultProjectsFolder;
						delete[] pszFw7DefaultProjectsFolderWithExtraSlash;
					}

					// Store it in an environment variable (which will only endure for this process):
					SetEnvironmentVariable(_T("FWINSTALLERPROJDIRDEFAULT"), szProjectsDir);
					g_Log.Write(_T("Writing environment variable FWINSTALLERPROJDIRDEFAULT as %s."), szProjectsDir);					
				}
				else
					g_Log.Write(_T("Could not read FW version %s Projects directory"), pszHighestExistingFwVersion);
			}
			else
				g_Log.Write(_T("Could not open %s registry key under HKLM\\%s"), pszHighestExistingFwVersion, kpszFwKey);
		}

		RegCloseKey(hKey);

		delete[] pszHighestExistingFwVersion;
		pszHighestExistingFwVersion = NULL;
	}
	g_Log.Write(_T("About to search for and remove previous FieldWorks version 7..."));
	g_Log.Indent();

	int retVal = RemovePreviousFW7s(Product);

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	return retVal;
}