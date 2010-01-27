#pragma once

bool TestKB958481_XPPresence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
						  SoftwareProduct * /*Product*/)
{
	_TCHAR * pszKB958481Default = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{C09FB3CD-3D0C-3F2D-899A-6A1D67F2073F}.KB958481"),
		NULL);

	bool fReturn = (pszKB958481Default != NULL);

	delete[] pszKB958481Default;
	pszKB958481Default = NULL;

	return fReturn;
}