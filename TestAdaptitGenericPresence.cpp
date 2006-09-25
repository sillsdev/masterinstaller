#pragma once

#include <tchar.h>

// Creates a string containing the path to the Adapt It installation folder, if it exists.
// This folder contains both the executable for Adapt It, and the uninstall.exe program to
// remove it.
// The returned path does not end with a backslash '\'.
// The caller must delete[]  the string afterwards.
TCHAR * MakePathToAdaptIt(bool fUnicode)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;
	TCHAR * pszKeyPath;
	const TCHAR * pszRegRoot = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");

	if (fUnicode)
	{
		// See if Unicode version is present:
		pszKeyPath = new_sprintf(_T("%s\\Adapt It Unicode"), pszRegRoot);
	}
	else
	{
		// See if non-Unicode version is present:
		pszKeyPath = new_sprintf(_T("%s\\Adapt It"), pszRegRoot);
	}
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyPath, NULL, KEY_READ, &hKey);
	delete[] pszKeyPath;
	pszKeyPath = NULL;

	if (lResult == ERROR_SUCCESS)
	{
		// Get size of buffer needed:
		DWORD cbData = 0;
		lResult = RegQueryValueEx(hKey, _TEXT("UninstallString"), NULL, NULL, NULL, &cbData);

		// Get path to uninstall program:
		TCHAR * pszPath = new TCHAR [cbData];
		lResult = RegQueryValueEx(hKey, _TEXT("UninstallString"), NULL, NULL, (LPBYTE)pszPath,
			&cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			// Truncate the uninstall.exe bit at the end of the string:
			TCHAR * ch = _tcsrchr(pszPath, (_TCHAR)('\\'));
			if (ch)
				*ch = 0;
			return pszPath;
		}
		delete[] pszPath;
	}
	return NULL;
}


// Generic test of Adapt It installation, for all localizations.
// We read the location of the uninstall file, and use its path to see if the relevant .exe
// is present.
bool TestAdaptitGenericPresence(const TCHAR * pszExeFileName, bool fUnicode,
								const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion)
{
	TCHAR * pszAdaptItFolder = MakePathToAdaptIt(fUnicode);
	TCHAR * pszAdaptItExe = new_sprintf(_T("%s\\%s"), pszAdaptItFolder, pszExeFileName);
	delete[] pszAdaptItFolder;
	pszAdaptItFolder = NULL;

	// See if the file exists and what version it is:
	__int64 nVersion;
	bool fFound = GetFileVersion(pszAdaptItExe, nVersion);
	delete[] pszAdaptItExe;
	pszAdaptItExe = NULL;

	if (!fFound)
		return false;

	return VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
}
