#pragma once

#include <tchar.h>

// Handle important stuff prior to installing SF Converters
int SfConverterPreinstall(SoftwareProduct * Product)
{
	g_Log.Write(_T("Starting SF Converter preinstallation function..."));
	g_Log.Indent();

	// We need to copy the VER.DLL file in the installation source folder to the WINDOWS
	// directory:
	_TCHAR * pszSourceFolder = my_strdup(Product->m_pszCriticalFileFlagTrue);
	if (!pszSourceFolder)
	{
		g_Log.Unindent();
		g_Log.Write(_T("...Error: could not duplicate Critical File."));
		return 0;
	}
	
	RemoveLastPathSection(pszSourceFolder);
	
	_TCHAR * pszVerDll = MakePath(pszSourceFolder, _T("VER.DLL"));

	delete[] pszSourceFolder;
	pszSourceFolder = NULL;

	_TCHAR * pszWindowsFolder = GetFolderPathNew(CSIDL_WINDOWS);	
	_TCHAR * pszDestFile = MakePath(pszWindowsFolder, _T("VER.DLL"));

	delete[] pszWindowsFolder;
	pszWindowsFolder = NULL;

	g_Log.Write(_T("Copying file '%s' to '%s'"), pszVerDll, pszDestFile);

	CopyFile(pszVerDll, pszDestFile, true);

	delete[] pszDestFile;
	pszDestFile = NULL;

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	return 0;
}
