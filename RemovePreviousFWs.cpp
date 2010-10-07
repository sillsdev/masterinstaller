#pragma once

#include <tchar.h>

#include "Uninstall.cpp"
#include "SearchOtherUsersInstallations.cpp"
#include "UniversalFixes.h"

// Remove any existing version of FieldWorks:
int RemovePreviousFWs(SoftwareProduct * /*Product*/)
{
	int nResult = 0;

	// List of all previous FW product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{E161291A-BD0C-46F6-9F39-6B165BDD4CCC}"), // FW 0.8.0, 1.3 and 1.4 ("Aussie", Disks 3 and 4)
		_T("{2D424459-8B40-41F3-94F7-4D02BFAC39E7}"), // FW 2.0 (Disk 5)
		_T("{D8D2DBAB-7487-4A6E-B369-7F1932B3BDFE}"), // old WW installer for Harvest alpha
		_T("{59EDA034-26E5-4B7A-9924-335BA200B461}"), // FW 3.0
		_T("{823B00C2-18DB-451B-93F6-068E96A90BFD}"), // FW 3.1
		_T("{DB19E5F7-187F-4E62-8D41-BA67C63BE88E}"), // After FW 3.1 but before the switch to VS 2005
		_T("{55590E14-D0D4-4D6F-9C33-884401BD26AE}"), // Trunk builds after upgrade to VS 2005
		_T("{6776923F-FF58-49DA-B57D-3A6056951EDD}"), // FW 4.0
		_T("{337432B4-5F5C-4BEB-B862-176D7CF38C8F}"), // FW 4.0.1
		_T("{AA897203-665E-4B51-9478-9D389E81E488}"), // FW 4.1 (TE alpha)
		_T("{821728B7-83CA-44FF-B779-F53D511F9982}"), // FW 4.2
		_T("{426DCB3B-2887-4218-802B-6DC3BBB420FC}"), // FW 4.9
		_T("{A8085A68-E2AC-43B1-913E-5D50D9FADF4D}"), // FW 5.0
		_T("{45FFED60-12CE-4430-945D-ED43943EAEA3}"), // FW 5.2
		_T("{96EB41BB-4ED7-43b6-838C-643C0FE574B8}"), // FW 5.2.1
		_T("{A0E40635-1CDF-4415-9D3F-AF18250F5253}"), // FW 5.3 alpha
		_T("{ECD94743-2EDF-4cc5-88FE-4515B84ACDE0}"), // FW 5.4
		_T("{91D0D4E7-43D5-486f-8B1D-B1B765C06928}"), // FW 5.4.1
		_T("{5A789B25-5206-49e2-968B-88C17835B80A}"), // FW 5.5 Trunk possibly never released
		_T("{FE9E1F44-3A41-40f3-8FB8-450A2FAECBC2}"), // FW 5.5 Beta version of 6.0
		_T("{3B831BB9-756E-49be-BFEC-83097C11EDB9}"), // FW 6.0
		_T("{A403C1D2-7ABD-4a53-981D-C6C0F6EF6AA4}"), // FW 6.0.1
		_T("{A7FD1340-2DFC-4072-B130-3363AB0B0A57}"), // FW 6.0.2
		_T("{8F8C0439-8E0F-4f74-BED6-DBD7FB05731C}"), // FW 6.0.3
		_T("{2A458F3A-B88E-416f-997E-F7935489C054}"), // FW 6.0.4
		//_T("{318EE721-1E1B-45c4-BA35-F317DA31298A}"), // FW 6.0.5
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
	DoFW40UninstallFix();

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

	// Finally, see if any of the above products were installed under other user's accounts:
	g_Log.Write(_T("Searching for installations in other user accounts..."));
	g_Log.Indent();
	int cIssues = 0; // Number of issues found.
	TCHAR * pszReport = SearchOtherUsersInstallations(pszProductCode,
		sizeof(pszProductCode) / sizeof(pszProductCode[0]),
		_T("Earlier version of SIL FieldWorks"), cIssues);
	g_Log.Unindent();
	g_Log.Write(_T("...done."));
	if (pszReport)
	{
		TCHAR * pszMessage = NULL;
		if (cIssues == 1)
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there is a program installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under another user's account which is incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user, then uninstall the program "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		else
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there are programs installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under other users' accounts which are incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user in each case, then uninstall the programs "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		new_sprintf_concat(pszMessage, 0, _T(" If this is not possible, check out alternatives in the "));
		new_sprintf_concat(pszMessage, 0, _T("Installation Issues section of the SetupFW.rtf "));
		new_sprintf_concat(pszMessage, 0, _T("file in the FieldWorks folder of this CD (or web download)."));

		new_sprintf_concat(pszMessage, 1, _T("%s"), pszReport);
		delete[] pszReport;
		new_sprintf_concat(pszMessage, 2, _T("This installation will now terminate."));
		HideStatusDialog();
		MessageBox(NULL, pszMessage, g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Found installations in other user accounts - reported: %s"), pszMessage);
		nResult = -2;
	}

	return nResult;
}
