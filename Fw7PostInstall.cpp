#pragma once

#include <tchar.h>
#include <shlobj.h>
#include "msi.h"

// Returns the INSTALLDIR folder from the installation denoted by the given Product Code GUID.
// Caller must delete[] the returned value.
_TCHAR * GetMsiComponentPath(const _TCHAR * pszProductCode, const _TCHAR * pszComponentId)
{
	// Get size of buffer needed for path:
	DWORD cchBuf = 0;
	DWORD ret = MsiGetComponentPath(pszProductCode, pszComponentId, NULL, &cchBuf);

	g_Log.Write(_T("MsiGetComponentPath initally returned %d"), ret);

	if (ret == INSTALLSTATE_LOCAL || ret == INSTALLSTATE_SOURCE)
	{
		_TCHAR * pszPath = new _TCHAR [++cchBuf];

		ret = MsiGetComponentPath(pszProductCode, pszComponentId, pszPath, &cchBuf);
		g_Log.Write(_T("MsiGetComponentPath subsequently returned %d"), ret);

		if (ret == INSTALLSTATE_LOCAL || ret == INSTALLSTATE_SOURCE)
			return pszPath;

		delete[] pszPath;
		pszPath = NULL;
	}
	return NULL;
}

// Since FW 5.3, we need to test for the presence of the Microsoft.mshtml.dll file on the user's
// machine, and if it is not present, install it by running vs_piaredist.exe, which we will
// have installed to [CommonFilesFolder]\SIL\FieldWorks.
void InitMshtml()
{
	g_Log.Write(_T("Looking for Microsoft.mshtml.dll..."));
	g_Log.Indent();

	// The Microsoft.mshtml.dll will be in C:\Program Files\Microsoft.NET\Primary Interop Assemblies
	// if it is present, so we just need to get the localized form of Program Files:
	_TCHAR * pszProgramFilesPath = GetFolderPathNew(CSIDL_PROGRAM_FILES);
	
	// Form the full path to where the DLL should be:
	_TCHAR * pszMshtmlPath = MakePath(pszProgramFilesPath,
		_T("Microsoft.NET\\Primary Interop Assemblies\\Microsoft.mshtml.dll"));
	delete[] pszProgramFilesPath;
	pszProgramFilesPath = NULL;

	g_Log.Write(_T("Looking for full path: %s"), pszMshtmlPath);

	// See if the DLL is there:
	FILE * f;
	if (_tfopen_s(&f, pszMshtmlPath, _T("rb")) == 0)
	{
		// The DLL is present, so we can quit without running vs_piaredist.exe:
		g_Log.Write(_T("File exists."));
		fclose(f);
		f = NULL;
	}
	else
	{
		g_Log.Write(_T("File does not exist. Preparing to launch vs_piaredist.exe."));

		// Get localized path to C:\Program Files\Common Files:
		_TCHAR * pszCommonPath = GetFolderPathNew(CSIDL_PROGRAM_FILES_COMMON);

		// Form path to .exe file:
		_TCHAR * pszVs_piaredistPath = MakePath(pszCommonPath, _T("SIL\\vs_piaredist.exe"));
		delete[] pszCommonPath;
		pszCommonPath = NULL;

		DisplayStatusText(0, _T("Installing Microsoft Primary Interoperability Assemblies..."));
		DisplayStatusText(1, _T(""));

		ExecCmd(pszVs_piaredistPath, NULL);

		delete[] pszVs_piaredistPath;
		pszVs_piaredistPath = NULL;
	}
	delete[] pszMshtmlPath;
	pszMshtmlPath = NULL;
	
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

// Uninstalls the product that has the given display name (used in Add/Remove Programs window).
void UninstallProduct(_TCHAR * pszDisplayName)
{
	g_Log.Write(_T("Attempting to uninstall %s"), pszDisplayName);

	// Enumerate registry keys in HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
	// looking for one with a matching DisplayName value:
	HKEY hKey;
	LONG lResult;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS != lResult)
	{
		g_Log.Write(_T("Could not open HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
		return;
	}

	const int knProductCodeBufSize = 100;
	_TCHAR szProductCode[knProductCodeBufSize];
	DWORD iKey = 0;
	DWORD cbData = knProductCodeBufSize;
	while (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey++, szProductCode, &cbData, NULL, NULL, NULL, NULL))
	{
		LONG lResultPC;
		HKEY hKeyPC;

		// Open reg key to get service pack number:
		lResultPC = RegOpenKeyEx(hKey, szProductCode, 0, KEY_READ, &hKeyPC);
		if (ERROR_SUCCESS == lResultPC)
		{
			// Get DisplayName value:
			const int knDisplayNameLen = 256;
			_TCHAR szDisplayName[knDisplayNameLen];
			DWORD cbData = knDisplayNameLen;
			lResultPC = RegQueryValueEx(hKeyPC, _T("DisplayName"), NULL, NULL, (LPBYTE)szDisplayName, &cbData);
			RegCloseKey(hKeyPC);

			if (ERROR_SUCCESS == lResultPC)
			{
				if (_tcscmp(szDisplayName, pszDisplayName) == 0)
				{
					g_Log.Write(_T("Got product code for %s: %s. Attempting to uninstall..."), pszDisplayName, szProductCode);

					// Attempt to uninstall:
					DisplayStatusText(1, _T("Removing %s"), pszDisplayName);

					if (ERROR_SUCCESS != MsiConfigureProduct(szProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT))
					{
						g_Log.Write(_T("...Uninstallation has failed."));
						_TCHAR * pszMsg = new_sprintf(_T("ERROR: Failed to uninstall %s."), pszDisplayName);
						MessageBox(NULL, pszMsg, _T("Uninstallation error"), MB_ICONINFORMATION | MB_OK);
						delete[] pszMsg;
					}
					else
						g_Log.Write(_T("...Done."));

					break;
				}
			}
		}
		// Reset data for next iteration:
		cbData = knProductCodeBufSize;
	} // Next key in enumaration

	RegCloseKey(hKey);
}

// Detects if our SILFW instance of SQL Server is installed. If so, offers to uninstall it.
// If there are no other instances, offers to uninstall all SQL Server components.
void DisposeOfSqlServer()
{
	g_Log.Write(_T("Searching for SQL Server installation..."));

	// See if SQL Server is installed:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL"), NULL, KEY_READ, &hKey);

	// We don't proceed unless the call above succeeds:
	if (lResult != ERROR_SUCCESS)
	{
		g_Log.Write(_T("SQL Server not installed."));
		return;
	}

	// Get the internal SQL instance number string for our SILFW instance:
	const int knInstanceIndexLen = 256;
	_TCHAR szInstanceIndex[knInstanceIndexLen];
	DWORD cbData = knInstanceIndexLen;

	lResult = RegQueryValueEx(hKey, _T("SILFW"), NULL, NULL, (LPBYTE)szInstanceIndex, &cbData);

	if (lResult != ERROR_SUCCESS)
	{
		g_Log.Write(_T("SQL Server detected, but could not read HKLM\\SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL::SILFW value"));
		return;
	}

	DWORD NumSqlInstances;
	lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &NumSqlInstances, NULL, NULL, NULL, NULL);

	g_Log.Write(_T("Detected %d SQL Server instances."), NumSqlInstances);

	RegCloseKey(hKey);
	hKey = NULL;

	// Get the MSI product code of our instance:
	_TCHAR * pszRegKey = new_sprintf(_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\%s\\Setup"), szInstanceIndex);

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegKey, NULL, KEY_READ, &hKey);
	delete[] pszRegKey;
	pszRegKey = NULL;

	// We don't proceed unless the call above succeeds:
	if (lResult != ERROR_SUCCESS)
	{
		g_Log.Write(_T("Could not open HKLM\\SOFTWARE\\Microsoft\\Microsoft SQL Server\\%s\\Setup."), szInstanceIndex);
		return;
	}

	const int knProductCodeLen = 256;
	_TCHAR szProductCode[knProductCodeLen];
	cbData = knProductCodeLen;

	lResult = RegQueryValueEx(hKey, _T("ProductCode"), NULL, NULL, (LPBYTE)szProductCode, &cbData);

	// We don't proceed unless the call above succeeds:
	if (lResult != ERROR_SUCCESS)
	{
		g_Log.Write(_T("Could not read product code for SILFW instance in HKLM\\SOFTWARE\\Microsoft\\Microsoft SQL Server\\%s\\Setup."), szInstanceIndex);
		return;
	}

	g_Log.Write(_T("Got product code for SILFW instance: %s"), szProductCode);

	// Generate question to pose to user:
	_TCHAR * pszQuestion;
	if (NumSqlInstances == 1)
		pszQuestion = new_sprintf(_T("SQL Server 2005 is installed on this machine, but it was only needed for FieldWorks. It is no longer needed. Would you like to uninstall it now?"));
	else
		pszQuestion = new_sprintf(_T("SQL Server 2005 is installed on this machine. There are %d instances, one of which (\"SILFW\") was needed for FieldWorks. That instance is no longer needed. Would you like to uninstall the SILFW instance now?"), NumSqlInstances);

	// Ask user:
	HideStatusDialog();
	g_Log.Write(_T("Asking user the following question: %s"), pszQuestion);

	if (MessageBox(NULL, pszQuestion, _T("SQL Server 2005 found"), MB_YESNO) == IDYES)
	{
		// User opted to uninstall SQL Server:
		g_Log.Write(_T("User answered \"yes\"; uninstalling..."));
		ShowStatusDialog();
		DisplayStatusText(0, _T("Uninstalling unneeded components of SQL Server 2005."));
		DisplayStatusText(1, _T("Removing SILFW instance"));

		// Uninstall SILFW instance first:
		if (ERROR_SUCCESS != MsiConfigureProduct(szProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT))
		{
			g_Log.Write(_T("...Uninstallation of SILFW instance has failed."));
			MessageBox(NULL, _T("ERROR: Failed to uninstall SILFW instance."),
				_T("Uninstallation error"), MB_ICONINFORMATION | MB_OK);
		}

		if (NumSqlInstances == 1)
		{
			// Uninstall SQL Server main product:
			g_Log.Write(_T("Attempting to uninstall SQL Server main product"));
			DisplayStatusText(1, _T("Removing SQL Server main product"));

			if (ERROR_SUCCESS != MsiConfigureProduct(_T("{2750B389-A2D2-4953-99CA-27C1F2A8E6FD}"),
				INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT))
			{
				g_Log.Write(_T("...Uninstallation of SQL Server has failed."));
				MessageBox(NULL, _T("ERROR: Failed to uninstall SQL Server main product."),
					_T("Uninstallation error"), MB_ICONINFORMATION | MB_OK);
			}

			// Uninstall other SQL Server bits:
			UninstallProduct(_T("Microsoft SQL Server Setup Support Files (English)"));
			UninstallProduct(_T("Microsoft SQL Server VSS Writer"));
			UninstallProduct(_T("Microsoft SQL Server Native Client"));
		}

		g_Log.Write(_T("...Done uninstalling SQL Server."));
	}
	else
		g_Log.Write(_T("User answered \"no\"."));

	delete[] pszQuestion;
}

