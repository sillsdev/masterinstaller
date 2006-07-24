#pragma once

#include <tchar.h>

// Tests for presence of Microsoft Text Services Framework.
// This does not seem to have any version number records.
// In addition, uninstalling on Windows 2000 appears to leave all components intact!
bool TestTsfPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
					 const TCHAR * /*pszCriticalFile*/)
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID\\{529A9E6B-6587-4F23-AB9E-9C7D683E3C50}"),
		NULL, KEY_READ, &hKey);
	RegCloseKey(hKey);

	if (ERROR_SUCCESS == lResult)
		return true;
	
	return false;
}
