#pragma once

#include <tchar.h>
#include <shlobj.h>
#include <msi.h>

// Returns the INSTALLDIR folder from the installation denoted by the given Product Code GUID.
_TCHAR * GetMsiComponentPath(const _TCHAR * pszProductCode, const _TCHAR * pszComponentId)
{
	// Get size of buffer needed for path:
	DWORD cchBuf = 0;
	DWORD ret = MsiGetComponentPath(pszProductCode, pszComponentId, NULL, &cchBuf);

	g_Log.Write(_T("MsiGetComponentPath initally returned %d"), ret);

	if (ret == INSTALLSTATE_LOCAL || ret == INSTALLSTATE_SOURCE)
	{
		_TCHAR * pszPath = new _TCHAR [++cchBuf];

		ret = MsiGetComponentPath(pszProductCode, pszComponentId, pszPath, &cchBuf);
		g_Log.Write(_T("MsiGetComponentPath subsequently returned %d"), ret);

		if (ret == INSTALLSTATE_LOCAL || ret == INSTALLSTATE_SOURCE)
			return pszPath;

		delete[] pszPath;
		pszPath = NULL;
	}
	return NULL;
}

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


int FwPostInstall(SoftwareProduct * Product)
{
	InitMshtml();

	// Run the MigrateSqlDbs.exe utility just installed:
	g_Log.Write(_T("About to run data migration utility..."));

	_TCHAR * pszMigrateUtilPath = GetMsiComponentPath(Product->m_kpszMsiProductCode, _T("{D25017CC-66F5-4BEE-B7BA-39BE8AE3698F}"));
	g_Log.Write(_T("Found MigrateSqlDbs path: %s"), pszMigrateUtilPath);

	if (pszMigrateUtilPath != NULL)
	{
		ShowStatusDialog();
		DisplayStatusText(0, _T("Migrating any FieldWorks data from earlier versions..."));
		DisplayStatusText(1, _T(""));

		_TCHAR * pszCmd = new_sprintf(_T("\"%s\" -autoclose"), pszMigrateUtilPath);
		ExecCmd(pszCmd, _T(""), true, NULL, _T("Monitor 4000"));

		delete[] pszCmd;
		pszCmd = NULL;
		delete[] pszMigrateUtilPath;
		pszMigrateUtilPath = NULL;
	}
	g_Log.Write(_T("...Done"));

	return 0;
}
