#pragma once

#include <tchar.h>
#include <shlobj.h>

// Tests for presence of Microsoft XML parser 3.
bool TestMsXml3Presence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						const _TCHAR * /*pszCriticalFile*/)
{
	// Get path to main DLL, typically C:\Windows\System32\MSXML3.dll:
	_TCHAR * pszSystemFolderPath = GetFolderPathNew(CSIDL_SYSTEM);
	_TCHAR * pszDllPath = MakePath(pszSystemFolderPath, _T("MSXML3.dll"));
	delete[] pszSystemFolderPath;

#if !defined NOLOGGING
	g_Log.Write(_T("Testing file version of '%s'"), pszDllPath);
#endif

	// Get file version:
	__int64 nVersion;
	bool fFound = GetFileVersion(pszDllPath, nVersion);

	delete[] pszDllPath;
	pszDllPath = NULL;

	if (fFound)
	{
#if !defined NOLOGGING
		TCHAR * pszVersion = GenVersionText(nVersion);
		g_Log.Write(_T("Found MSXML3 version %s"), pszVersion);
		delete[] pszVersion;
		pszVersion = NULL;
#endif

		return VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
	}

#if !defined NOLOGGING
	g_Log.Write(_T("File not found"));
#endif

	return false;
}



