#pragma once

#include <tchar.h>

// Crude test of Toolbox 1.5.1 installation
bool TestToolbox15Presence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						   SoftwareProduct * /*Product*/)
{
	bool fResult = false;

	_TCHAR * pszDisplayName = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Toolbox_is1"),
		_T("DisplayName"));

	if (pszDisplayName)
	{
		const TCHAR * pszTemplateDisplayName = _T("Toolbox 1.5.1");

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
