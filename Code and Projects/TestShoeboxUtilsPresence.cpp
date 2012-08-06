#pragma once

#include <tchar.h>

// Crude test of Shoebox Utilities installation
bool TestShoeboxUtilsPresence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						   SoftwareProduct * /*Product*/)
{
	bool fResult = false;

	_TCHAR * pszDisplayName = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SIL Shoebox Utilities_is1"),
		_T("DisplayName"));

	if (pszDisplayName)
	{
		const TCHAR * pszTemplateDisplayName = _T("SIL Shoebox Utilities");

		if (_tcsnicmp(pszDisplayName, pszTemplateDisplayName,
					_tcslen(pszTemplateDisplayName)) == 0)
		{
			fResult = true;
		}
		delete[] pszDisplayName;
		pszDisplayName = NULL;
	}
	return fResult;
}
