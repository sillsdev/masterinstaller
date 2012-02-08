#pragma once

#include <tchar.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlguid.h>

#include "msi.h"

#include "Fw7PostInstall.h"

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

// Launch the FieldWorks Bridge installer, which should have been installed to the Installers sub folder.
void InstallFieldWorksBridge(SoftwareProduct * Product)
{
	g_Log.Write(_T("About to run FieldWorks Bridge installer..."));
	g_Log.Indent();

	// Find the installer file using its component GUID from the FW installer's Files.wxs source:
	_TCHAR * pszFieldWorksBridgeInstallerPath = GetMsiComponentPath(Product->m_kpszMsiProductCode,
		_T("{9442BF43-4A64-4885-9593-43DD7E927290}"));

	if (!pszFieldWorksBridgeInstallerPath)
	{
		g_Log.Write(_T("Could not locate FieldWorks Bridge installer"));
		return;
	}

	g_Log.Write(_T("Found FieldWorks Bridge installer path: %s"), pszFieldWorksBridgeInstallerPath);

	// Uninstall any previous version of FieldWorks Bridge, if there is one:
	const _TCHAR * pszFieldWorksBridgeProductCode = _T("{FDD4B174-8190-4819-880F-77D1F9F1DA9C}");
	
	INSTALLSTATE state = MsiQueryProductState(pszFieldWorksBridgeProductCode);
	switch (state)
	{
		case INSTALLSTATE_ADVERTISED:
		case INSTALLSTATE_LOCAL:
		case INSTALLSTATE_SOURCE:
		case INSTALLSTATE_DEFAULT:
			g_Log.Write(_T("Found existing installation of FieldWorks Bridge - uninstalling..."));
			DisplayStatusText(0, _T("Installing previous FieldWorks Bridge ready for current version..."));
			DisplayStatusText(1, _T(""));

			if (ERROR_SUCCESS != MsiConfigureProduct(pszFieldWorksBridgeProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT))
			{
				g_Log.Write(_T("...Uninstallation of FieldWorks Bridge has failed."));
			}
			else
				g_Log.Write(_T("...Done."));
	}


	DisplayStatusText(0, _T("Installing FieldWorks Bridge..."));
	DisplayStatusText(1, _T(""));

	_TCHAR * pszCmd = new_sprintf(_T("msiexec /i \"%s\" /qb"), pszFieldWorksBridgeInstallerPath);

	ExecCmd(pszCmd, NULL);

	delete[] pszCmd;
	pszCmd = NULL;

	delete[] pszFieldWorksBridgeInstallerPath;
	pszFieldWorksBridgeInstallerPath = NULL;
	
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

// Detects if our SILFW instance of SQL Server is installed. If so, uninstalls it.
// If there are no other instances, uninstalls all SQL Server components.
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

	// Uninstall SQL Server:
	g_Log.Write(_T("Uninstalling SQL Server..."));
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

// Returns the target of a Windows shortcut (.lnk file)
// Caller must delete[] the returned value;
_TCHAR * GetShortcutTarget(_TCHAR * pszShortcutPath)
{
	IShellLink * pISL = NULL;    
	IPersistFile * ppf  = NULL;
	_TCHAR * pszTargetPath = new _TCHAR [MAX_PATH];
	pszTargetPath[0] = 0;

	CoInitialize(NULL);

	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pISL)))
	{
		if (SUCCEEDED (pISL->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
		{
			// If it did, load the shortcut into our persistent file
			if (SUCCEEDED (ppf->Load(pszShortcutPath, 0)))
			{
				// Read the target information from the shortcut object
				if (S_OK != (pISL->GetPath(pszTargetPath, MAX_PATH, NULL, SLGP_UNCPRIORITY)))
				{
					pszTargetPath[0] = 0;
					g_Log.Write(_T("GetPath() failed while attempting to get target for shortcut %s"), pszShortcutPath);
				}
			}
			else
				g_Log.Write(_T("Load() failed while attempting to get target for shortcut %s"), pszShortcutPath);
		}
		else
			g_Log.Write(_T("QueryInterface() failed while attempting to get target for shortcut %s"), pszShortcutPath);
	}
	else
		g_Log.Write(_T("CoCreateInstance() failed while attempting to get target for shortcut %s"), pszShortcutPath);

	if (ppf)
		ppf->Release();

	if (pISL)
		pISL->Release();

	return pszTargetPath;
}

// Searches for older FW shortcuts in the given folder, and deletes them, including project-specific ones.
void SearchAndDeleteOldFwShortcuts(_TCHAR * pszFolder)
{
	g_Log.Write(_T("Searching for older FW shortcuts in %s..."), pszFolder);

	// Look up the older FieldWorks code directory in the registry:
	_TCHAR * pszRootCodeDir = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\SIL\\FieldWorks"), _T("RootCodeDir"));

	if (!pszRootCodeDir)
	{
		g_Log.Write(_T("Could not read registry value RootCodeDir"));
		return;
	}

	// Make strings of paths to old FW apps (except WorldPad):
	_TCHAR * pszOldTePath = MakePath(pszRootCodeDir, _T("TE.exe"));
	_TCHAR * pszOldFLExPath = MakePath(pszRootCodeDir, _T("FLEx.exe"));
	_TCHAR * pszOldDnPath = MakePath(pszRootCodeDir, _T("FwNotebook.exe"));
	_TCHAR * pszOldTlePath = MakePath(pszRootCodeDir, _T("FwListEditor.exe"));

	// Search the given folder for all shortcuts (.lnk files):
	_TCHAR * pszSearch = MakePath(pszFolder, _T("*.lnk"));

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(pszSearch, &wfd);

	delete[] pszSearch;
	pszSearch = NULL;

	if (hFind == INVALID_HANDLE_VALUE)
	{
		g_Log.Write(_T("No shortcuts exist in %s..."), pszFolder);
		return;
	}
	do
	{
		// Got a shortcut. Retrieve its full path:
		_TCHAR * pszShortcut = MakePath(pszFolder, wfd.cFileName);
		// Get the target that the shortuct points to:
		_TCHAR * pszShortcutTarget = GetShortcutTarget(pszShortcut);

		// If the target begins with the path to an older FW app (except WorldPad)
		// (e.g. "C:\Program Files\SIL\FieldWorks\TE.exe"), then delete it:
		if (_tcsicmp(pszShortcutTarget, pszOldTePath) == 0
			|| _tcsicmp(pszShortcutTarget, pszOldFLExPath) == 0
			|| _tcsicmp(pszShortcutTarget, pszOldDnPath) == 0
			|| _tcsicmp(pszShortcutTarget, pszOldTlePath) == 0)
		{
			g_Log.Write(_T("Found shortcut %s which has a target of %s: deleting"), pszShortcut, pszShortcutTarget);
			DeleteFile(pszShortcut);
		}

		// Tidy up:
		delete[] pszShortcutTarget;
		pszShortcutTarget = NULL;
		delete[] pszShortcut;
		pszShortcut = NULL;

	} while (FindNextFile(hFind, &wfd));

	// Tidy up:
	FindClose(hFind);
	hFind = NULL;
	delete[] pszOldTePath;
	pszOldTePath = NULL;
	delete[] pszOldFLExPath;
	pszOldFLExPath = NULL;
	delete[] pszOldDnPath;
	pszOldDnPath = NULL;
	delete[] pszOldTlePath;
	pszOldTlePath = NULL;
}

// Examines all desktop shortcuts (current user and all-users) and deletes ones for FLEx and TE
// from FW version 6 and earlier, including project-specific ones.
void DeleteOldFwShortcuts()
{
	// Form list of system folders where we will search for old FW shortcuts:
	const int FolderIds[] = { CSIDL_DESKTOPDIRECTORY, CSIDL_COMMON_DESKTOPDIRECTORY };

	// Iterate through above list of system folders:
	for (int i = 0; i < sizeof(FolderIds) / sizeof(FolderIds[0]); i++)
	{
		// Get a string to the actual folder:
		_TCHAR * pszFolderPath = GetFolderPathNew(FolderIds[i]);
		// Delete all old FW shortcuts in folder:
		SearchAndDeleteOldFwShortcuts(pszFolderPath);

		// Tidy up:
		delete[] pszFolderPath;
		pszFolderPath = NULL;
	}
}

INT_PTR CALLBACK DlgProcEarlierFwDetected(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// The dialog procedure for informing user that an earlier version of FW has been found.
{
	static _TCHAR * pszVersion = NULL;

	switch(msg)
	{
	case WM_INITDIALOG: // Dialog is being shown.
		{
			CentralizeWindow(hwnd);

			pszVersion = reinterpret_cast<_TCHAR *>(lParam);

			if (pszVersion)
			{
				// Set first checkbox text:
				_TCHAR * pszRemoveShortcuts = new_sprintf(_T("Remove the shortcuts for the earlier version of FieldWorks (%s)."), pszVersion);
				SendDlgItemMessage(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS, WM_SETTEXT, 0, (LPARAM)pszRemoveShortcuts);
				delete[] pszRemoveShortcuts;
				pszRemoveShortcuts = NULL;

				// Set second checkbox text:
				_TCHAR * pszUninstall = new_sprintf(_T("Uninstall the earlier version of FieldWorks (%s)."), pszVersion);
				SendDlgItemMessage(hwnd, IDC_CHECKBOX_UNINSTALL_EARLIER_FW, WM_SETTEXT, 0, (LPARAM)pszUninstall);
				delete[] pszUninstall;
				pszUninstall = NULL;
			}

			// Set Icon:
			SetSilIcon(hwnd);

			// Select first checkbox:
			SendDlgItemMessage(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;

	case WM_COMMAND: // We got a message from a control/menu - in this case, a button.
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int retVal = 0;
				// Collect user settings and combine into a single integer:
				if (SendDlgItemMessage(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS, BM_GETCHECK, 0, 0) == BST_CHECKED)
					retVal |= EARLIER_FW_REMOVE_SHORTCUTS;
				if (SendDlgItemMessage(hwnd, IDC_CHECKBOX_UNINSTALL_EARLIER_FW, BM_GETCHECK, 0, 0) == BST_CHECKED)
					retVal |= EARLIER_FW_UNINSTALL;

				EndDialog(hwnd, retVal);
			}
			break;
		case IDC_CHECKBOX_UNINSTALL_EARLIER_FW:
			// If the "uninstall earlier FW" box has just been selected, select the "remove shortcuts" option and 
			// disable its box:
			if (SendDlgItemMessage(hwnd, IDC_CHECKBOX_UNINSTALL_EARLIER_FW, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				SendDlgItemMessage(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow(GetDlgItem(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS), FALSE);
			}
			else // Enable the "remove shortcuts" box
				EnableWindow(GetDlgItem(hwnd, IDC_CHECKBOX_REMOVE_SHORTCUTS), TRUE);
			break;
		default: // All the WM_COMMAND messages we don't handle are handled by Windows.
			return 0;
		}
		break;

	case WM_DESTROY: // Dialog is off the screen by now.
		pszVersion = NULL;
		break;

	default: // All the messages we don't handle are handled by Windows.
		return 0;
	}
	return 1; // This means we have processed the message.
}

#include "InitEC.cpp"

int Fw7PostInstall(SoftwareProduct * Product)
{
	InitMshtml();
	InstallFieldWorksBridge(Product);

	// For FieldWorks 7.0.x, we still need to initialize the Encoding Converters by
	// running EncConvertersAppDataMover30.exe. This was erroneously omitted through
	// version 7.0.3.
	g_Log.Write(_T("Contemplating running EncConvertersAppDataMover30.exe: Testing for FW 7.0.x..."));
	if (Product)
	{
		if (_tcsicmp(Product->m_kpszMsiProductCode, _T("{8E80F1ED-826A-46d5-A59A-D8A203F2F0D9}")) == 0)
		{
			g_Log.Write(_T("Identified FW7 GUID."));
			if (VersionInRange(Product->m_kpszMsiUpgradeTo, _T("7.0.4"), _T("7.0.32767")))
			{
				g_Log.Write(_T("MsiUpgradeTo version is %s, so we will try to launch EncConvertersAppDataMover30.exe"), Product->m_kpszMsiUpgradeTo);
				InitEC();
			}
			else
				g_Log.Write(_T("MsiUpgradeTo version is %s, so not part of 7.0.x."), Product->m_kpszMsiUpgradeTo);
		}
		else
			g_Log.Write(_T("Product GUID is not from FW7."));
	}
	else
		g_Log.Write(_T("Product is NULL."));

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

	// Examine the return value from MigrateSqlDbs.exe and prepare a text recommending whether or
	// not the user should allow uninstallation of an earlier version of FW.
	switch (migrationStatus)
	{
	case -1:
		g_Log.Write(_T("Data Migration failed for some unknown reason."));
		break;
	case 0:
		g_Log.Write(_T("Data Migration succeeded."));
		break;
	default:
		g_Log.Write(_T("%d projects failed to migrate."), migrationStatus);
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

		HideStatusDialog();

		g_Log.Write(_T("Notifying user about discovery of version %s"), pszVersion);

		int userResponse = DialogBoxParam(GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_DIALOG_EARLIER_FW_FOUND), NULL,  DlgProcEarlierFwDetected,
		(LPARAM)(pszVersion));

		if (userResponse >= 0)
		{
			if (userResponse & EARLIER_FW_REMOVE_SHORTCUTS)
			{
				g_Log.Write(_T("User opted to delete desktop shortcuts from older FW installation; deleting..."));
				g_Log.Indent();
				DeleteOldFwShortcuts();
				g_Log.Unindent();
				g_Log.Write(_T("...Done."));
			}
			else
				g_Log.Write(_T("User did not opt to delete desktop shortcuts."));

			if (userResponse & EARLIER_FW_UNINSTALL)
			{
				// User opted to uninstall earlier version of FW:
				g_Log.Write(_T("User opted to uninstall; uninstalling..."));
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
				g_Log.Write(_T("User did not opt to uninstall."));
		}
		else
			g_Log.Write(_T("Could not display dialog asking user if they wished to uninstall older FW and/or shortcuts. DialogBoxParam returned %d, GetLastError = %d."), userResponse, GetLastError());


		delete[] pszVersion;
		delete[] pszProductCode;

		ShowStatusDialog();
	}

	// If there is now no earlier FW installed, we should uninstall SQL Server (SILFW instance) if it is present:
	pszProductCode = DetectEarlierFwInstallation();
	if (pszProductCode)
	{
		g_Log.Write(_T("Earlier FW with product code %s detected, so won't uninstall SQL Server."), pszProductCode);
		delete[] pszProductCode;
	}
	else
		DisposeOfSqlServer();

	return 0;
}
