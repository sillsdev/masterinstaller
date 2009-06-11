#pragma once

#include <tchar.h>

// Crude test of Lexique Pro installation - currently only recognizes Lexique Pro 3.2
bool TestLexiquePresence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						 SoftwareProduct * /*Product*/)
{
	bool fResult = false;

	_TCHAR * pszAppName = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Lexique Pro - Bambara_is1"),
		_T("DisplayName"));

	if (pszAppName)
	{
		if (_tcscmp(pszAppName, _T("Lexique Pro 3.2")) == 0)
			fResult = true;

		delete[] pszAppName;
		pszAppName = NULL;
	}
	return fResult;
}
