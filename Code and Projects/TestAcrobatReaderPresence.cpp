#pragma once

#include <tchar.h>

// Tests for presence of Adobe Acrobat Reader.
// The odd thing about this program is that it is easy to end up with the .pdf file extension
// "unplugged": if you install version 5, then version 6, then uninstall version 6, there is no
// file association for the .pdf extension, even thought version 5 of the reader is still there.
// It is possible to have multuple versions installed, but only one (maximum) can have the file
// association set up.
// This function will merely test for the existence of any version within the given range.
bool TestAcrobatReaderPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
							   SoftwareProduct * /*Product*/)
{
	bool fResult = false;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Adobe\\Acrobat Reader"), NULL,
		KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knVersionBufSize = 20;
		_TCHAR szVersion[knVersionBufSize];
		DWORD iKey = 0;
		DWORD cbData = knVersionBufSize;
		while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szVersion, &cbData, NULL, NULL, NULL,
			NULL))
		{
#if !defined NOLOGGING
			g_Log.Write(_T("Found Acrobat Reader version %s"), szVersion);
#endif

			if (VersionInRange(szVersion, pszMinVersion, pszMaxVersion))
			{
				fResult = true;
				break;
			}
			// Reset cbData:
			cbData = knVersionBufSize;
		}
		RegCloseKey(hKey);
	}
	return fResult;
}
