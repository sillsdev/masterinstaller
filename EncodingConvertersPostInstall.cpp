#pragma once

#include <tchar.h>


// Detect if there is a registry key signalling we are to run the KB908002 Fix, and
// run it if so.
void RunKB908002Fix()
{
	g_Log.Write(_T("Checking whether to run KB908002 Fix..."));
	g_Log.Indent();

	TCHAR * pszCmd = NULL;
	HKEY hKey;
	const TCHAR * pszKeyPath = _T("SOFTWARE\\SIL\\Installer\\EC\\MS KB908002 Fix");
	const TCHAR * pszKeyValue = _T("Run");
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeyPath, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		g_Log.Write(_T("Opened reg key."));
		DWORD cbData = 0;		
		lResult = RegQueryValueEx(hKey, pszKeyValue, NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			g_Log.Write(_T("Got required buffer size."));
			cbData++;
			pszCmd = new TCHAR [cbData];
			lResult = RegQueryValueEx(hKey, pszKeyValue, NULL, NULL, (LPBYTE)pszCmd,
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
		RegCloseKey(hKey);

		// Remove the registry setting:
		RegDeleteKey(HKEY_LOCAL_MACHINE, pszKeyPath);
	}

	if (pszCmd)
	{
		g_Log.Write(_T("About to launch '%s'."), pszCmd);
		ShowStatusDialog();
		DisplayStatusText(0, _T("Installing Microsoft KB908002 Fix for Word macros"));
		DisplayStatusText(1, _T("Please follow instructions in dialog"));

		ExecCmd(pszCmd, false, true);
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
	};

	bool fFoundOne = false;
	for (int i = 0; i < (sizeof(pszEcFeatures) / sizeof(pszEcFeatures[0])); i++)
	{
		g_Log.Write(_T("Testing for %s"), pszEcFeatures[i]);

		INSTALLSTATE state = INSTALLSTATE_UNKNOWN;

		state = WindowsInstaller.MsiQueryFeatureState(
			_T("{6242D132-64F4-46E3-9F39-26B02215B896}"), pszEcFeatures[i]);

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

		dwExitCode = ExecCmd(pszSetupSC, false, true);
	}

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	return dwExitCode;
}

// Call RunKB908002Fix and SetupInstalledConverters
int EncodingConvertersPostInstall(const TCHAR * pszCriticalFile)
{
	RunKB908002Fix();
	return SetupInstalledConverters(pszCriticalFile);
}