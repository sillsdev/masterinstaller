#pragma once

#include <tchar.h>


// Detect if there is a registry key signalling we are to run the KB908002 Fix, and
// run it if so. Then remove the temporary Extensibility.dll file, if it exists.
void RunKB908002Fix()
{
	g_Log.Write(_T("Checking whether to run KB908002 Fix..."));
	g_Log.Indent();

	HKEY hKey;
	// First, check to see if the fix is already installed:
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{09959E11-AD5D-408E-96AF-E3346954D6B8}"),
		0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		g_Log.Write(_T("KB908002 Fix is already installed; Product GUID {09959E11-AD5D-408E-96AF-E3346954D6B8} found in registry."));
		RegCloseKey(hKey);
	}
	else
	{
		// Test if a registry key was left telling us where the Fix can be found:
		const _TCHAR * pszKeyPath = _T("SOFTWARE\\SIL\\Installer\\EC\\MS KB908002 Fix");
		const _TCHAR * pszKeyValueRun = _T("Run");
		const _TCHAR * pszKeyValueDelete = _T("Delete");

		g_Log.Write(_T("Looking for temporary Extensibility.dll."));

		_TCHAR * pszExtensibilityPath = NewRegString(HKEY_LOCAL_MACHINE, pszKeyPath,
			pszKeyValueDelete);

		if (pszExtensibilityPath)
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
			g_Log.Write(_T("Could not read path."));

		delete[] pszExtensibilityPath;
		pszExtensibilityPath = NULL;

		_TCHAR * pszCmd = NewRegString(HKEY_LOCAL_MACHINE, pszKeyPath, pszKeyValueRun);
		if (pszCmd)
		{
			g_Log.Write(_T("About to launch '%s'."), pszCmd);
			ShowStatusDialog();
			DisplayStatusText(0, _T("Installing Shared Add-in Support Update for Microsoft .NET Framework 2.0 (KB908002)."));
			DisplayStatusText(1, _T("Please press the install button on its setup dialog."));

			ExecCmd(pszCmd, NULL, true);
		}
		else
			g_Log.Write(_T("Could not read 'Run' Command."));
	}
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

// Detect if any converters were installed to the machine, and if so,
// run the installer to put them in the repository.
int SetupInstalledConverters(SoftwareProduct * /*Product*/)
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

		state = MsiQueryFeatureState(
			_T("{F28612B2-B73F-4561-9B9F-1DA1F538020B}"), pszEcFeatures[i]);

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

	_TCHAR * pszPath = NewRegString(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\SIL\\SilEncConverters40\\Installer"), _T("InstallerPath"));

	if (!pszPath)
	{
		g_Log.Unindent();
		g_Log.Write(_T("...Done - could not find path of SetupSC.exe."));

		return 0;
	}

	ShowStatusDialog();
	DisplayStatusText(0, _T("Installing converters to repository"));
	DisplayStatusText(1, _T("Please follow instructions in dialog"));

	g_Log.Write(_T("Found ", pszPath));

	DWORD dwExitCode = ExecCmd(pszPath, NULL, true);

	delete[] pszPath;
	pszPath = NULL;

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	return dwExitCode;
}

// Call RunKB908002Fix and SetupInstalledConverters
int EncodingConvertersPostInstall(SoftwareProduct * Product)
{
	RunKB908002Fix();
	return SetupInstalledConverters(Product);
}
