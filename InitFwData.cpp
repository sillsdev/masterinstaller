#pragma once

#include <tchar.h>

_TCHAR * pszFormerFwInstallationFolder = NULL;

// Utility to initialize FieldWorks access to SQL Server.
// This used to be done by FieldWorks applications, but is now done at the end
// of the installation sequence, while we still have administrator privileges.
int InitSQLServerForFW()
{
	DisplayStatusText(0, _T("Initializing FieldWorks data in SQL Server."));
	DisplayStatusText(1, _T(""));

	// Look up the FieldWorks code directory in the registry:
	_TCHAR * pszRootCodeDir = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\SIL\\FieldWorks"), _T("RootCodeDir"));

	if (!pszRootCodeDir)
	{
		g_Log.Write(_T("Could not read registry value RootCodeDir"));
		return -3;
	}

	// Form full path to DbAccess.dll:
	_TCHAR * pszDbAccessDllPath = MakePath(pszRootCodeDir, _T("DbAccess.dll"));
	delete[] pszRootCodeDir;
	pszRootCodeDir = NULL;

	// Load the DbAccess.dll:
	HMODULE hmodDbAccess = LoadLibrary(pszDbAccessDllPath);
	if (!hmodDbAccess)
	{
		g_Log.Write(_T("Could not load library %s"), pszDbAccessDllPath);
		delete[] pszDbAccessDllPath;
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
		delete[] pszDbAccessDllPath;
		return -5;
	}
	delete[] pszDbAccessDllPath;
	pszDbAccessDllPath = NULL;

	_ExtInitMSDE(NULL, NULL, "force", SW_SHOW);

	FreeLibrary(hmodDbAccess);
	hmodDbAccess = NULL;

	DisplayStatusText(0, _T("Completed initialization of FieldWorks data."));
	DisplayStatusText(1, _T(""));

	return 0;
}

void CopyOldFwData()
{
	// Version 4.2 or earlier of FW was previously installed. That means we have to run the
	// utility to transfer all the data files from the previous installation:
	DisplayStatusText(0, _T("Copying FieldWorks data to new location."));
	DisplayStatusText(1, _T(""));

	// Remove any trailing backslash from pszFormerFwInstallationFolder, or else the MoveData
	// utility will garble its command line arguments:
	RemoveTrailingBackslashes(pszFormerFwInstallationFolder);

	g_Log.Write(_T("Preparing to move data from %s - looking up new data folder..."),
		pszFormerFwInstallationFolder);

	// Look up new data location:
	_TCHAR * pszFwDataFolder = NewRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"),
		_T("RootDataDir"));

	if (!pszFwDataFolder)
		g_Log.Write(_T("...RootDataDir could not be read."));
	else
	{
		// Remove any trailing backslash, or else the MoveData utility will garble
		// its command line arguments:
		RemoveTrailingBackslashes(pszFwDataFolder);

		g_Log.Write(_T("...FW data folder = '%s'."), pszFwDataFolder);

		// Look up new code location:
		g_Log.Write(_T("Looking up new code folder..."));

		_TCHAR * pszFwCodeFolder = NewRegString(HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\SIL\\FieldWorks"), _T("RootCodeDir"));

		if (!pszFwCodeFolder)
			g_Log.Write(_T("...RootCodeDir could not be read."));
		else
		{
			g_Log.Write(_T("...FW code folder = '%s'."), pszFwCodeFolder);

			// Remove any trailing backslash:
			RemoveTrailingBackslashes(pszFwCodeFolder);

			// Make command line from code folder and utility name, with new and
			// old data folders as arguments:
			_TCHAR * pszCmd = new_sprintf(_T("\"%s\\MoveData.exe\" \"%s\" \"%s\""),
				pszFwCodeFolder, pszFwDataFolder, pszFormerFwInstallationFolder);

			// Run utility to move files:
			ExecCmd(pszCmd, NULL, true, _T("FieldWorks data file transfer"),
				_T("show"));

			delete[] pszFwCodeFolder;
			pszFwCodeFolder = NULL;
		}
		delete[] pszFwDataFolder;
		pszFwDataFolder = NULL;
	}
	delete[] pszFormerFwInstallationFolder;
	pszFormerFwInstallationFolder = NULL;
}

// Main function called to initialize SQL Server and if needed, copy over legacy data
// from previous releases of FW.
int InitFwData()
{
	g_Log.Write(_T("Starting InitFwData()..."));
	g_Log.Indent();

	if (pszFormerFwInstallationFolder)
	{
		g_Log.Write(_T("Former installation folder = %s"), pszFormerFwInstallationFolder);
		CopyOldFwData();
	}
	else
		g_Log.Write(_T("No former installation folder recorded"));

	int nRetVal = InitSQLServerForFW();
	g_Log.Unindent();
	g_Log.Write(_T("Ending InitFwData()"));
	return nRetVal;
}