// Detects if any earlier (than 7.0) version of FW was installed.
// Returns installer product code of earlier version if found, otherwise NULL.
// Assumes user can only have at most only one earlier version.
// Caller must delete[] the returned value.
_TCHAR * DetectEarlierFwInstallation()
{
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
		_T("{318EE721-1E1B-45c4-BA35-F317DA31298A}"), // FW 6.0.5
		_T("{182F68EB-3290-4a49-915E-BEC6866AB8C0}"), // FW 6.0.6
	};

	g_Log.Write(_T("Looking for earlier version of FW installation..."));

	// Iterate over all versions, from latest through to earliest:
	for (int i = (sizeof(pszProductCode) / sizeof(pszProductCode[0])) - 1; i >= 0 ; i--)
	{
		// See if the installation is present:
		INSTALLSTATE state = MsiQueryProductState(pszProductCode[i]);
		bool fInstalled = false;
		switch (state)
		{
			case INSTALLSTATE_ADVERTISED:
			case INSTALLSTATE_LOCAL:
			case INSTALLSTATE_SOURCE:
			case INSTALLSTATE_DEFAULT:
				g_Log.Write(_T("...Found %s"), pszProductCode[i]);
				return my_strdup(pszProductCode[i]);
		}
	}
	g_Log.Write(_T("...None found."));
	return NULL;
}

