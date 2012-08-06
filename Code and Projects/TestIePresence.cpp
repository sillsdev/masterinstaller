#pragma once

#include <tchar.h>

// Tests for the presence of Internet Explorer.
bool TestIePresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
					SoftwareProduct * /*Product*/)
{
	_TCHAR * pszVersion = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Internet Explorer"), _T("Version"));

	if (pszVersion)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Found Internet Explorer version %s"), pszVersion);
#endif
		bool fResult = VersionInRange(pszVersion, pszMinVersion, pszMaxVersion);

		delete[] pszVersion;
		pszVersion = NULL;

		return fResult;
	}
#if !defined NOLOGGING
	else
		g_Log.Write(_T("Internet Explorer not found"));
#endif

	return false;
}
