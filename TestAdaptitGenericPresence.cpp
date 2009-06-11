#pragma once

#include <tchar.h>

// Creates a string containing the path to the Adapt It installation folder, if it exists.
// This folder contains both the executable for Adapt It, and the uninstall.exe program to
// remove it.
// The returned path does not end with a backslash '\'.
// The caller must delete[]  the string afterwards.
_TCHAR * MakePathToAdaptIt(bool fUnicode)
{
	bool fResult = false;
	_TCHAR * pszKeyPath;
	const _TCHAR * pszRegRoot = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

	if (fUnicode)
	{
		// See if Unicode version is present:
		pszKeyPath = MakePath(pszRegRoot, _T("Adapt It WX Unicode"));
#if !defined NOLOGGING
		g_Log.Write(_T("Looking for Adapt It Unicode path in HKEY_LOCAL_MACHINE\\%s..."), pszKeyPath);
#endif
	}
	else
	{
		// See if non-Unicode version is present:
		pszKeyPath = MakePath(pszRegRoot, _T("Adapt It WX"));
#if !defined NOLOGGING
		g_Log.Write(_T("Looking for Adapt It path in HKEY_LOCAL_MACHINE\\%s..."), pszKeyPath);
#endif
	}

	_TCHAR * pszPath = NewRegString(HKEY_LOCAL_MACHINE, pszKeyPath, _T("UninstallString"));

	delete[] pszKeyPath;
	pszKeyPath = NULL;

	if (pszPath)
	{
		// Truncate the uninstall.exe bit at the end of the string:
		RemoveLastPathSection(pszPath);

#if !defined NOLOGGING
		g_Log.Write(_T("...Found at %s"), pszPath);
#endif
		return pszPath;
	}

#if !defined NOLOGGING
	g_Log.Write(_T("...Not found"));
#endif
	return NULL;
}


// Generic test of Adapt It installation.
// We read the location of the "Adapt It changes.txt" file from the registry, then read the
// first line of that file to see what version it is.
bool TestAdaptitGenericPresence(bool fUnicode, const _TCHAR * pszMinVersion,
								const _TCHAR * pszMaxVersion)
{
	_TCHAR * pszAdaptItFolder = MakePathToAdaptIt(fUnicode);
	if (!pszAdaptItFolder)
		return false;
	_TCHAR * pszAdaptItChanges = MakePath(pszAdaptItFolder, _T("Adapt It changes.txt"));
	delete[] pszAdaptItFolder;
	pszAdaptItFolder = NULL;

#if !defined NOLOGGING
	g_Log.Write(_T("Opening '%s' to read version number."), pszAdaptItChanges);
#endif

	// See what version the changes file claims to be:
	FILE * f = NULL;
	_TCHAR * pszVersion = NULL;
	if (0 == _tfopen_s(&f, pszAdaptItChanges, _T("rt")))
	{
		// Get first line of Changes file:
		const int cchLine = 200;
		_TCHAR pszLine[cchLine] = { 0 };
		_fgetts(pszLine, cchLine, f);
		fclose(f);
		f = NULL;

#if !defined NOLOGGING
		g_Log.Write(_T("First line of '%s' is '%s'."), pszAdaptItChanges, pszLine);
#endif

		// Test if it starts with "Version ":
		const _TCHAR * pszHeader = _T("Version ");
		int cchHeader = _tcslen(pszHeader);
		if (_tcsncicmp(pszLine, pszHeader, cchHeader) == 0)
		{
			// The next characters up to the space represent the version:
			pszVersion = &pszLine[cchHeader];
			// Terminate the string after the version number:
			_TCHAR * pszHeaderEnd = pszVersion;
			while (*pszHeaderEnd != 0 && *pszHeaderEnd != ' ')
				pszHeaderEnd++;
			if (*pszHeaderEnd == ' ')
				*pszHeaderEnd = 0;

#if !defined NOLOGGING
			g_Log.Write(_T("Extracted version string '%s'."), pszVersion);
#endif
		}
	}
	else
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Could not open '%s'."), pszAdaptItChanges);
#endif
	}
	delete[] pszAdaptItChanges;
	pszAdaptItChanges = NULL;

	if (pszVersion)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Found Adapt It%sversion %s"), fUnicode? _T(" Unicode ") : _T(" "), pszVersion);
#endif

		return VersionInRange(pszVersion, pszMinVersion, pszMaxVersion);
	}
	return false;
}
