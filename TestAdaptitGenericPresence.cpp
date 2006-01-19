#pragma once

// Generic test of Adapt It installation, for all localizations.
// We read the location of the uninstall file, and use its path to see if the relevant .exe
// is present.
bool TestAdaptitGenericPresence(const char * pszRegKey, const char * pszExeFileName)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;
	char szKeyPath[255];
	strcpy(szKeyPath, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
	strcat(szKeyPath, pszRegKey);

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, NULL, KEY_READ, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		const int knPathSize = 512;
		char szPath[knPathSize];
		DWORD cbData = knPathSize;

		lResult = RegQueryValueEx(hKey, "UninstallString", NULL, NULL, (LPBYTE)szPath, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			char * ch = strrchr(szPath, '\\');
			if (ch)
			{
				ch++;
				*ch = 0;
				// Form the full path to the .exe for this flavor of Adapt It:
				strcat(szPath, pszExeFileName);
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
