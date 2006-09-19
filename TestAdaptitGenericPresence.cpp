#pragma once

#include <tchar.h>

// Generic test of Adapt It installation, for all localizations.
// We read the location of the uninstall file, and use its path to see if the relevant .exe
// is present.
bool TestAdaptitGenericPresence(const TCHAR * pszRegKey, const TCHAR * pszExeFileName)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;
	const int kcch = 255;
	TCHAR szKeyPath[kcch];
	_tcscpy_s(szKeyPath, kcch, _TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"));
	_tcscat_s(szKeyPath, kcch, pszRegKey);

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, NULL, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		const int kcchPathSize = 512;
		TCHAR szPath[kcchPathSize];
		DWORD cbData = kcchPathSize;

		lResult = RegQueryValueEx(hKey, _TEXT("UninstallString"), NULL, NULL, (LPBYTE)szPath, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			TCHAR * ch = _tcsrchr(szPath, (_TCHAR)('\\'));
			if (ch)
			{
				ch++;
				*ch = 0;
				// Form the full path to the .exe for this flavor of Adapt It:
				_tcscat_s(szPath, kcchPathSize, pszExeFileName);
				// See if the files exists:
				HANDLE hFile = CreateFile(szPath, FILE_EXECUTE, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					fResult = true;
					CloseHandle(hFile);
				}
			}
		}
	}

	return fResult;
}
