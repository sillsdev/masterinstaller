#pragma once

#include <tchar.h>

#include "RemovePreviousFWs.cpp"
#include "PreserveWordFormingCharOverrides.cpp"

// Instantiate globally accessible variable storing path to FW installation (4.2 or earlier):
_TCHAR * pszFormerFwInstallationFolder = NULL;

// Deals with a bunch of things that we need to do to clear the way for FieldWorks 4.0
int CleanStartFw40(const TCHAR * pszCriticalFile)
{
	int nResult = 0;

	g_Log.Write(_T("Preparing clean start for FW 4.0..."));
	g_Log.Indent();

	// Prepare to move data from FW 4.2 or earlier by noting where FW is installed,
	// using the RootDir reg setting from those earlier versions. We will write this
	// value into the RoodDataDir reg value:
	LONG lResult;
	HKEY hKey = NULL;

	PreserveWordFormingCharOverrides();

	g_Log.Write(_T("Looking up FW installation folder (4.2 or earlier)..."));
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"), NULL, KEY_READ,
		&hKey);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD cbData = 0;
		// Get required buffer size:
		lResult = RegQueryValueEx(hKey, _T("RootDir"), NULL, NULL, NULL, &cbData);

		if (ERROR_SUCCESS != lResult)
			g_Log.Write(_T("...Cannot get required buffer size."));
		else
		{
			pszFormerFwInstallationFolder = new _TCHAR [cbData];

			// Retrieve folder path:
			lResult = RegQueryValueEx(hKey, _T("RootDir"), NULL, NULL,
				(LPBYTE)pszFormerFwInstallationFolder, &cbData);

			if (ERROR_SUCCESS == lResult)
				g_Log.Write(_T("...Previous FW installation folder = '%s'."), pszFormerFwInstallationFolder);
			else
				g_Log.Write(_T("...RootDir could not be read."));
		}
		RegCloseKey(hKey);
	}
	else
		g_Log.Write(_T("...RootDir not found."));

	g_Log.Write(_T("Removing previous versions of FW..."));
	g_Log.Indent();
	nResult = RemovePreviousFWs(pszCriticalFile);
	g_Log.Unindent();

	if (nResult == 0)
		g_Log.Write(_T("...Done removing previous versions of FW."));
	else
		g_Log.Write(_T("...Failed to remove previous versions of FW."));

	g_Log.Unindent();

	if (nResult == 0)
		g_Log.Write(_T("...Done preparing clean start for FW 4.0"));
	else
		g_Log.Write(_T("...Failed to prepare clean start for FW 4.0..."));

	return nResult;
}
