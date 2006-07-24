#pragma once

// Test for DirectX
bool TestDirectXPresence(const char * pszMinVersion, const char * pszMaxVersion,
						 const char * /*pszCriticalFile*/)
{
	bool fResult = false;

	// Test for presence of a version number within range:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectX", NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVerSize = 128;
		char szVer[knVerSize];
		DWORD cbData = knVerSize;

		lResult = RegQueryValueEx(hKey, "Version", NULL, NULL, (LPBYTE)szVer, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			if (VersionInRange(szVer, pszMinVersion, pszMaxVersion))
				fResult = true;
		}
	}
	return fResult;
}