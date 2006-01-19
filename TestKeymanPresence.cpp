#pragma once
// Tests for the presence of one of the flavors of Tavultesoft's Keyman.
// The Registry does not record the build number, or the version number part after the decimal
// point, so for example, if version 6.2.171 is installed, the registry will only record
// version 6.0 - the .2.171 will have to be read from the version info of the file Keyman32.dll.
bool TestKeymanPresence(const char * pszMinVersion, const char * pszMaxVersion,
						const char * pszFlavor)
{
	bool fResult = false;

	// If either version number was not given, make up a suitable default:
	if (!pszMinVersion)
		pszMinVersion = "0.0.0.0";
	if (!pszMaxVersion)
		pszMaxVersion = "32767.32767.32767.32767";

	// Test for presence of a version number within range:
	LONG lResult;
	HKEY hKeyKeyman = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Tavultesoft\\Keyman", NULL,
		KEY_READ, &hKeyKeyman);

	if (ERROR_SUCCESS == lResult)
	{
		// Enumerate all recorded versions:
		const int knVersionBufSize = 50;
		char szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKeyKeyman, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
			// We've now found the registry entries for some as yet unknown version of Keyman.
			// Find the root path of the Keyman32.dll file:
			HKEY hKeyVersion = NULL;
			char * pszKeyVersion;
			pszKeyVersion = new_sprintf("SOFTWARE\\Tavultesoft\\Keyman\\%s", szVersion);
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyVersion, NULL, KEY_READ,
				&hKeyVersion);
			delete[] pszKeyVersion;
			pszKeyVersion = NULL;
			if (ERROR_SUCCESS == lResult)
			{
				const int knRootPathLen = 512;
				char szRootPath[knRootPathLen];
				DWORD cbData = knRootPathLen;

				lResult = RegQueryValueEx(hKeyVersion, "root path", NULL, NULL,
					(LPBYTE)szRootPath, &cbData);

				if (ERROR_SUCCESS == lResult)
				{
					// Append to root path to get full path of Keyman32.dll:
					int cch = (int)strlen(szRootPath);
					if (cch < 2)
						break;
					char * ch = &szRootPath[cch - 1];
					if (*ch == '\\')
						*ch  = 0;
					strcat(szRootPath, "\\Keyman32.dll");

					// Find out file's version number:
					__int64 nVersion;
					if (GetFileVersion(szRootPath, nVersion))
					{
						char * pszVersion = GenVersionText(nVersion);
						g_Log.Write("Found Keyman version %s", pszVersion);
						delete[] pszVersion;
						pszVersion = NULL;

						if (VersionInRange(nVersion, pszMinVersion, pszMaxVersion))
						{
							// See if a flavor was specified:
							if (pszFlavor && nVersion > GetHugeVersion("6.0"))
							{
								const int kcchEditionLen = 512;
								char szEdition[kcchEditionLen];
								DWORD cbData = kcchEditionLen;

								lResult = RegQueryValueEx(hKeyVersion, "edition", NULL, NULL,
									(LPBYTE)szEdition, &cbData);

								if (ERROR_SUCCESS == lResult)
									if (stricmp(szEdition, pszFlavor) == 0)
										fResult = true;
							}
							else // No flavor specified, or version too old for flavors
								fResult = true;
							break;
						}
					}
				} // End if found root path data in registry

				RegCloseKey(hKeyVersion);
				hKeyVersion = NULL;
			} // End if successfully opened versioned registry key for Keyman
			// Reset cbData:
			cbData = knVersionBufSize;
		} // Next version recorded in registry
		RegCloseKey(hKeyKeyman);
		hKeyKeyman = NULL;
	}
	return fResult;
}
