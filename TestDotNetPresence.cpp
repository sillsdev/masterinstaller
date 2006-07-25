#pragma once

#include <tchar.h>

// Tests for presence of .NET runtime
bool TestDotNetPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						const TCHAR * /*pszCriticalFile*/)
{
	// Simple test: Look for the major and minor version number in the registry.
	// This ignores the build number, and doesn't check that any relavant files or folders are
	// present.
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	_TCHAR * pszMin;
	_TCHAR * pszMax;

	// Check if min and max values were specified:
	if (pszMinVersion)
		pszMin = my_strdup(pszMinVersion);
	else
		pszMin = my_strdup(_T("0.0"));

	if (pszMaxVersion)
		pszMax = my_strdup(pszMaxVersion);
	else
		pszMax = my_strdup(_T("32767.32767"));

	// Terminate version strings at second '.' if exists:
	_TCHAR * pszDot = _tcschr(pszMin, _TCHAR('.'));
	if (pszDot)
	{
		pszDot = _tcschr((pszDot + 1), _TCHAR('.'));
		if (pszDot)
			*pszDot = 0;
	}
	pszDot = _tcschr(pszMax, _TCHAR('.'));
	if (pszDot)
	{
		pszDot = _tcschr((pszDot + 1), _TCHAR('.'));
		if (pszDot)
			*pszDot = 0;
	}

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\.NETFramework\\policy"),
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionBufSize = 20;
		_TCHAR szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
			if (_tcslen(szVersion) > 2)
			{
				if (szVersion[0] = _TCHAR('v') && isdigit(szVersion[1]))
				{
					_TCHAR * pszVersionNumber = (szVersion + 1);

#if !defined NOLOGGING
					g_Log.Write(_T("Found .NET runtime version %s"), pszVersionNumber);
#endif

					if (VersionInRange(pszVersionNumber, pszMin, pszMax))
					{
						fResult = true;
						break;
					}
				}
			}
			// Reset cbData:
			cbData = knVersionBufSize;
		}
		RegCloseKey(hKey);
	}
	delete[] pszMin;
	delete[] pszMax;

	return fResult;

/*
	If you want to be really picky, and make sure that the English language version with the exact build number
	given is present, use this code:

	// At the moment, it is too much hard work to allow for a range of .NET versions.
	// So we will just quit if pszMinVersion and pszMaxVersion are not expressed and equal:
	bool fTooDifficult = false;
	if (!pszMinVersion || !pszMaxVersion)
		fTooDifficult = true;
	else if (strcmp(pszMinVersion, pszMaxVersion) != 0)
		fTooDifficult = true;
	if (fTooDifficult)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL,
			_T("The .NET runtime can only be tested for a distinct version number."));
		return false;
	}

	bool fResult = false;
	_TCHAR * pszWksp;

	// Collect the three sections of the version number:
	_TCHAR * pszVersion = my_strdup(pszMinVersion);
	_TCHAR * pszMajor = strtok(pszVersion, _T("."));
	_TCHAR * pszMinor = strtok(NULL, _T("."));
	_TCHAR * pszBuild = strtok(NULL, _T("."));

	// Check the registry for this version:
	pszWksp = new_sprintf(_T("SOFTWARE\\Microsoft\\.NETFramework\\policy\\v%s.%s"), pszMajor, pszMinor);
	LONG lResult;
	HKEY hKey = NULL;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszWksp, NULL, KEY_READ, &hKey);
	delete[] pszWksp;
	pszWksp = NULL;

	if (lResult == ERROR_SUCCESS)
	{
		const int knPolicyLen = 256;
		_TCHAR szPolicy[knPolicyLen];
		DWORD dwBufLen = knPolicyLen;

		lResult = RegQueryValueEx(hKey, pszBuild, NULL, NULL, (LPBYTE)szPolicy, &dwBufLen);
		RegCloseKey(hKey);
		hKey = NULL;

		if (ERROR_SUCCESS == lResult)
		{
			// Key found, now we need to check for the existence of the appropriate language
			// install directory:
			_TCHAR * pszLanguageDir = NULL;
			int nReqLen = GetWindowsDirectory(pszLanguageDir, 0);
			pszLanguageDir = new _TCHAR [nReqLen];
			if (GetWindowsDirectory(pszLanguageDir, nReqLen))
			{
				// Currently, we're only testing for the English version:
				new_sprintf_concat(pszLanguageDir, 0,
					_T("\\Microsoft.Net\\Framework\\v%s.%s.%s\\1033"), pszMajor, pszMinor,
					pszBuild);

				DWORD dwResult = GetFileAttributes(pszLanguageDir);

				if (dwResult != INVALID_FILE_ATTRIBUTES &&
					(dwResult & FILE_ATTRIBUTE_DIRECTORY))
				{
					// We found our subdirectory, so .NET runtime is present:
					fResult = true;
				}
				g_Log.Write(_T("Found .NET version %s.%s.%s"), pszMajor, pszMinor, pszBuild);
			}
			delete[] pszLanguageDir;
			pszLanguageDir = NULL;
		}
	}
	delete[] pszVersion;
	pszVersion = NULL;

	return fResult;
*/
}
