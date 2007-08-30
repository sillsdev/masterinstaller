#pragma once

#include <tchar.h>

#include "services.cpp"

// Utility to initialize FieldWorks access to SQL Server.
// This used to be done by FieldWorks applications, but is now done at the end
// of the installation sequence, while we still have administrator privileges.
int InitSQLServerForFW()
{
	const _TCHAR * pszDbAccessDll = _T("DbAccess.dll");
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

	int cchDbAccessDllPath = cbData + 1 + _tcslen(pszDbAccessDll);
	_TCHAR * pszDbAccessDllPath = new _TCHAR [cchDbAccessDllPath];

	lResult = RegQueryValueEx(hKey, _T("RootCodeDir"), NULL, NULL, LPBYTE(pszDbAccessDllPath), &cbData);
	if (ERROR_SUCCESS != lResult)
	{
		g_Log.Write(_T("Could not read registry value RootCodeDir"));
		return -3;
	}

	// Form full path to DbAccess.dll:
	if (pszDbAccessDllPath[cbData - 1] != '\\')
		_tcscat_s(pszDbAccessDllPath, cchDbAccessDllPath, _T("\\"));
	_tcscat_s(pszDbAccessDllPath, cchDbAccessDllPath, pszDbAccessDll);

	// Load the DbAccess.dll:
	HMODULE hmodDbAccess = LoadLibrary(pszDbAccessDllPath);
	if (!hmodDbAccess)
	{
		g_Log.Write(_T("Could not load library %s"), pszDbAccessDllPath);
		return -4;
	}

	// Get pointer to ExtInitMSDE function:
	typedef void (WINAPI * ExtInitMSDEFn)(HWND, HINSTANCE, LPSTR, int);
	ExtInitMSDEFn _ExtInitMSDE;
	_ExtInitMSDE = (ExtInitMSDEFn)GetProcAddress(hmodDbAccess, "ExtInitMSDE");

	if (!_ExtInitMSDE)
	{
		FreeLibrary(hmodDbAccess);
		g_Log.Write(_T("Could not find function ExtInitMSDE() in %s"), pszDbAccessDllPath);
		return -5;
	}

	_ExtInitMSDE(NULL, NULL, "force", SW_SHOW);

	FreeLibrary(hmodDbAccess);
	hmodDbAccess = NULL;

	return 0;
}

// Import the Surface Area settings
int SqlServer2005PostInstall(const _TCHAR * pszCriticalFile)
{
	// Locate the surface area settings file, should be in the same folder as the critical file:
	_TCHAR * pszCriticalFileCopy = my_strdup(pszCriticalFile);
	_TCHAR * pch = _tcsrchr(pszCriticalFileCopy, '\\');
	if (pch)
		*pch = 0;
	// Build full path by getting path of current root:
	const int knLen = MAX_PATH;
	_TCHAR szRootPath[knLen];
	GetModuleFileName(NULL, szRootPath, knLen);
	pch = _tcsrchr(szRootPath, _TCHAR('\\'));
	if (pch)
		pch++;
	else
		pch = szRootPath;
	*pch = 0;

	_TCHAR * pszSACFile = new_sprintf(_T("%s%s\\Surface Area Settings.xml"), szRootPath, pszCriticalFileCopy);
	delete[] pszCriticalFileCopy;
	pszCriticalFileCopy = NULL;

	// Check existence of file:
	FILE * f;
	if (_tfopen_s(&f, pszSACFile, _T("r")) != 0)
	{
		MessageBox(NULL, _T("Error: Cannot find Surface Area Settings.xml. SQL Server 2005 will be configured with default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot find SAC file '%s'"), pszSACFile);
		return -1;
	}
	fclose(f);
	f = NULL;

	// Check existence of SAC.exe utility:
	_TCHAR * pszSacPath = NULL;
	const _TCHAR * pszToolsKey = _T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\90\\Tools\\ClientSetup");
	LONG lResult;
	HKEY hKey = NULL;
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszToolsKey, NULL, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS)
	{
		// Get size of buffer needed:
		DWORD cbData = 0;
		lResult = RegQueryValueEx(hKey, _T("SQLBinRoot"), NULL, NULL, NULL, &cbData);

		// Get path to SAC.exe program:
		_TCHAR * pszSacFolderPath = new TCHAR [cbData];
		lResult = RegQueryValueEx(hKey, _T("SQLBinRoot"), NULL, NULL, (LPBYTE)pszSacFolderPath,
			&cbData);
		RegCloseKey(hKey);
		hKey = NULL;

		if (ERROR_SUCCESS == lResult)
			pszSacPath = new_sprintf(_T("%sSAC.exe"), pszSacFolderPath);

		delete[] pszSacFolderPath;
		pszSacFolderPath = NULL;
	}
	if (!pszSacPath)
	{
		MessageBox(NULL, _T("Error: Cannot find Surface Area Configuration tool. SQL Server 2005 will be configured with default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot find SAC utility."));
		return -1;
	}

	DisplayStatusText(0, _T("Configuring SQL Server: Starting SQL Server Service."));
	DisplayStatusText(1, _T(""));

	// Make sure SQL Server service is running:
	g_Log.Write(_T("Making sure SQL Server service is running."));
	ServiceManager_t ServiceManager;
	if (!ServiceManager.StartService(_T("MSSQL$SILFW")))
	{
		MessageBox(NULL, _T("Error: Cannot start SQL Server 2005. It will be left with default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot start SQL Server 2005."));
		return -1;
	}

	// Import SAC settings:
	_TCHAR * pszCmd = new_sprintf(_T("\"%s\" in \"%s\""), pszSacPath, pszSACFile);
	delete[] pszSACFile;
	pszSACFile = NULL;
	delete[] pszSacPath;
	pszSacPath = NULL;
	DisplayStatusText(0, _T("Configuring SQL Server: Importing settings."));
	g_Log.Write(_T("Importing Surface Area Configuration settings for SQL Server."));
	if (0 != ExecCmd(pszCmd, true, true, _T("configuration file for SQL Server 2005"), _T("show")))
	{
		MessageBox(NULL, _T("Error: Failed to configure SQL Server 2005. It will have default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot start SQL Server 2005."));
		return -1;
	}
	delete[] pszCmd;
	pszCmd = NULL;

	// Restart SQL Server service:
	DisplayStatusText(0, _T("Configuring SQL Server: Restarting SQL Server Service."));
	g_Log.Write(_T("Restarting SQL Server service."));
	if (!ServiceManager.RestartService(_T("MSSQL$SILFW"), true, 60000))
	{
		// Cannot restart SQL Server, so order a reboot:
		g_fRebootPending = true;
	}

	g_Log.Write(_T("Performing FieldWorks initialization of SQL Server..."));
	g_Log.Indent();
	int InitSqlRet = InitSQLServerForFW();
	g_Log.Unindent();
	g_Log.Write(_T("...done. InitSQLServerForFW() returned %d"), InitSqlRet);

	return 0;
}
