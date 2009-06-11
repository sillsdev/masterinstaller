#pragma once

#include <tchar.h>

// Tests for presence of Service Packs for Microsoft .NET 2.0
bool TestDotNet2point0SPPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
								 SoftwareProduct * /*Product*/)
{
	// If the operating system is Windows Vista or later, then the service packs for .NET 2.0
	// cannot be directly installed. They can only be obtained through OS service packs.
	// In those cases, we will just pretend the service pack is present, to avoid running the
	// installer when we know it will fail:
	// Get details of the version of Windows we're running on:
	OSVERSIONINFOEX OSversion;
	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	::GetVersionEx((LPOSVERSIONINFO)(&OSversion));
	if (OSversion.dwMajorVersion >= 6)
		return true;

	DWORD nMinVersion = 0;
	DWORD nMaxVersion = 0xFFFFFFFF;
	if (pszMinVersion)
		nMinVersion = (DWORD)_tstoi(pszMinVersion);
	if (pszMaxVersion)
		nMaxVersion = (DWORD)_tstoi(pszMaxVersion);

	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"), NULL, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD nSP;
		DWORD cbData = sizeof(nSP);

		lResult = RegQueryValueEx(hKey, _T("SP"), NULL, NULL, (LPBYTE)&nSP, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			if (nSP >= nMinVersion && nSP <= nMaxVersion)
                return true;
		}
	}
	return false;
}
