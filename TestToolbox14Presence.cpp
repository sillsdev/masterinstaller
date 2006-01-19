// Crude test of Toolbox 1.4 installation
bool TestToolbox14Presence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						   const char * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Toolbox_is1",
		NULL, KEY_READ, &hKey);

	RegCloseKey(hKey);

	return (ERROR_SUCCESS == lResult);
}
