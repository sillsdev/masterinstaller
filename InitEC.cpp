#pragma once

#include "FixRepositoryAndMapsFilesACLs.cpp"

// Initializes Encoding Converters, including moving old data if the location has been changed.
// This is all done by launching the EncConvertersAppDataMover.exe utility, which is expected
// to have been installed in a folder recorded in the registry in key 
// HKEY_LOCAL_MACHINE\SOFTWARE\SIL\SilEncConverters22 and value RootDir.
void InitEC()
{
	DisplayStatusText(0, _T("Initializing Encoding Converters module."));
	DisplayStatusText(1, _T(""));

	// Look up new code location:
	g_Log.Write(_T("Looking up EC installation folder..."));
	LONG lResult;
	HKEY hKey;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\SilEncConverters22"), NULL,
		KEY_READ, &hKey);
	if (ERROR_SUCCESS != lResult)
		g_Log.Write(_T("...HKEY_LOCAL_MACHINE\\SOFTWARE\\SIL\\SilEncConverters22 cannot be opened."));
	else
	{
		DWORD cbData = 0;
		_TCHAR * pszEcFolder = NULL;

		// Get required buffer size:
		lResult = RegQueryValueEx(hKey, _T("RootDir"), NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS != lResult)
			g_Log.Write(_T("...Cannot get required buffer size."));
		else
		{
			pszEcFolder = new _TCHAR [cbData];

			// Retrieve folder path:
			lResult = RegQueryValueEx(hKey, _T("RootDir"), NULL, NULL, (LPBYTE)pszEcFolder,
				&cbData);

			if (ERROR_SUCCESS != lResult)
				g_Log.Write(_T("...RootDir could not be read."));
			else
			{
				g_Log.Write(_T("...EC root folder = '%s'."), pszEcFolder);

				// Remove any trailing backslash:
				if (pszEcFolder[_tcslen(pszEcFolder) - 1] == '\\')
					pszEcFolder[_tcslen(pszEcFolder) - 1] = 0;

				// Make command line from code folder and utility name:
				_TCHAR * pszCmd = new_sprintf(_T("\"%s\\EncConvertersAppDataMover.exe\""),
					pszEcFolder);

				// Run utility - return code will be logged by ExecCmd() function:
				ExecCmd(pszCmd, NULL, true, _T("Encoding Converters initialization"),
					_T("show"));
			}
			delete[] pszEcFolder;
			pszEcFolder = NULL;
		}
		RegCloseKey(hKey);
	}
	// Finally, patch up the file permissions which were omitted in older versions of
	// EncConvertersAppDataMover:
	FixRepositoryAndMapsFilesACLs();
}
