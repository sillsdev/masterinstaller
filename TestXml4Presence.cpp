#pragma once

#include <tchar.h>

// Tests for presence of Microsoft XML parser 4.
// This does not seem to have any version number records.
bool TestXml4Presence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
					  const TCHAR * /*pszCriticalFile*/)
{
/*
	// We test for presence of MSXML4 by seeing if its CLSID is registered:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("Msxml2.DOMDocument.4.0\\CLSID"), NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knGuidLen = 40;
		TCHAR szGuid[knGuidLen];
		DWORD dwBufLen = knGuidLen;

		lResult = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)szGuid, &dwBufLen);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			if (stricmp(szGuid, _T("{F6D90F10-9C73-11D3-B32E-00C04F990BB4}")) == 0)
				return true;
		}
	}
*/
	// That test didn't work, so we'll try a simple call to Windows Installer, to test presence
	// of the MSXML2 product code:
	const TCHAR * pszProductCode = _T("{716E0306-8318-4364-8B8F-0CC4E9376BAC}");
	INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(pszProductCode);
	switch (state)
	{
	case INSTALLSTATE_ADVERTISED:
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
	case INSTALLSTATE_DEFAULT:
		return true;
	}
	return false;
}
