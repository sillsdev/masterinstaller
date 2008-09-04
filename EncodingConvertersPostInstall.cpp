#pragma once

#include <tchar.h>

#include "InitEC.cpp"


// Detect if there is a registry key signalling we are to run the KB908002 Fix, and
// run it if so. Then remove the temporary Extensibility.dll file, if it exists.
void RunKB908002Fix()
{
	g_Log.Write(_T("Checking whether to run KB908002 Fix..."));
	g_Log.Indent();

	TCHAR * pszCmd = NULL;
	TCHAR * pszExtensibilityPath = NULL;
	HKEY hKey;
	const TCHAR * pszKeyPath = _T("SOFTWARE\\SIL\\Installer\\EC\\MS KB908002 Fix");
	const TCHAR * pszKeyValueRun = _T("Run");
	const TCHAR * pszKeyValueDelete = _T("Delete");
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyPath, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		g_Log.Write(_T("Opened reg key."));
		DWORD cbData = 0;		
		lResult = RegQueryValueEx(hKey, pszKeyValueRun, NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			g_Log.Write(_T("Got required buffer size for 'run' file."));
			cbData++;
			pszCmd = new TCHAR [cbData];
			lResult = RegQueryValueEx(hKey, pszKeyValueRun, NULL, NULL, (LPBYTE)pszCmd,
				&cbData);
			if (ERROR_SUCCESS == lResult)
				g_Log.Write(_T("Got executable path: '%s'."), pszCmd);
			else
			{
				g_Log.Write(_T("Could not read path."));
				delete[] pszCmd;
				pszCmd = NULL;
			}
		}

		g_Log.Write(_T("Looking for temporary Extensibility.dll."));
		cbData = 0;		
		lResult = RegQueryValueEx(hKey, pszKeyValueDelete, NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			g_Log.Write(_T("Got required buffer size."));
			cbData++;
			pszExtensibilityPath = new TCHAR [cbData];
			lResult = RegQueryValueEx(hKey, pszKeyValueDelete, NULL, NULL, (LPBYTE)pszExtensibilityPath,
				&cbData);
			if (ERROR_SUCCESS == lResult)
			{
				g_Log.Write(_T("Deleting Extensibility.dll path: '%s'."), pszExtensibilityPath);
				if (0 == DeleteFile(pszExtensibilityPath))
				{
					int err = GetLastError();
					g_Log.Write(_T("'%s' was not deleted [error %d]."), pszExtensibilityPath, err);
				}
				g_Log.Write(_T("'%s' deleted successfully."), pszExtensibilityPath);
			}
			else
			{
				g_Log.Write(_T("Could not read path."));
				delete[] pszExtensibilityPath;
				pszExtensibilityPath = NULL;
			}
		}

		RegCloseKey(hKey);
	}

	if (pszCmd)
	{
		g_Log.Write(_T("About to launch '%s'."), pszCmd);
		ShowStatusDialog();
		DisplayStatusText(0, _T("Installing Shared Add-in Support Update for Microsoft .NET Framework 2.0 (KB908002)."));
		DisplayStatusText(1, _T("Please press the install button on its setup dialog."));

		ExecCmd(pszCmd, NULL, true);
	}
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

// Detect if any converters were installed to the machine, and if so,
// run the installer to put them in the repository.
int SetupInstalledConverters(const TCHAR * /*pszCriticalFile*/)
{
	g_Log.Write(_T("Looking for installed converters..."));
	g_Log.Indent();

	// See if any converters were installed:
	const TCHAR * pszEcFeatures [] =
	{
		_T("BasicConverters"),
		_T("ICUTransliterators"),
		_T("FindPhone2IPA"),
		_T("SAGIndic"),
		_T("Cameroon"),
		_T("Central_Africa_converters"),
		_T("East_Africa_converters"),
		_T("West_Africa_Converters"),
		_T("Eastern_Congo_Group"),
		_T("NLCI_India"),
		_T("Hebrew"),
		_T("Indic_converters"),
		_T("Papua_New_Guinea_converters"),
	};

	bool fFoundOne = false;
	for (int i = 0; i < (sizeof(pszEcFeatures) / sizeof(pszEcFeatures[0])); i++)
	{
		g_Log.Write(_T("Testing for %s"), pszEcFeatures[i]);

		INSTALLSTATE state = INSTALLSTATE_UNKNOWN;

		state = WindowsInstaller.MsiQueryFeatureState(
			_T("{287F6ADF-DA8A-46fc-8BE0-351AA6412B79}"), pszEcFeatures[i]);

		switch (state)
		{
		case INSTALLSTATE_ADVERTISED:
		case INSTALLSTATE_LOCAL:
		case INSTALLSTATE_SOURCE:
		case INSTALLSTATE_DEFAULT:
			fFoundOne = true;
			g_Log.Write(_T("This one was installed."));
			break;
		}
		if (fFoundOne)
			break;
	}
	if (!fFoundOne)
	{
		g_Log.Unindent();
		g_Log.Write(_T("...Done - didn't find any."));
		return 0;
	}

	g_Log.Write(_T("Looking for path of SetupSC.exe."));

	_TCHAR * pszPath = NULL;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\SIL\\SilEncConverters22\\Installer"), NULL, KEY_READ, &hKey);

	// We don't proceed unless the call above succeeds:
	if (ERROR_SUCCESS == lResult)
	{
		DWORD cbData = 0;

		// Find out how much space we need to hold path string:
		lResult = RegQueryValueEx(hKey, _T("InstallerPath"), NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			pszPath = new TCHAR [cbData];
			RegQueryValueEx(hKey, _T("InstallerPath"), NULL, NULL, (LPBYTE)pszPath, &cbData);
		}
	}

	if (!pszPath)
	{
		g_Log.Unindent();
		g_Log.Write(_T("...Done - could not find path of SetupSC.exe."));

		return 0;
	}

	TCHAR * pszSetupSC = NULL;
	new_sprintf_concat(pszSetupSC, 0, _T("%s\\SetupSC.exe"), pszPath);

	DWORD dwExitCode = 0;

	if (pszSetupSC)
	{
		ShowStatusDialog();
		DisplayStatusText(0, _T("Installing converters to repository"));
		DisplayStatusText(1, _T("Please follow instructions in dialog"));

		dwExitCode = ExecCmd(pszSetupSC, NULL, true);
	}

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	return dwExitCode;
}

// Call RunKB908002Fix and SetupInstalledConverters
int EncodingConvertersPostInstall(const TCHAR * pszCriticalFile)
{
	InitEC();
	RunKB908002Fix();
	return SetupInstalledConverters(pszCriticalFile);
}
