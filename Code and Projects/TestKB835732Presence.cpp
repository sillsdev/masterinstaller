#pragma once

bool TestKB835732Presence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszKB835732DisplayName = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\KB835732"),
		_T("DisplayName"));

	bool fReturn = (pszKB835732DisplayName != NULL);

	delete[] pszKB835732DisplayName;
	pszKB835732DisplayName = NULL;

	return fReturn;
}