// Tests for presence of Service Packs for Microsoft .NET 1.1
bool TestDotNet1point1SPPresence(const char * pszMinVersion, const char * pszMaxVersion, const char * /*pszCriticalFile*/)
{
	DWORD nMinVersion = 0;
	DWORD nMaxVersion = 0xFFFFFFFF;
	if (pszMinVersion)
		nMinVersion = (DWORD)atoi(pszMinVersion);
	if (pszMaxVersion)
		nMaxVersion = (DWORD)atoi(pszMaxVersion);

	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v1.1.4322", NULL, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD nSP;
		DWORD cbData = sizeof(nSP);

		lResult = RegQueryValueEx(hKey, "SP", NULL, NULL, (LPBYTE)&nSP, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			if (nSP >= nMinVersion && nSP <= nMaxVersion)
                return true;
		}
	}
	return false;
}
