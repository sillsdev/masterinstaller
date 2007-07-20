#pragma once

#include <TCHAR.h>

// If fTestOnly is true, tests to see if the keyman keyboards in pszKeyboardsPath are installed.
// If fTestOnly is false, installs the keyman keyboards from pszKeyboardsPath.
DWORD KeymanFunction(bool fTestOnly, const TCHAR * pszKeyboardsPath)
{
	const TCHAR * pszTitle = _T("Keyman Keyboard Installer");
	const TCHAR * pszErrReg = _T("Cannot find Keyman shell. Keyboards will not be installed.");
	const TCHAR * pszErrNoFiles = _T("No Keyman files found in specified folder.");
	const TCHAR * pszErrLaunchKeyman = _T("Could not run Keyman to install keyboards.");

	HKEY hKey;
	const int ctchShellCmd = 300;
	TCHAR pszShellCmd[ctchShellCmd];

	if (!fTestOnly)
	{
		// Locate Keyman Shell:
		LONG lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
			_T("Keyman.File.Package\\Shell\\open\\Command"), 0, KEY_READ, &hKey);

		if (ERROR_SUCCESS != lResult)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Read the Keyman shell command line:
		DWORD cbData = ctchShellCmd;
		lResult = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)pszShellCmd, &cbData);
		RegCloseKey(hKey);
		hKey = NULL;
		if (ERROR_SUCCESS != lResult)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Look for the "-i" part of the shell command:
		TCHAR *ch = _tcsstr(pszShellCmd, _T("-i"));
		if (!ch)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Remove everything after the "-i":
		ch += 2;
		*ch = 0;
	}

	// Using path of .exe  plus pszKeyboardsPath as the source path, find
	// (and install if !fTestOnly) all .kmx files:
	const int knLen = MAX_PATH + 20;
	TCHAR pszSearch[knLen];
	TCHAR pszFolder[knLen];
	GetModuleFileName(NULL, pszSearch, knLen);
	TCHAR * pszFolderEnd = _tcsrchr(pszSearch, _TCHAR('\\'));
	if (pszFolderEnd)
		*pszFolderEnd = 0;
	_tcscat_s(pszSearch, knLen, _T("\\"));
	_tcscat_s(pszSearch, knLen, pszKeyboardsPath);
	_tcscpy_s(pszFolder, knLen, pszSearch);
	_tcscat_s(pszSearch, knLen, _T("\\*.kmx"));

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(pszSearch, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, pszErrNoFiles, pszTitle, MB_ICONSTOP | MB_OK);
		return 0;
	}
	do
	{
		// See if Keyboard is already installed:
		bool fKeyboardExists = false;
		const int kcchReg = 1000;
		TCHAR pszRegKey[kcchReg];
		_tcscpy_s(pszRegKey, kcchReg, _T("Software\\Tavultesoft\\Keyman\\6.0\\Active Keyboards\\"));
		_tcscat_s(pszRegKey, kcchReg, wfd.cFileName);
		// Remove file extension:
		TCHAR * ch = _tcsrchr(pszRegKey, _TCHAR('.'));
		if (ch)
			*ch = 0;
		HKEY hKeyTemp;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszRegKey, 0, KEY_READ, &hKeyTemp))
		{
			fKeyboardExists = true;
			RegCloseKey(hKeyTemp);
			hKeyTemp = NULL;
		}

		if (!fKeyboardExists)
		{
			if (fTestOnly)
			{
				FindClose(hFind);
				return -1; // At least one keyboard is not installed.
			}

			// Make up command line to install current keyboard:
			const int cchBuf = 1000;
			_TCHAR pszCmd[cchBuf];
			_stprintf_s(pszCmd, cchBuf, _T("%s -s \"%s\\%s\""), pszShellCmd, pszFolder, wfd.cFileName);

			// Execute command:
			BOOL bReturnVal = false;
			STARTUPINFO si;
			DWORD dwExitCode =	0;
			PROCESS_INFORMATION	process_info;

			ZeroMemory(&si,	sizeof(si));
			si.cb =	sizeof(si);

			// Launch new process:
			bReturnVal = CreateProcess(NULL, (LPTSTR)pszCmd, NULL, NULL, false, 0, NULL, NULL, &si,
				&process_info);

			if (!bReturnVal)
			{
				MessageBox(NULL, pszErrLaunchKeyman, pszTitle, MB_ICONSTOP | MB_OK);
				return 0;
			}

			CloseHandle(process_info.hThread);
			// Wait for Keyman to finish:
			WaitForSingleObject(process_info.hProcess, INFINITE);
			CloseHandle(process_info.hProcess);
		} // End if !fKeyboardExists
	} while (FindNextFile(hFind, &wfd));
	
	FindClose(hFind);
	hFind = NULL;

	return 0;
}

bool TestKeymanKeyboardsPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/, const TCHAR * pszCriticalFile)
{
	return (0 == KeymanFunction(true, pszCriticalFile));
}
