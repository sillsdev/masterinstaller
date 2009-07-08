#pragma once

#include <tchar.h>

// Test for Solid installation. Uses the path in the registry for product uninstallation
// to get the version in the display name
bool TestSolidPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
					   SoftwareProduct * /*Product*/)
{
	bool fResult = false;
	_TCHAR * pszRegPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{6f3075b4-e668-41e6-8e16-75503b24dfee}_is1");

	// Get the DisplayName string:
	_TCHAR * pszDisplayName = NewRegString(HKEY_LOCAL_MACHINE, pszRegPath, _T("DisplayName"));

	if (pszDisplayName)
	{
		// The DisplayName is of the form "Solid n.n.n", where the version number appears
		// after the word "Solid" and the space.
		const _TCHAR * pszFirstSection = _T("Solid ");
		int cchFirstSection = _tcslen(pszFirstSection);

		if (_tcsnccmp(pszDisplayName, pszFirstSection, cchFirstSection) == 0)
		{
			_TCHAR * pszVersion = &pszDisplayName[cchFirstSection];
#if !defined NOLOGGING
			g_Log.Write(_T("Found Solid version %s."), pszVersion);
#endif
			fResult = VersionInRange(pszVersion, pszMinVersion, pszMaxVersion);
		}
		delete[] pszDisplayName;
	}
	return fResult;
}
