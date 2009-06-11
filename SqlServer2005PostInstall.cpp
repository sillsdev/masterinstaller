#pragma once

#include <tchar.h>

#include "services.cpp"
#include "InitFwData.cpp"


// Import the Surface Area settings
int ImportSACSettings(SoftwareProduct * Product)
{
	// The surface area settings file should be in the same folder as the critical file:
	_TCHAR * pszCriticalFileFolderPath = my_strdup(Product->GetCriticalFile());
	// Get folder path of critical file:
	RemoveLastPathSection(pszCriticalFileFolderPath);

	// Build full path by getting root path of current process:
	_TCHAR * pszRootPath = NewGetExeFolder();
	_TCHAR * pszTemp = MakePath(pszRootPath, pszCriticalFileFolderPath);
	_TCHAR * pszSACFile = MakePath(pszTemp, _T("Surface Area Settings.xml"));

	// Delete temporary strings:
	delete[] pszRootPath;
	pszRootPath = NULL;
	delete[] pszCriticalFileFolderPath;
	pszCriticalFileFolderPath = NULL;
	delete[] pszTemp;
	pszTemp = NULL;

	// Check existence of SAC settings file by trying to open it for reading:
	FILE * f;
	if (_tfopen_s(&f, pszSACFile, _T("r")) != 0)
	{
		MessageBox(NULL, _T("Error: Cannot find Surface Area Settings.xml. SQL Server 2005 will be configured with default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot find SAC file '%s'"), pszSACFile);
		delete[] pszSACFile;
		pszSACFile = NULL;
		return -1;
	}
	fclose(f);
	f = NULL;

	// Check existence of SAC.exe utility:
	_TCHAR * pszSacPath = NULL;
	_TCHAR * pszSacFolderPath = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\90\\Tools\\ClientSetup"),
		_T("SQLBinRoot"));

	if (pszSacFolderPath)
	{
		pszSacPath = MakePath(pszSacFolderPath, _T("SAC.exe"));

		delete[] pszSacFolderPath;
		pszSacFolderPath = NULL;
	}
	if (!pszSacPath)
	{
		MessageBox(NULL, _T("Error: Cannot find Surface Area Configuration tool. SQL Server 2005 will be configured with default settings. Some features may not work."), g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Cannot find SAC utility."));
		delete[] pszSACFile;
		pszSACFile = NULL;
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

		delete[] pszSACFile;
		pszSACFile = NULL;
		delete[] pszSacPath;
		pszSacPath = NULL;

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

int SqlServer2005PostInstall(SoftwareProduct * Product)
{
	g_Log.Write(_T("Importing SQL Server surface area settings..."));
	g_Log.Indent();
	int ImportSACSettingsRet = ImportSACSettings(Product);
	g_Log.Unindent();
	g_Log.Write(_T("...Done. ImportSACSettings() returned %d"), ImportSACSettingsRet);

	g_Log.Write(_T("Performing FieldWorks initialization of SQL Server..."));
	g_Log.Indent();
	int InitFwDataRet = InitFwData();
	g_Log.Unindent();
	g_Log.Write(_T("...done. InitSQLServerForFW() returned %d"), InitFwDataRet);

	return 0;
}
