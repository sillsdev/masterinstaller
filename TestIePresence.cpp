// Tests for the presence of Internet Explorer.
bool TestIePresence(const char * pszMinVersion, const char * pszMaxVersion, const char * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Internet Explorer", NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionLen = 256;
		char szVersion[knVersionLen];
		DWORD cbData = knVersionLen;

		lResult = RegQueryValueEx(hKey, "Version", NULL, NULL, (LPBYTE)szVersion, &cbData);
		RegCloseKey(hKey);

		g_Log.Write("Found Internet Explorer version %s", szVersion);

		if (ERROR_SUCCESS == lResult)
			return VersionInRange(szVersion, pszMinVersion, pszMaxVersion);
	}
	return false;
}
