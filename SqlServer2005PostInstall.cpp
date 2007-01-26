#pragma once

#include <tchar.h>

#include "services.cpp"

// Forward declarations:
BOOL StartSampleService(SC_HANDLE schSCManager);

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

	return 0;
}
