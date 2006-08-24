#pragma once

#include <tchar.h>

#include "Uninstall.cpp"

// Remove any existing version of FieldWorks
int RemovePreviousFWs(const TCHAR * /*pszCriticalFile*/)
{
	int nResult = 0;

	// List of all previous FW product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{E161291A-BD0C-46F6-9F39-6B165BDD4CCC}"), // FW 0.8.0, 1.3 and 1.4 ("Aussie", Disks 3 and 4)
		_T("{2D424459-8B40-41F3-94F7-4D02BFAC39E7}"), // FW 2.0 (Disk 5)
		_T("{D8D2DBAB-7487-4A6E-B369-7F1932B3BDFE}"), // old WW installer for Harvwest alpha
		_T("{59EDA034-26E5-4B7A-9924-335BA200B461}"), // FW 3.0
		_T("{823B00C2-18DB-451B-93F6-068E96A90BFD}"), // FW 3.1
		_T("{DB19E5F7-187F-4E62-8D41-BA67C63BE88E}"), // After FW 3.1 but before the switch to VS 2005
	};
	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write(_T("Uninstalling earlier FW (Product code %s)."), pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], _T("Preparing for new FieldWorks programs"));
		if (dwResult == 0)
			g_Log.Write(_T("Uninstall succeeded."));
		else
		{
			g_Log.Write(_T("Uninstall failed with error code %d."), dwResult);
			nResult = (int)dwResult;
			return nResult;
		}
	}

	// Look for the setting in the PATH environment variable where we inadvertently wrote a
	// double backslash:
	g_Log.Write(_T("Looking for '\\\\Common' in system environement Path..."));
	g_Log.Indent();
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), NULL,
		KEY_ALL_ACCESS, &hKey);

	// We don't proceed unless the call above succeeds:
	TCHAR * pszPath = NULL;
	if (ERROR_SUCCESS == lResult)
	{
		DWORD cbData = 0;
		// Get requirement for buffer length:
		lResult = RegQueryValueEx(hKey, _T("Path"), NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			cbData++; // Allow for terminating zero contingency.

			// Collect Path setting:
			pszPath = new TCHAR [cbData];
			lResult = RegQueryValueEx(hKey, _T("Path"), NULL, NULL, (LPBYTE)pszPath, &cbData);	
		}
		else
			g_Log.Write(_T("Could not determine required buffer length to collect Path."));

		if (ERROR_SUCCESS == lResult)
		{
			// We have read in the system environment Path variable. Now check it for our
			// previous goof:
			TCHAR * pchGoof = _tcsstr(pszPath, _T("\\\\Common"));
			if (pchGoof)
			{
				g_Log.Write(_T("Found double backslash followed by 'Common'."));

				// We found the double backslash. Correct it by copying the characters after
				// it one place to the left:
				_tcscpy_s(pchGoof, _tcslen(pchGoof), pchGoof + 1);

				// Now write the Path back into the registry:
				lResult = RegSetValueEx(hKey, _T("Path"), 0, REG_SZ, (const BYTE *) pszPath,
					(DWORD)_tcslen(pszPath) * sizeof(pszPath[0]));

				if (lResult == ERROR_SUCCESS)
				{
					// Now we have to broadcast a WM_SETTINGCHANGE message:
					g_Log.Write(_T("Broadcasting WM_SETTINGCHANGE message..."));
					DWORD dwDummy;
					SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0,
						SMTO_ABORTIFHUNG, 100, &dwDummy);
					g_Log.Write(_T("...Done."));
				}
				else
					g_Log.Write(_T("Could not write new Path."));
			}
			else
				g_Log.Write(_T("'\\\\Common' not in Path."));
		}
		else
			g_Log.Write(_T("Could not get Path value."));
		RegCloseKey(hKey);
	}
	else
		g_Log.Write(_T("Could not open registry key 'HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment'"));
	delete[] pszPath;
	pszPath = NULL;
	g_Log.Unindent();
	g_Log.Write(_T("...Done with Path variable"));

	return nResult;
}
