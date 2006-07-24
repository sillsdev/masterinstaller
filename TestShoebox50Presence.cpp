#pragma once

#include <tchar.h>

// Test for Shoebox 5.0
// Unfortunately, the Shoebox installer doesn't record proper version numbering, either in
// the registry, or in the .exe file. So we check the presence of this registry key:
// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\MenuOrder\Start Menu\Programs\Shoebox 5.0

bool TestShoebox50Presence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	bool fResult = false;

	// Test for presence of the key:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu\\Programs\\Shoebox 5.0"),
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
		fResult = true;

	return fResult;
}

This test is unreliable. I deleted the registry key, re-ran the installer, and this test
gave the wrong answer.