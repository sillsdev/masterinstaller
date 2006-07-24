#pragma once

#include <tchar.h>

void UninstallMsde()
{
	// Get the installer product code:
	LONG lResult;
	HKEY hKey = NULL;

	try
	{
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\Setup"), NULL, KEY_READ, &hKey);

		// We don't proceed unless the call above succeeds:
		if (ERROR_SUCCESS != lResult && ERROR_FILE_NOT_FOUND != lResult)
		{
			throw HRESULT_FROM_WIN32(lResult);
		}

		if (ERROR_SUCCESS == lResult)
		{
			const int knProductCodeLen = 256;
			TCHAR szProductCode[knProductCodeLen];
			DWORD cbData = knProductCodeLen;

			lResult = RegQueryValueEx(hKey, _T("ProductCode"), NULL, NULL, (LPBYTE)szProductCode,
				&cbData);
			RegCloseKey(hKey);

			if (ERROR_SUCCESS == lResult)
			{
				TCHAR szCommand[100];
				sprintf(szCommand, _T("MsiExec.exe /X%s /qb-"), szProductCode);
				ExecCmd(szCommand, false);
				return;
			}
		}
	}
	catch (...)
	{
	}
	// report error?
}
