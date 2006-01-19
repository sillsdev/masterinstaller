// Tests for the presence of MDAC.
bool TestMDACPresence(const char * pszMinVersion, const char * pszMaxVersion, const char * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DataAccess", NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionLen = 256;
		char szVersion[knVersionLen];
		DWORD cbData = knVersionLen;

		lResult = RegQueryValueEx(hKey, "FullInstallVer", NULL, NULL, (LPBYTE)szVersion,
			&cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS != lResult)
			return false;

		g_Log.Write("Found MDAC version %s", szVersion);
		return VersionInRange(szVersion, pszMinVersion, pszMaxVersion);
	}
	return false;
}
