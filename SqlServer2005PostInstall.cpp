#pragma once

#include <tchar.h>

#include "services.cpp"
#include "InitFwData.cpp"


// Import the Surface Area settings
int ImportSACSettings(const _TCHAR * pszCriticalFile)
{
	// Locate the surface area settings file, should be in the same folder as the critical file:
	_TCHAR * pszCriticalFileCopy = my_strdup(pszCriticalFile);
	_TCHAR * pch = _tcsrchr(pszCriticalFileCopy, '\\');
	if (pch)
		pch++;
	else
		pch = pszCriticalFileCopy;
	*pch = 0;

	// Build full path by getting root path of current process:
	const int knLen = MAX_PATH;
	_TCHAR szRootPath[knLen];
	GetModuleFileName(NULL, szRootPath, knLen);
	pch = _tcsrchr(szRootPath, '\\');
	if (pch)
		pch++;
	else
		pch = szRootPath;
	*pch = 0;

	_TCHAR * pszSACFile;
	const _TCHAR * kpszSACFileName = _T("Surface Area Settings.xml");

	if (_tcslen(szRootPath) > 0 && _tcslen(pszCriticalFileCopy) > 0)
		pszSACFile = new_sprintf(_T("%s%s%s"), szRootPath, pszCriticalFileCopy, kpszSACFileName);
	else
	{
		if (_tcslen(szRootPath) > 0)
			pszSACFile = new_sprintf(_T("%s%s"), szRootPath, kpszSACFileName);
		else if (_tcslen(pszCriticalFileCopy) > 0)
			pszSACFile = new_sprintf(_T("%s%s"), pszCriticalFileCopy, kpszSACFileName);
		else
			pszSACFile = new_sprintf(_T("%s"), kpszSACFileName);
	}
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
		_TCHAR * pszSacFolderPath = new _TCHAR [cbData];
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
		return -2;
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
		return -3;
	}

	// Import SAC settings to the SILFW instance:
	// Note the capital 'I's in this command are required for Turkish Windows to work.
	_TCHAR * pszCmd = new_sprintf(_T("\"%s\" -ISILFW IN \"%s\""), pszSacPath, pszSACFile);
	delete[] pszSACFile;
	pszSACFile = NULL;
	delete[] pszSacPath;
	pszSacPath = NULL;
	DisplayStatusText(0, _T("Configuring SQL Server: Importing settings."));
	g_Log.Write(_T("Importing Surface Area Configuration settings for SQL Server."));

	// Arrange for utility to not display a window:
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	if (0 != ExecCmd(pszCmd, _T(""), true, _T("configuration file for SQL Server 2005"),
		_T("show"), 0, &si))
	{
		MessageBox(NULL, _T("Error: Failed to configure SQL Server 2005. It will have default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot start SQL Server 2005."));
		return -4;
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

int SqlServer2005PostInstall(const _TCHAR * pszCriticalFile)
{
	g_Log.Write(_T("Importing SQL Server surface area settings..."));
	g_Log.Indent();
	int ImportSACSettingsRet = ImportSACSettings(pszCriticalFile);
	g_Log.Unindent();
	g_Log.Write(_T("...Done. ImportSACSettings() returned %d"), ImportSACSettingsRet);

	g_Log.Write(_T("Performing FieldWorks initialization of SQL Server..."));
	g_Log.Indent();
	int InitFwDataRet = InitFwData();
	g_Log.Unindent();
	g_Log.Write(_T("...done. InitSQLServerForFW() returned %d"), InitFwDataRet);

	return 0;
}
