#pragma once

#include <tchar.h>

#include "InitFwData.cpp"
#include "TestSqlSILFWPresence.cpp"
#include "InitEC.cpp"



// Utility to initialize FieldWorks InstallLanguage.
// This used to be done by FieldWorks applications, but is now done at the end
// of the installation sequence, while we still have administrator privileges.
// The only reason we need admin privileges is to reset the InitIcu registry flag
// once done.
int InitInstallLanguage()
{
	DisplayStatusText(0, _T("Initializing FieldWorks Install Language utility."));
	DisplayStatusText(1, _T(""));

	const _TCHAR * pszInstallLanguage = _T("InstallLanguage.exe");
	LONG lResult;
	HKEY hKey;

	// Look up the FieldWorks code directory in the registry:
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"), 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS != lResult)
	{
		g_Log.Write(_T("Could not find registry key HKEY_LOCAL_MACHINE\\SOFTWARE\\SIL\\FieldWorks"));
		return -1;
	}

	DWORD cbData = 0;

	// Fetch required buffer size:
	lResult = RegQueryValueEx(hKey, _T("RootCodeDir"), NULL, NULL, NULL, &cbData);
	if (cbData == 0)
	{
		g_Log.Write(_T("Could not find registry value RootCodeDir"));
		return -2;
	}

	int cchInstallLanguageCmd = cbData + 4 + _tcslen(pszInstallLanguage);
	_TCHAR * pszInstallLanguageCmd = new _TCHAR [cchInstallLanguageCmd];

	lResult = RegQueryValueEx(hKey, _T("RootCodeDir"), NULL, NULL,
		LPBYTE(pszInstallLanguageCmd), &cbData);

	RegCloseKey(hKey);
	hKey = NULL;

	if (ERROR_SUCCESS != lResult)
	{
		g_Log.Write(_T("Could not read registry value RootCodeDir"));
		return -3;
	}

	// Form command line including full path to InstallLanguage.exe:
	if (pszInstallLanguageCmd[_tcslen(pszInstallLanguageCmd) - 1] == '\\')
		pszInstallLanguageCmd[_tcslen(pszInstallLanguageCmd) - 1] = 0;
	_tcscat_s(pszInstallLanguageCmd, cchInstallLanguageCmd, _T("\\"));
	_tcscat_s(pszInstallLanguageCmd, cchInstallLanguageCmd, pszInstallLanguage);
	_tcscat_s(pszInstallLanguageCmd, cchInstallLanguageCmd, " -o");

	// Run the utility:
	ExecCmd(pszInstallLanguageCmd, false, true,
		_T("FieldWorks Install Language initialization"), _T("show"));

	return 0;
}



int FwPostInstall(const _TCHAR * /*pszCriticalFile*/)
{
	// If SQL Server 2005 is already present, then we need to initialize the FW data
	// now. Otherwise, this needs to wait until after SQL Server is installed.
	// (See SqlServer2005PostInstall.cpp for that case.)
	if (TestSqlSILFWPresence(_T("9.0.0.0"), _T("9.0.32767.32767"), NULL))
		InitFwData();

	InitInstallLanguage();
	InitEC();

	return 0;
}
