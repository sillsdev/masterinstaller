// Crude test of Carla Studio installation
bool TestCarlaStudioPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
							 const char * /*pszCriticalFile*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CarlaStudio",
		NULL, KEY_READ, &hKey);

	RegCloseKey(hKey);

	return (ERROR_SUCCESS == lResult);
}
