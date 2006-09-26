#pragma once

#include <tchar.h>

// Tests for the presence of one of the flavors of Tavultesoft's Keyman.
// The Registry does not record the build number, or the version number part after the decimal
// point, so for example, if version 6.2.171 is installed, the registry will only record
// version 6.0 - the .2.171 will have to be read from the version info of the file Keyman32.dll.
bool TestKeymanPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						const TCHAR * pszFlavor)
{
	bool fResult = false;

	// If either version number was not given, make up a suitable default:
	if (!pszMinVersion)
		pszMinVersion = _T("0.0.0.0");
	if (!pszMaxVersion)
		pszMaxVersion = _T("32767.32767.32767.32767");

	// Test for presence of a version number within range:
	LONG lResult;
	HKEY hKeyKeyman = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Tavultesoft\\Keyman"), NULL,
		KEY_READ, &hKeyKeyman);

	if (ERROR_SUCCESS == lResult)
	{
		// Enumerate all recorded versions:
		const int knVersionBufSize = 50;
		TCHAR szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKeyKeyman, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
			// We've now found the registry entries for some as yet unknown version of Keyman.
			// Find the root path of the Keyman32.dll file:
			HKEY hKeyVersion = NULL;
			TCHAR * pszKeyVersion;
			pszKeyVersion = new_sprintf(_T("SOFTWARE\\Tavultesoft\\Keyman\\%s"), szVersion);
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyVersion, NULL, KEY_READ,
				&hKeyVersion);
			delete[] pszKeyVersion;
			pszKeyVersion = NULL;
			if (ERROR_SUCCESS == lResult)
			{
				const int kcchRootPathLen = 512;
				TCHAR szRootPath[kcchRootPathLen];
				DWORD cbData = kcchRootPathLen;

				lResult = RegQueryValueEx(hKeyVersion, _T("root path"), NULL, NULL,
					(LPBYTE)szRootPath, &cbData);

				if (ERROR_SUCCESS == lResult)
				{
					// Append to root path to get full path of Keyman32.dll:
					int cch = (int)_tcslen(szRootPath);
					if (cch < 2)
						break;
					TCHAR * ch = &szRootPath[cch - 1];
					if (*ch == '\\')
						*ch  = 0;
					_tcscat_s(szRootPath, kcchRootPathLen, _T("\\Keyman32.dll"));

					// Find out file's version number:
					__int64 nVersion;
					if (GetFileVersion(szRootPath, nVersion))
					{
#if !defined NOLOGGING
						TCHAR * pszVersion = GenVersionText(nVersion);
						g_Log.Write(_T("Found Keyman version %s"), pszVersion);
						delete[] pszVersion;
						pszVersion = NULL;
#endif
						if (VersionInRange(nVersion, pszMinVersion, pszMaxVersion))
						{
							// See if a flavor was specified:
							if (pszFlavor && nVersion > GetHugeVersion(_T("6.0")))
							{
								const int kcchEditionLen = 512;
								TCHAR szEdition[kcchEditionLen];
								DWORD cbData = kcchEditionLen;

								lResult = RegQueryValueEx(hKeyVersion, _T("edition"), NULL, NULL,
									(LPBYTE)szEdition, &cbData);

								if (ERROR_SUCCESS == lResult)
									if (_tcsicmp(szEdition, pszFlavor) == 0)
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
