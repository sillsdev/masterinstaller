#pragma once

// Initializes Encoding Converters, including moving old data if the location has been changed.
// This is all done by launching the EncConvertersAppDataMover30.exe utility, which is expected
// to have been installed in a folder recorded in the registry in key 
// HKEY_LOCAL_MACHINE\SOFTWARE\SIL\SilEncConverters31 and value RootDir.
void InitEC()
{
	DisplayStatusText(0, _T("Initializing Encoding Converters module."));
	DisplayStatusText(1, _T(""));

	// Look up new code location:
	g_Log.Write(_T("Looking up EC installation folder..."));

	_TCHAR * pszEcFolder = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\SIL\\SilEncConverters31"), _T("RootDir"));

	if (!pszEcFolder)
		g_Log.Write(_T("...RootDir could not be read."));
	else
	{
		g_Log.Write(_T("...EC root folder = '%s'."), pszEcFolder);

		RemoveTrailingBackslashes(pszEcFolder);

		// Make command line from code folder and utility name:
		_TCHAR * pszCmd = MakePath(pszEcFolder, _T("EncConvertersAppDataMover30.exe"),
			true); // Quoted path

		delete[] pszEcFolder;
		pszEcFolder = NULL;

		// Arrange for utility to not display a window:
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		// Run utility - return code will be logged by ExecCmd() function:
		ExecCmd(pszCmd, NULL, true, _T("Encoding Converters initialization"),
			_T("show"), 0, &si);

		delete[] pszCmd;
		pszCmd = NULL;
	}
}
