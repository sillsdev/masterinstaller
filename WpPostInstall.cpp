#pragma once

#include <tchar.h>
#include <shlobj.h>

//#include "InitEC.cpp"
//#include "PreserveWordFormingCharOverrides.cpp"


// Since FW 5.3, we need to test for the presence of the Microsoft.mshtml.dll file on the user's
// machine, and if it is not present, install it by running vs_piaredist.exe, which we will
// have installed to [CommonFilesFolder]\SIL\FieldWorks.
/*
void InitMshtml()
{
	g_Log.Write(_T("Looking for Microsoft.mshtml.dll..."));
	g_Log.Indent();

	// The Microsoft.mshtml.dll will be in C:\Program Files\Microsoft.NET\Primary Interop Assemblies
	// if it is present, so we just need to get the localized form of Program Files:
	_TCHAR * pszProgramFilesPath = GetFolderPathNew(CSIDL_PROGRAM_FILES);
	
	// Form the full path to where the DLL should be:
	_TCHAR * pszMshtmlPath = new_sprintf(_T("%s\\Microsoft.NET\\Primary Interop Assemblies\\Microsoft.mshtml.dll"), pszProgramFilesPath);
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
		_TCHAR * pszVs_piaredistPath = new_sprintf(_T("%s\\SIL\\vs_piaredist.exe"), pszCommonPath);
		delete[] pszCommonPath;
		pszCommonPath = NULL;

		DisplayStatusText(0, _T("Installing Microsoft Primary Interoperability Assemblies..."));
		DisplayStatusText(1, _T(""));

		ExecCmd(pszVs_piaredistPath, NULL);
	}
	
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}
*/
// Utility to initialize FieldWorks InstallLanguage.
// This used to be done by FieldWorks applications, but is now done at the end
// of the installation sequence, while we still have administrator privileges.
// The only reason we need admin privileges is to reset the InitIcu registry flag
// once done.
int InitInstallLanguage()
{
	DisplayStatusText(0, _T("Initializing FieldWorks Install Language utility."));
	DisplayStatusText(1, _T(""));

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

	_TCHAR * pszRootCodeDir = new _TCHAR [cbData];

	lResult = RegQueryValueEx(hKey, _T("RootCodeDir"), NULL, NULL,
		LPBYTE(pszRootCodeDir), &cbData);

	RegCloseKey(hKey);
	hKey = NULL;

	if (ERROR_SUCCESS != lResult)
	{
		g_Log.Write(_T("Could not read registry value RootCodeDir"));
		return -3;
	}

	// Remove any trailing backslash from the root code folder:
	if (pszRootCodeDir[_tcslen(pszRootCodeDir) - 1] == '\\')
		pszRootCodeDir[_tcslen(pszRootCodeDir) - 1] = 0;

	// Form command line including full path to InstallLanguage.exe:
	_TCHAR * pszInstallLanguageCmd = new_sprintf(_T("%s\\InstallLanguage.exe -o"),
		pszRootCodeDir);

	// Now add the ICU DLL folder path to the PATH environment variable, so that 
	// when we call InstallLanguage, it can access the DLLs. This PATH setting will
	// already have been made by the FW installer, but because our process started
	// before that, we didn't get a copy.

	g_Log.Write(_T("Attempting to add directory location of ICU DLLs to PATH..."));
	g_Log.Indent();

	// Get localized path to C:\Program Files\Common Files:
	_TCHAR * pszCommonPath = GetFolderPathNew(CSIDL_PROGRAM_FILES_COMMON);

	// Form full path to ICU DLLs by adding "SIL":
	_TCHAR * pszIcuDllfolder = new_sprintf(_T("%s\\SIL"), pszCommonPath);
	delete[] pszCommonPath;
	pszCommonPath = NULL;

	// Add ICU DLL path to PATH environment variable:
	AddToPathEnvVar(pszIcuDllfolder);

	delete[] pszIcuDllfolder;
	pszIcuDllfolder = NULL;

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	// Run the utility:
	DWORD dwResult = ExecCmd(pszInstallLanguageCmd, NULL, true,
		_T("FieldWorks Install Language initialization"), _T("show"));

	delete[] pszInstallLanguageCmd;
	pszInstallLanguageCmd = NULL;

	// Respond to returned value:
	if (dwResult == 0 || dwResult == -999)
	{
		if (dwResult == -999)
			g_Log.Write(_T("InstallLanguage returned error code -999, but we don't mind."));

		// Reset the InitIcu registry flag:
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL"), 0, KEY_WRITE, &hKey);
		if (ERROR_SUCCESS == lResult)
		{
			DWORD dwZero = 0;
			lResult = RegSetValueEx(hKey, _T("InitIcu"), 0, REG_DWORD, (LPBYTE)&dwZero,
				sizeof(dwZero));

			if (ERROR_SUCCESS == lResult)
				g_Log.Write(_T("Reset InitIcu registry flag."));
			else
				g_Log.Write(_T("Failed to reset InitIcu registry flag."));

			RegCloseKey(hKey);
			hKey = NULL;
		}
		else
			g_Log.Write(_T("Could not open registry key HKEY_LOCAL_MACHINE\\SOFTWARE\\SIL for writing"));

	}
	else
	{
		_TCHAR * pszMsg = new_sprintf(_T("Error: Not all writing systems registered correctly. InstallLanguage returned error code %d."), dwResult);
		g_Log.Write(_T("Displaying error message '%s'"), pszMsg);
		::MessageBox(NULL, pszMsg, _T("Writing System Installation"), MB_ICONSTOP | MB_OK);
	}

	return 0;
}



int WpPostInstall(const _TCHAR * /*pszCriticalFile*/)
{
//	ReinstantiateWordFormingCharOverrides();

//	InitMshtml();

	InitInstallLanguage();
//	InitEC();

	return 0;
}
