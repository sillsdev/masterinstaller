#pragma once

#include <tchar.h>

// Tests for the presence of one of the flavors of Tavultesoft's Keyman 7.
// The Registry does not record the build number. So for example, if version 7.0.240 is
// installed, the registry will only record version 7.0 - the .0.240 will have to be read from
// the version info of the file keyman32.dll.
bool TestKeyman7Presence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						 const _TCHAR * pszFlavor)
{
	bool fResult = false;

#if !defined NOLOGGING
	g_Log.Write(_T("Searching for Keyman %s version %s through %s"),
		(pszFlavor ? pszFlavor : ""), pszMinVersion, pszMaxVersion);
#endif

	// If either version number was not given, make up a suitable default:
	if (!pszMinVersion)
		pszMinVersion = _T("0.0.0.0");
	if (!pszMaxVersion)
		pszMaxVersion = _T("32767.32767.32767.32767");

	// Test for presence of a version number within range:
	LONG lResult;
	HKEY hKeyKeyman = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\tavultesoft\\keyman engine"), NULL,
		KEY_READ, &hKeyKeyman);

	if (ERROR_SUCCESS == lResult)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Found HKEY_LOCAL_MACHINE\\SOFTWARE\\tavultesoft\\keyman engine"));
#endif

		// Enumerate all recorded versions:
		const int knVersionBufSize = 50;
		_TCHAR szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKeyKeyman, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
#if !defined NOLOGGING
			g_Log.Write(_T("Found version %s registry key."), szVersion);
#endif
			// We've now found the registry entries for some as yet unknown version of Keyman.
			// Find the root path of the keyman32.dll file:
			HKEY hKeyVersion = NULL;
			lResult = RegOpenKeyEx(hKeyKeyman, szVersion, NULL, KEY_READ, &hKeyVersion);

			if (ERROR_SUCCESS == lResult)
			{
				const int kcchRootPathLen = 512;
				_TCHAR szRootPath[kcchRootPathLen];
				DWORD cbData = kcchRootPathLen;

				lResult = RegQueryValueEx(hKeyVersion, _T("root path"), NULL, NULL,
					(LPBYTE)szRootPath, &cbData);

				if (ERROR_SUCCESS == lResult)
				{
					// Append to root path to get full path of keyman32.dll:
					int cch = (int)_tcslen(szRootPath);
					if (cch < 2)
						break;

					_TCHAR * pszKeymanFilePath = MakePath(szRootPath, _T("keyman32.dll"));

#if !defined NOLOGGING
					g_Log.Write(_T("Attempting to locate %s"), pszKeymanFilePath);
#endif
					// Find out file's version number:
					__int64 nVersion;
					if (GetFileVersion(pszKeymanFilePath, nVersion))
					{
#if !defined NOLOGGING
						_TCHAR * pszVersion = GenVersionText(nVersion);
						g_Log.Write(_T("Found Keyman version %s"), pszVersion);
						delete[] pszVersion;
						pszVersion = NULL;
#endif
						if (VersionInRange(nVersion, pszMinVersion, pszMaxVersion))
						{
							// See if a flavor was specified:
							if (pszFlavor && nVersion >= GetHugeVersion(_T("7.0")))
							{
#if !defined NOLOGGING
								g_Log.Write(_T("Testing for version 7.0 or greater"));
#endif
								// In version 7, the flavor installed is recorded under the
								// "Installed Products" sub-key:
								HKEY hKeyProducts = NULL;
								lResult = RegOpenKeyEx(hKeyVersion, _T("Installed Products"),
									NULL, KEY_READ, &hKeyProducts);
								if (ERROR_SUCCESS == lResult)
								{
#if !defined NOLOGGING
									g_Log.Write(_T("Found 'Installed Products' registry key."));
#endif
									const int kcchProductLen = 512;
									_TCHAR szProduct[kcchProductLen];
									DWORD cbData = kcchProductLen;

									lResult = RegQueryValueEx(hKeyProducts, pszFlavor, NULL,
										NULL, (LPBYTE)szProduct, &cbData);

									if (ERROR_SUCCESS == lResult)
									{
#if !defined NOLOGGING
										g_Log.Write(_T("Found flavor '%s'."), szProduct);
#endif
										if (_tcsicmp(szProduct, pszFlavor) == 0)
											fResult = true;
									}

									RegCloseKey(hKeyProducts);
									hKeyProducts = NULL;
								}
							}
							else // No flavor specified, or version too old for these flavors
								fResult = true;
							break;
						} // End if file version in range
					} // End if got file version
#if !defined NOLOGGING
					else
						g_Log.Write(_T("Could not find %s"), pszKeymanFilePath);
#endif
					delete[] pszKeymanFilePath;
					pszKeymanFilePath = NULL;

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
