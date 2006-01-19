// Tests for the presence of Windows Installer. Here, pszMinVersion is treated as the
// minimum acceptable version number, and pszMaxVersion is ignored.
bool TestWinInstallerPresence(const char * pszMinVersion, const char * /*pszMaxVersion*/, const char * /*pszCriticalFile*/)
{
	// See if we can detect where the Windows Installer .exe file is:
	char * szLoc = GetInstallerLocation();
	if (!szLoc)
		return false; // Couldn't find it.

	bool fResult = false;

	// Now add file name to path:
	const char * szFile = "\\msiexec.exe";
	char * szLocation = new char [1 + strlen(szLoc) + strlen(szFile)];
	strcpy(szLocation, szLoc);
	strcat(szLocation, szFile);
	delete[] szLoc;
	szLoc = NULL;

	__int64 nVersion;
	if (GetFileVersion(szLocation, nVersion))
	{
		char * pszVersion = GenVersionText(nVersion);
		g_Log.Write("Found msiexec.exe version %s", pszVersion);
		delete[] pszVersion;
		pszVersion = NULL;

		fResult = VersionInRange(nVersion, pszMinVersion, NULL);
	}

	delete[] szLocation;
	szLocation = NULL;

	return fResult;
}
