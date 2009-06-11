#pragma once

#include <tchar.h>

// Tests for presence of Microsoft XML parser 2.
// This does not seem to have any version number records.
bool TestXml2Presence(const _TCHAR * /*pszMinVersion*/, const _TCHAR * /*pszMaxVersion*/,
					  SoftwareProduct * /*Product*/)
{
	// We test for presence of MSXML2 by seeing if its CLSID is registered:
	_TCHAR * pszGuid = NewRegString(HKEY_CLASSES_ROOT, _T("Msxml2\\CLSID"))

	if (pszGuid)
	{
		bool fResult = (_tcsicmp(pszGuid, _T("{F6D90F10-9C73-11D3-B32E-00C04F990BB4}")) == 0);

		delete[] pszGuid;
		pszGuid = NULL;

		if (fResult)
			return true;
	}

	// That test didn't work, so we'll try a simple call to Windows Installer, to test presence
	// of the MSXML2 product code:
	const _TCHAR * pszProductCode = _T("{7CFAEC66-BA0E-4076-AAA5-2BE29153E6DF}");
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
