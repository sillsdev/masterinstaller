#pragma once

#include <tchar.h>

#include "RemovePreviousFW7s.cpp"

// TODO: Rename to _FormerFw7PlusProjectsFolder?
_TCHAR * _FormerFw7ProjectsFolder = NULL; // Projects directory from the most recent installation of FW 7 or Later, if any

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

		// If we found FW 7 or Later as the highest installed version, store its projects path for later:
		if (GetHugeVersion(pszHighestExistingFwVersion) >= 0x0007000000000000)
		{
			// Get ProjectsDir value:
			_FormerFw7ProjectsFolder = NewRegString(hKey, pszHighestExistingFwVersion, _T("ProjectsDir"));

			if (_FormerFw7ProjectsFolder)
			{
				RemoveTrailingBackslashes(_FormerFw7ProjectsFolder);
				g_Log.Write(_T("FW 7 Projects directory was %s."), _FormerFw7ProjectsFolder);

				// If the FW 7 or Later project directory is not the FW 7 default (C:\ProgramData\SIL\FieldWorks 7\Projects)
				// then we will change the default projects folder presented in the new installer
				// to facilitate preservation of the existing folder:
				_TCHAR * pszCommonAppDataFolder = GetFolderPathNew(CSIDL_COMMON_APPDATA);
				_TCHAR * pszDefaultFw7ProjectsFolder = MakePath(pszCommonAppDataFolder, _T("SIL\\FieldWorks 7\\Projects"));

				if (_tcsicmp(_FormerFw7ProjectsFolder, pszDefaultFw7ProjectsFolder) != 0)
				{
					// Store FW 7 projects folder in an environment variable (which will endure only for this process):
					SetEnvironmentVariable(_T("FWINSTALLERPROJDIRDEFAULT"), _FormerFw7ProjectsFolder);
					g_Log.Write(_T("Writing environment variable FWINSTALLERPROJDIRDEFAULT as %s."), _FormerFw7ProjectsFolder);
				}
				else
				{
					// REVIEW Hasso 2014.01: Why are we storing the *default* location in the registry *iff* a FW 7 folder exists *and*
					// it is the FW 7 default?  Seems we could comletely omit this step if we don't need to do it every time.
					// Store FW 8 default projects folder in an environment variable (which will endure only for this process):
					_TCHAR * pszDefaultFw8ProjectsFolder = MakePath(pszCommonAppDataFolder, _T("SIL\\FieldWorks\\Projects"));
					SetEnvironmentVariable(_T("FWINSTALLERPROJDIRDEFAULT"), pszDefaultFw8ProjectsFolder);
					g_Log.Write(_T("Writing environment variable FWINSTALLERPROJDIRDEFAULT as %s."), pszDefaultFw8ProjectsFolder);
					delete[] pszDefaultFw8ProjectsFolder;
				}
				delete[] pszCommonAppDataFolder;
			}
			else
				g_Log.Write(_T("Could not read FW 7 Projects directory"));
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