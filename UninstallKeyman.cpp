void UninstallKeyman()
{
	// Get the uninstall string:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Tavultesoft Keyman 6.0",
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knStringLen = 512;
		char szUninstallString[knStringLen];
		DWORD cbData = knStringLen;

		lResult = RegQueryValueEx(hKey, "UninstallString", NULL, NULL,
			(LPBYTE)szUninstallString, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			ExecCmd(szUninstallString, false);
		}
	}
}