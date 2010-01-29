// This file is included in other C++ files, as needed, so that other definitions and includes
// can be tailored to the particular project using it.

// Replaces the deprecated strdup function.
// Caller must delete[] the result when finished.
_TCHAR * my_strdup(const _TCHAR * pszOriginal)
{
	if (!pszOriginal)
		return my_strdup(_T(""));

	int cch = (int)_tcslen(pszOriginal);
	_TCHAR * pszResult = new _TCHAR [1 + cch];
	_tcscpy_s(pszResult, 1 + cch, pszOriginal);
	return pszResult;
}

// Acts the same way as new_sprintf() below, except the variable arguments have already been
// collected.
// Caller must delete[] the return value.
_TCHAR * new_vsprintf(const _TCHAR * pszFormat, const va_list arglist)
{
	if (!pszFormat)
		return my_strdup(_T(""));

	int cchWksp = 100; // First guess at size needed.
	_TCHAR * szWksp = new _TCHAR [1 + cchWksp];

	// Format it with variable arguments, repeating until Wksp is big enough:
	int cch = _vsntprintf_s(szWksp, 1 + cchWksp, cchWksp, pszFormat, arglist);
	// If the reported number of _TCHARacters written is the same as the size of our buffer, then
	// the terminating zero will have been missed off!
	while (cch == -1 || cch == cchWksp)
	{
		delete[] szWksp;
		cchWksp *= 2;
		szWksp = new _TCHAR [1 + cchWksp];
		cch = _vsntprintf_s(szWksp, 1 + cchWksp, cchWksp, pszFormat, arglist);
	}
	return szWksp;
}

// Acts the same way as sprintf(), but creates a buffer to hold the text that is at least
// as big as the minimum needed.
// Caller must delete[] the return value.
_TCHAR * new_sprintf(const _TCHAR * pszFormat, ...)
{
	if (!pszFormat)
		return my_strdup(_T(""));

	// We will be passing on the variable arguments to the new_vsprintf() function:
	va_list arglist;
	va_start(arglist, pszFormat);

	_TCHAR * pszResult = new_vsprintf(pszFormat, arglist);
	return pszResult;
}

// Acts the same as new_vsprintf() above, only it appends the formatted string to rpszMain,
// having inserted ctInsertNewline newlines.
void new_vsprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline, const _TCHAR * pszAddendumFmt,
						 const va_list arglist)
{
	if (!pszAddendumFmt)
		return;

	_TCHAR * pszWksp;

	for (int n = 0; n < ctInsertNewline; n++)
	{
		pszWksp = new_sprintf(_T("%s\r\n"), rpszMain ? rpszMain : _T(""));
		delete[] rpszMain;
		rpszMain = pszWksp;
	}
	pszWksp = new_vsprintf(pszAddendumFmt, arglist);
	_TCHAR * pszWksp2 = new_sprintf(_T("%s%s"), rpszMain ? rpszMain : _T(""), pszWksp);
	delete[] pszWksp;
	pszWksp = NULL;

	delete[] rpszMain;
	rpszMain = pszWksp2;
}

// Acts the same as new_sprintf() above, only it appends the formatted string to rpszMain,
// having inserted ctInsertNewline newlines.
void new_sprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline, const _TCHAR * pszAddendumFmt, ...)
{
	// We will be passing on the variable arguments to the new_vsprintf_concat() function:
	va_list arglist;
	va_start(arglist, pszAddendumFmt);

	new_vsprintf_concat(rpszMain, ctInsertNewline, pszAddendumFmt, arglist);
}

// Write nIndent spaces at the start of the returned string, then treat the rest of the
// arguments as in a new_sprintf() call.
// Caller must delete[] the return value;
_TCHAR * new_ind_sprintf(int nIndent, const _TCHAR * pszFormat, ...)
{
	_TCHAR * pszWksp = NULL;

	for (int n = 0; n < nIndent; n++)
		new_sprintf_concat(pszWksp, 0, _T(" "));

	// We will be passing on the variable arguments to the new_vsprintf_concat() function:
	va_list arglist;
	va_start(arglist, pszFormat);

	new_vsprintf_concat(pszWksp, 0, pszFormat, arglist);

	return pszWksp;
}

// Removes all contiguous backslashes from the end of the given string.
void RemoveTrailingBackslashes(_TCHAR * pszFolder)
{
	size_t nLen = _tcslen(pszFolder);
	if (nLen >= 1)
	{
		if (pszFolder[nLen - 1] == '\\')
		{
			pszFolder[nLen - 1] = 0;
			RemoveTrailingBackslashes(pszFolder);
		}
	}
}

// Concatenates the two path fragments, making sure exactly one backslash is between them.
// Returns a new string that the caller must delete[].
_TCHAR * MakePath(const _TCHAR * pszFolder, const _TCHAR * pszFile, bool fQuoted)
{
	_TCHAR * pszFolderDup = my_strdup(pszFolder);
	RemoveTrailingBackslashes(pszFolderDup);

	while (pszFile[0] == '\\')
		pszFile++;

	_TCHAR * pszPath;

	if (_tcslen(pszFolderDup) > 0)
	{
		if (fQuoted)
			pszPath = new_sprintf(_T("\"%s\\%s\""), pszFolderDup, pszFile);
		else
			pszPath = new_sprintf(_T("%s\\%s"), pszFolderDup, pszFile);
	}
	else
	{
		if (fQuoted)
			pszPath = new_sprintf(_T("\"%s\""), pszFile);
		else
			pszPath = new_sprintf(_T("%s"), pszFile);
	}
	delete[] pszFolderDup;

	return pszPath;
}

