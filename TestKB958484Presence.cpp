#pragma once

bool TestKB958484Presence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszKB958484DisplayName = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{CE2CDD62-0124-36CA-84D3-9F4DCF5C5BD9}.KB958484"),
		_T("DisplayName"));

	bool fReturn = (pszKB958484DisplayName != NULL);

	delete[] pszKB958484DisplayName;
	pszKB958484DisplayName = NULL;

	return fReturn;
}