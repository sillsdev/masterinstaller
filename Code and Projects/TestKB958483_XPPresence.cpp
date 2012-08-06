#pragma once

bool TestKB958483_XPPresence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszKB958483Default = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{A3051CD0-2F64-3813-A88D-B8DCCDE8F8C7}.KB958483"),
		NULL);

	bool fReturn = (pszKB958483Default != NULL);

	delete[] pszKB958483Default;
	pszKB958483Default = NULL;

	return fReturn;
}