// Terminates given string at the last backslash, thus removing any file name
// from a full path, or the last folder from a folder path.
void RemoveLastPathSection(_TCHAR * pszFilePath)
{
	_TCHAR * ch = _tcsrchr(pszFilePath, (_TCHAR)('\\'));
	if (ch)
		*ch = 0;
}

// Concatenates the two path fragments, making sure exactly one backslash is between them.
// Result overwrites the first argument, which must be dynamically allocated.
void NewConcatenatePath(_TCHAR *& pszFolder, const _TCHAR * pszFile)
{
	while (pszFolder[_tcslen(pszFolder) - 1] == '\\')
		pszFolder[_tcslen(pszFolder) - 1] = 0;

	while (pszFile[0] == '\\')
		pszFile++;

	new_sprintf_concat(pszFolder, 0, pszFile);
}

// Returns newly allocated string containing the specified registry value from the specified
// key. Caller must delete[] the returned value. pszValueName can be NULL to get default value.
// Returns NULL if something went wrong.
_TCHAR * NewRegString(HKEY hKeyRoot, const _TCHAR * pszSubKey, const _TCHAR * pszValueName)
{
	LONG lResult;
	HKEY hKey;

	const _TCHAR * pszKeyRoot = _T("");
	if (hKeyRoot == HKEY_CLASSES_ROOT)
		pszKeyRoot = _T("HKEY_CLASSES_ROOT");
	else if (hKeyRoot == HKEY_CURRENT_USER)
		pszKeyRoot = _T("HKEY_CURRENT_USER");
	if (hKeyRoot == HKEY_LOCAL_MACHINE)
		pszKeyRoot = _T("HKEY_LOCAL_MACHINE");
	else if (hKeyRoot == HKEY_USERS)
		pszKeyRoot = _T("HKEY_USERS");

	// Open Registry at required key:
	lResult = RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS != lResult)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Could not find registry key %s\\%s"), pszKeyRoot, pszSubKey);
#endif
		return NULL;
	}

	// Fetch required buffer size:
	DWORD cbData = 0;
	lResult = RegQueryValueEx(hKey, pszValueName, NULL, NULL, NULL, &cbData);
	if (cbData == 0)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Could not find registry value '%s' in key %s\\%s"),
			(pszValueName? pszValueName : _T("[Default]")), pszKeyRoot, pszSubKey);
#endif
		return NULL;
	}

	// Read actual data:
	_TCHAR * pszRetVal = new _TCHAR [cbData + 1];
	lResult = RegQueryValueEx(hKey, pszValueName, NULL, NULL, LPBYTE(pszRetVal), &cbData);

	RegCloseKey(hKey);
	hKey = NULL;

	if (ERROR_SUCCESS != lResult)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Could not read registry value '%s' in key %s\\%s"), pszValueName,
			pszKeyRoot, pszSubKey);
#endif
		delete[] pszRetVal;
		return NULL;
	}
	return pszRetVal;
}

// Returns a newly allocated buffer containing the path of the specified folder.
// Caller must delete[] the returned value.
// csidlFolder is the CSIDL of the folder requested.
// This routine uses the SHGetFolderPath, which may not be in Windows 98, so the DLL
// that has it is loaded dynamically.
_TCHAR * GetFolderPathNew(int csidlFolder)
{
#if !defined NOLOGGING
	g_Log.Write(_T("Attempting to retrieve folder path with CSIDL %d..."), csidlFolder);
	g_Log.Indent();
#endif

	// Prepare for dynamic loading of Shell32.dll, and use of a function which will be missing
	// on Windows 98:
	typedef HRESULT (WINAPI * fwpSHGetFolderPathFn)(HWND hwndOwner, int nFolder, HANDLE hToken,
		DWORD dwFlags, LPTSTR pszPath);
	fwpSHGetFolderPathFn _fwpSHGetFolderPath;

	HMODULE hmodfwpShell32 = LoadLibrary(_T("SHELL32.DLL"));
	if (hmodfwpShell32)
	{
		// Now get pointers to the functions we want to use:
#ifdef UNICODE
		_fwpSHGetFolderPath = (fwpSHGetFolderPathFn)GetProcAddress(hmodfwpShell32, "SHGetFolderPathW");
#else
		_fwpSHGetFolderPath = (fwpSHGetFolderPathFn)GetProcAddress(hmodfwpShell32, "SHGetFolderPathA");
#endif
	}
	else
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Could not load Shell32.dll"));
#endif
	}

	HRESULT hr = E_NOTIMPL;
	_TCHAR * pszReturnPath = NULL;

	if (_fwpSHGetFolderPath)
	{
		pszReturnPath = new _TCHAR [MAX_PATH];

		if (S_OK == _fwpSHGetFolderPath(NULL, csidlFolder, NULL, SHGFP_TYPE_CURRENT,
			pszReturnPath))
		{
			// Remove any trailing backslash from the folder:
			RemoveTrailingBackslashes(pszReturnPath);
#if !defined NOLOGGING
			g_Log.Write(_T("Retrieved %s"), pszReturnPath);
#endif
		}
		else
		{
#if !defined NOLOGGING
			g_Log.Write(_T("SHGetFolderPath did not return a folder path."));
#endif
			delete[] pszReturnPath;
			pszReturnPath = NULL;
		}
	}
	else
	{
#if !defined NOLOGGING
		g_Log.Write(_T("SHGetFolderPath function not found - can't fetch directory."));
#endif
	}

	if (hmodfwpShell32)
		FreeLibrary(hmodfwpShell32);
	hmodfwpShell32 = NULL;
	_fwpSHGetFolderPath = NULL;

#if !defined NOLOGGING
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
#endif

	return pszReturnPath;
}
