#pragma once

#include <tchar.h>

// Check if any version of MSDE (SILFW) is present. Remove if so.
int RemoveAnyPreviousMSDE(const TCHAR * /*pszCriticalFile*/)
{
	g_Log.Write(_T("MSDE Pre-install function."));

	// Get the installer product code:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\Setup"), NULL, KEY_READ, &hKey);

	// We don't proceed unless the call above succeeds:
	if (lResult != ERROR_SUCCESS)
	{
		g_Log.Write(_T("MSDE (SILFW) not installed"));
		return 0;
	}

	const int knProductCodeLen = 256;
	TCHAR szProductCode[knProductCodeLen];
	DWORD cbData = knProductCodeLen;

	lResult = RegQueryValueEx(hKey, _T("ProductCode"), NULL, NULL, (LPBYTE)szProductCode,
		&cbData);
	RegCloseKey(hKey);

	if (ERROR_SUCCESS == lResult)
	{
		TCHAR * pszCommand = new_sprintf(_T("MsiExec.exe /X%s /qb-"), szProductCode);
		g_Log.Write(_T("Uninstalling MSDE with '%s'."), pszCommand);
		DisplayStatusText(0, _T("Uninstalling obsolete version of MSDE (SILFW)"));
		DWORD dwExitCode = ExecCmd(pszCommand, NULL, true);
		delete[] pszCommand;
		if (dwExitCode != 0)
			return dwExitCode;
	}
	else
		g_Log.Write(_T("Could not retrieve product code from registry."));

	return 0;
}

/*
This alternative version has not been tested...
{
	LONG lResult;
	HKEY hKey = NULL;

	g_Log.Write(_T("Searching for installed MSDE (SILFW) in order to remove it."));

	// Look for the installation listed in the registry:
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
		NULL, KEY_READ, &hKey);

	// We don't proceed unless the call above succeeds:
	if (ERROR_SUCCESS != lResult)
		return false;

	// We're going to enumerate all sub keys, and look for the one which has a DisplayName value
	// of "Microsoft SQL Server 2000 (SILFW)".

	// Registry max sizes come from:
	// ms-help://MS.VSCC.2003/MS.MSDNQTR.2004JUL.1033/sysinfo/base/registry_element_size_limits.htm
	const int kcchKeyNameBufSize = 256;
	const TCHAR * pszMsdeName = _T("Microsoft SQL Server Desktop Engine (silfw)");

	TCHAR szKeyName[kcchKeyNameBufSize];
	DWORD iKey = 0;
	DWORD cbData = kcchKeyNameBufSize;
	while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szKeyName, &cbData, NULL, NULL, NULL,
		NULL))
	{
		// Determine the length of the DisplayName Value string:
		DWORD dwBufLen;
		lResult = RegQueryValueEx(hKey, _T("DisplayName"), NULL, NULL, NULL,
			&dwBufLen);
		if (ERROR_SUCCESS == lResult)
		{
			// Get the current key's DisplayName:
			dwBufLen++; // Allow for terminating zero.
			TCHAR * pszDisplayName = new TCHAR [dwBufLen];

			lResult = RegQueryValueEx(hKey, _T("DisplayName"), NULL, NULL, (LPBYTE)pszDisplayName,
				&dwBufLen);

			if (ERROR_SUCCESS == lResult)
			{
				// Test if the DisplayName was our instance of MSDE:
				if (stricmp(pszDisplayName, _T("Microsoft SQL Server 2000 (SILFW)")) == 0)
				{
					// Found our instance.
					g_Log.Write(_T("Found %s under HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"), pszDisplayName, szKeyName);
					delete[] pszDisplayName;
					pszDisplayName = NULL;

					// To uninstall, we need to get the UninstallString. See how long that is:
					lResult = RegQueryValueEx(hKey, _T("UninstallString"), NULL, NULL, NULL,
						&dwBufLen);

					if (ERROR_SUCCESS == lResult)
					{
						TCHAR * pszUninstallMsde = new TCHAR [dwBufLen];

						// Now get the uninstall string:
						lResult = RegQueryValueEx(hKey, _T("UninstallString), NULL, NULL,
							(LPBYTE)pszUninstallMsde, &dwBufLen);

						g_Log.Write(_T("Uninstalling MSDE with UninstallString '%s'."), pszUninstallMsde);

						DWORD dwExitCode = ExecCmd(pszUninstallMsde, NULL, true);
						if (dwExitCode != 0)
							return dwExitCode;

						delete[] pszUninstallMsde;
						pszUninstallMsde = NULL;

						return 0;
					} // End if we got the UninstallString
				} // End if we found the DisplayName _T("Microsoft SQL Server 2000 (SILFW)")
			} // End if we got a DisplayName

			delete[] pszDisplayName;
			pszDisplayName = NULL;

		} // End if we got the length of the DisplayName Value

		// Reset cbData:
		cbData = kcchKeyNameBufSize;
	} // Next key
	RegCloseKey(hKey);

	return 0;
}
*/