int Fw7PostInstall(SoftwareProduct * Product)
{
	InitMshtml();

	g_Log.Write(_T("About to run data migration utility..."));

	// See if we can locate the data migration utility we should have just installed:
	_TCHAR * pszMigrateUtilPath = GetMsiComponentPath(Product->m_kpszMsiProductCode, _T("{D25017CC-66F5-4BEE-B7BA-39BE8AE3698F}"));
	g_Log.Write(_T("Found MigrateSqlDbs path: %s"), pszMigrateUtilPath);

	DWORD migrationStatus = -1; // Default to value meaning "utility failed for some reason other than migration failure".

	if (pszMigrateUtilPath != NULL)
	{
		ShowStatusDialog();
		DisplayStatusText(0, _T("Migrating any FieldWorks data from earlier versions..."));
		DisplayStatusText(1, _T(""));

		// Run the MigrateSqlDbs.exe utility:
		_TCHAR * pszCmd = new_sprintf(_T("\"%s\" -autoclose"), pszMigrateUtilPath);
		migrationStatus = ExecCmd(pszCmd, _T(""), true, NULL, _T("Monitor 4000"));

		// Tidy up:
		delete[] pszCmd;
		pszCmd = NULL;
		delete[] pszMigrateUtilPath;
		pszMigrateUtilPath = NULL;
	}
	else
		g_Log.Write(_T("Could not find path to MigrateSqlDbs.exe."));

	_TCHAR * pszRecommendation;

	// Examine the return value from MigrateSqlDbs.exe and prepare a text recommending whether or
	// not the user should allow uninstallation of an earlier version of FW.
	switch (migrationStatus)
	{
	case -1:
		g_Log.Write(_T("Data Migration failed for some unknown reason."));
		pszRecommendation = my_strdup(_T(", although this is not recommended because the data migration process failed unexpectedly."));
		break;
	case 0:
		g_Log.Write(_T("Data Migration succeeded."));
		pszRecommendation = my_strdup(_T(", especially since copies of all your data have been successfully migrated to the new version."));
		break;
	default:
		g_Log.Write(_T("%d projects failed to migrate."), migrationStatus);
		pszRecommendation = new_sprintf(_T(", although this is not recommended because %d of your projects failed to migrate to the new version"), migrationStatus);
		break;
	}
	g_Log.Write(_T("...Done"));

	// See if an earlier version of FW is installed:
	_TCHAR * pszProductCode = DetectEarlierFwInstallation();

	if (pszProductCode)
	{
		// Found one. Get the listed version number of the product:
		_TCHAR * pszVersion = my_strdup(_T("unknown version"));
		const int kcchVersion = 256;
		_TCHAR szIntalledVersion[kcchVersion];
		DWORD cch = kcchVersion;
		if (MsiGetProductInfo(pszProductCode, INSTALLPROPERTY_VERSIONSTRING,
			szIntalledVersion, &cch) == ERROR_SUCCESS)
		{
			delete[] pszVersion;
			pszVersion = my_strdup(szIntalledVersion);
		}

		// Prepare message to offer uninstallation to user:
		_TCHAR * pszMessage = new_sprintf(_T("FieldWorks 7.0 is now installed, but an earlier version of FieldWorks (%s) is also installed.\n\n")
			_T("You could uninstall FieldWorks %s now and just use FieldWorks 7.0%s. Would you like FieldWorks %s to be uninstalled right now?"),
			pszVersion, pszVersion, pszRecommendation, pszVersion);

		delete[] pszRecommendation;

		HideStatusDialog();

		g_Log.Write(_T("Asking user the following question: %s"), pszMessage);

		if (MessageBox(NULL, pszMessage, _T("Earlier version of FieldWorks found"),
			MB_YESNO | (migrationStatus == 0 ? MB_DEFBUTTON1 : MB_DEFBUTTON2)) == IDYES)
		{
			// User opted to uninstall earlier version of FW:
			g_Log.Write(_T("User answered \"yes\"; uninstalling..."));
			ShowStatusDialog();
			DisplayStatusText(0, _T("Uninstalling FieldWorks %s."), pszVersion);
			DisplayStatusText(1, _T(""));

			if (ERROR_SUCCESS != MsiConfigureProduct(pszProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT))
			{
				g_Log.Write(_T("...Uninstallation has failed."));
				_TCHAR * pszMsg = new_sprintf(_T("ERROR: Failed to uninstall FieldWorks %s."), pszVersion);
				MessageBox(NULL, pszMsg, _T("Uninstallation error"), MB_ICONINFORMATION | MB_OK);
				delete[] pszMsg;
			}
			else
				g_Log.Write(_T("...Done."));
		}
		else
			g_Log.Write(_T("User answered \"no\"."));

		delete[] pszVersion;
		delete[] pszProductCode;
		delete[] pszMessage;

		ShowStatusDialog();
	}

	// If there is now no earlier FW installed, we should offer to uninstall SQL Server if it is present:
	pszProductCode = DetectEarlierFwInstallation();
	if (pszProductCode)
	{
		g_Log.Write(_T("Earlier FW with product code %s detected, so won't ask to uninstall SQL Server."), pszProductCode);
		delete[] pszProductCode;
	}
	else
		DisposeOfSqlServer();

	return 0;
}
