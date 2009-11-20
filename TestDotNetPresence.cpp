#pragma once

#include <tchar.h>

// Tests for presence of .NET runtime
bool TestDotNetPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						SoftwareProduct * /*Product*/)
{
	// Prerequisite: pszMinVersion and pszMaxVersion are concatenations of
	// major version number, minor version number, and service pack, as in this format:
	// maj.min.sp
	// This test ignores the build number, and doesn't check that any relavant files or folders are
	// present.
	const _TCHAR * pszNdpPath = _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP");
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	_TCHAR * pszMin;
	_TCHAR * pszMax;

	// Check if min and max values were specified:
	if (pszMinVersion)
		pszMin = my_strdup(pszMinVersion);
	else
		pszMin = my_strdup(_T("0.0.0"));

	if (pszMaxVersion)
		pszMax = my_strdup(pszMaxVersion);
	else
		pszMax = my_strdup(_T("32767.32767.32767"));

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszNdpPath, NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionBufSize = 20;
		_TCHAR szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
			if (_tcslen(szVersion) > 2)
			{
				if (szVersion[0] == _TCHAR('v') && isdigit(szVersion[1]))
				{
					LONG lResult;
					HKEY hKeyVersion;

					// Open reg key to get service pack number:
					lResult = RegOpenKeyEx(hKey, szVersion, 0, KEY_READ, &hKeyVersion);
					if (ERROR_SUCCESS == lResult)
					{
						DWORD dwSP = 0;
						DWORD cbData = sizeof(dwSP);
						LONG lResult = RegQueryValueEx(hKeyVersion, _T("SP"), NULL, NULL, (LPBYTE)&dwSP, &cbData);

						if (ERROR_SUCCESS == lResult)
						{
							// Construct mangled version number with major, minor and service pack numbers:
							_TCHAR * pszVersionNumber = (szVersion + 1);

							// Truncate at second '.' (if present) and append service pack number:
							_TCHAR * pszDot = _tcschr(pszVersionNumber, _TCHAR('.'));

							if (pszDot)
							{
								pszDot = _tcschr((pszDot + 1), _TCHAR('.'));
								if (pszDot)
									*pszDot = 0;
							}
#if !defined NOLOGGING
							g_Log.Write(_T("Found .NET runtime version %s service pack %d."), pszVersionNumber, dwSP);
#endif

							_TCHAR * pszMangledVersion = new_sprintf(_T("%s.%d"), pszVersionNumber, dwSP);

							if (VersionInRange(pszMangledVersion, pszMin, pszMax))
							{
								fResult = true;
								break;
							}
						}
#if !defined NOLOGGING
						else
							g_Log.Write(_T("Could not read SP registry value."));
#endif
					}
#if !defined NOLOGGING
					else
						g_Log.Write(_T("Could not open %s registry key."), szVersion);
#endif
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
}
