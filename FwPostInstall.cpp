#pragma once

#include <tchar.h>
#include <shlobj.h>

#include "InitEC.cpp"
#include "PreserveWordFormingCharOverrides.cpp"


// Since FW 5.3, we need to test for the presence of the Microsoft.mshtml.dll file on the user's
// machine, and if it is not present, install it by running vs_piaredist.exe, which we will
// have installed to [CommonFilesFolder]\SIL\FieldWorks.
void InitMshtml()
{
	g_Log.Write(_T("Looking for Microsoft.mshtml.dll..."));
	g_Log.Indent();

	// The Microsoft.mshtml.dll will be in C:\Program Files\Microsoft.NET\Primary Interop Assemblies
	// if it is present, so we just need to get the localized form of Program Files:
	_TCHAR * pszProgramFilesPath = GetFolderPathNew(CSIDL_PROGRAM_FILES);
	
	// Form the full path to where the DLL should be:
	_TCHAR * pszMshtmlPath = MakePath(pszProgramFilesPath,
		_T("Microsoft.NET\\Primary Interop Assemblies\\Microsoft.mshtml.dll"));
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
		_TCHAR * pszVs_piaredistPath = MakePath(pszCommonPath, _T("SIL\\vs_piaredist.exe"));
		delete[] pszCommonPath;
		pszCommonPath = NULL;

		DisplayStatusText(0, _T("Installing Microsoft Primary Interoperability Assemblies..."));
		DisplayStatusText(1, _T(""));

		ExecCmd(pszVs_piaredistPath, NULL);

		delete[] pszVs_piaredistPath;
		pszVs_piaredistPath = NULL;
	}
	delete[] pszMshtmlPath;
	pszMshtmlPath = NULL;
	
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}


int FwPostInstall(SoftwareProduct * /*Product*/)
{
	ReinstantiateWordFormingCharOverrides();

	InitMshtml();
	InitEC();

	return 0;
}
