// Tests for presence of .NET runtime
bool TestDotNetPresence(const char * pszMinVersion, const char * pszMaxVersion,
						const char * /*pszCriticalFile*/)
{
	// Simple test: Look for the major and minor version number in the registry.
	// This ignores the build number, and doesn't check that any relavant files or folders are
	// present.
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	char * pszMin;
	char * pszMax;

	// Check if min and max values were specified:
	if (pszMinVersion)
		pszMin = strdup(pszMinVersion);
	else
		pszMin = strdup("0.0");

	if (pszMaxVersion)
		pszMax = strdup(pszMaxVersion);
	else
		pszMax = strdup("32767.32767");

	// Terminate version strings at second '.' if exists:
	char * pszDot = strchr(pszMin, '.');
	if (pszDot)
	{
		pszDot = strchr((pszDot + 1), '.');
		if (pszDot)
			*pszDot = 0;
	}
	pszDot = strchr(pszMax, '.');
	if (pszDot)
	{
		pszDot = strchr((pszDot + 1), '.');
		if (pszDot)
			*pszDot = 0;
	}

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\.NETFramework\\policy",
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionBufSize = 20;
		char szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
			if (strlen(szVersion) > 2)
			{
				if (szVersion[0] = 'v' && isdigit(szVersion[1]))
				{
					char * pszVersionNumber = (szVersion + 1);

					g_Log.Write("Found .NET runtime version %s", pszVersionNumber);

					if (VersionInRange(pszVersionNumber, pszMin, pszMax))
					{
						fResult = true;
						break;
					}
				}
			}
			// Reset cbData:
			cbData = knVersionBufSize;
		}
		RegCloseKey(hKey);
	}
	delete[] pszMin;
	delete[] pszMax;

	return fResult;

/*
	If you want to be really picky, and make sure that the English language version with the exact build number
	given is present, use this code:

	// At the moment, it is too much hard work to allow for a range of .NET versions.
	// So we will just quit if pszMinVersion and pszMaxVersion are not expressed and equal:
	bool fTooDifficult = false;
	if (!pszMinVersion || !pszMaxVersion)
		fTooDifficult = true;
	else if (strcmp(pszMinVersion, pszMaxVersion) != 0)
		fTooDifficult = true;
	if (fTooDifficult)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL,
			"The .NET runtime can only be tested for a distinct version number.");
		return false;
	}

	bool fResult = false;
	char * pszWksp;

	// Collect the three sections of the version number:
	char * pszVersion = strdup(pszMinVersion);
	char * pszMajor = strtok(pszVersion, ".");
	char * pszMinor = strtok(NULL, ".");
	char * pszBuild = strtok(NULL, ".");

	// Check the registry for this version:
	pszWksp = new_sprintf("SOFTWARE\\Microsoft\\.NETFramework\\policy\\v%s.%s", pszMajor, pszMinor);
	LONG lResult;
	HKEY hKey = NULL;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszWksp, NULL, KEY_READ, &hKey);
	delete[] pszWksp;
	pszWksp = NULL;

	if (lResult == ERROR_SUCCESS)
	{
		const int knPolicyLen = 256;
		char szPolicy[knPolicyLen];
		DWORD dwBufLen = knPolicyLen;

		lResult = RegQueryValueEx(hKey, pszBuild, NULL, NULL, (LPBYTE)szPolicy, &dwBufLen);
		RegCloseKey(hKey);
		hKey = NULL;

		if (ERROR_SUCCESS == lResult)
		{
			// Key found, now we need to check for the existence of the appropriate language
			// install directory:
			char * pszLanguageDir = NULL;
			int nReqLen = GetWindowsDirectory(pszLanguageDir, 0);
			pszLanguageDir = new char [nReqLen];
			if (GetWindowsDirectory(pszLanguageDir, nReqLen))
			{
				// Currently, we're only testing for the English version:
				new_sprintf_concat(pszLanguageDir, 0,
					"\\Microsoft.Net\\Framework\\v%s.%s.%s\\1033", pszMajor, pszMinor,
					pszBuild);

				DWORD dwResult = GetFileAttributes(pszLanguageDir);

				if (dwResult != INVALID_FILE_ATTRIBUTES &&
					(dwResult & FILE_ATTRIBUTE_DIRECTORY))
				{
					// We found our subdirectory, so .NET runtime is present:
					fResult = true;
				}
				g_Log.Write("Found .NET version %s.%s.%s", pszMajor, pszMinor, pszBuild);
			}
			delete[] pszLanguageDir;
			pszLanguageDir = NULL;
		}
	}
	delete[] pszVersion;
	pszVersion = NULL;

	return fResult;
*/
}
