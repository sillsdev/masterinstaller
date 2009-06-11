/*
	Copyright (c) 2003 SIL International.

	This file is the basis of a DLL which is used to display the Setup Help files of products
	that need a license key to unlock their installers.

	Normally, InstallerHelp2.dll is built automatically by the Master Installer Configuration
	Utility. To build InstallerHelp2.dll manually, follow these steps:
	
	1) Use the Bin2Cpp utility (in Utils folder) to produce a .cpp file for each help file you
	wish to embed this way.
	2) Edit the FileList.cpp to #include each of the .cpp files produced by Bin2Cpp.
	3) Edit the Helps.cpp file to connect each help file to the array produced by the Bin2Cpp
	   utility.
	   For example: Translator's Workplace help file is SetupTW.rtf, so Bin2Cpp defines an array 
	   called  rgbSetupTW. So you would add this section:

			if (strcmp(pszStem, "SetupTW") == 0) // File name with extension omitted
			{
				pbBuffer = rgbSetupTW;
				cbSize = sizeof(rgbSetupTW) / sizeof(rgbSetupTW[0]);
			}

	4) Compile this project.

	To invoke InstallerHelp2.dll within a master installer, define in the master installer 
	configuation file an internal help file. For example, using Translator's Workplace as above:

	<Help Internal="true">SetupTW.rtf</Help>

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <vector>

#include "FileList.cpp"

HINSTANCE g_hinstDll = NULL;

const _TCHAR * pszErrorNotAvailable = _T("Help file not available.");
const _TCHAR * pszErrorAccess = _T("Error accessing help file.");

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hinstDll = hinstDLL;
	return true;
}

struct HelpThreadData
{
	const _TCHAR * pszStem;
	HANDLE hShutdownEvent;
};

DWORD WINAPI HelpThread(LPVOID lpParameter)
{
	HelpThreadData * htd = (HelpThreadData *)lpParameter;
	size_t cch = 1 + _tcslen(htd->pszStem);
	_TCHAR * pszStem = new _TCHAR [cch];
	_tcscpy_s(pszStem, cch, htd->pszStem);
	BYTE * pbBuffer = NULL;
	int cbSize = 0;
	// Look for embedded extension:
	_TCHAR * pszExtn = _tcsrchr(pszStem, '.');
	if (pszExtn)
	{
		*pszExtn = 0;
		pszExtn++;
	}
	if (!pszExtn)
	{
		MessageBox(NULL, pszErrorNotAvailable, _T(""), MB_OK);
		delete[] pszStem;
		return 0;
	}

// Include the file which picks out the embedded file from the available options:
#include "Helps.cpp"

	if (!pbBuffer)
	{
		MessageBox(NULL, pszErrorNotAvailable, _T(""), MB_OK);
		delete[] pszStem;
		return 0;
	}

	// Get the temp path:
	const _TCHAR * pszDefaultPath = _T("C:\\");
	_TCHAR pszTempPath[MAX_PATH];
	if (GetTempPath(MAX_PATH, pszTempPath) == 0)
		_tcscpy_s(pszTempPath, pszDefaultPath); // Use default instead
	
	// See if temp path exists:
	DWORD dwFileAttrib = GetFileAttributes(pszTempPath);
	bool fExists = (dwFileAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY));
	if (!fExists && _tcsicmp(pszTempPath, pszDefaultPath) != 0)
	{
		// Temp path doesn't exist, but we haven't tried the default path yet:
		_tcscpy_s(pszTempPath, MAX_PATH, pszDefaultPath); // Use default instead
		GetFileAttributes(pszTempPath);
		fExists = (dwFileAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwFileAttrib & FILE_ATTRIBUTE_DIRECTORY));
		if (!fExists)
		{
			MessageBox(NULL, pszErrorAccess, _T(""), MB_OK);
			delete[] pszStem;
			return 0;
		}
	}

	// Add stem file name:
	_tcscat_s(pszTempPath, MAX_PATH, pszStem);
	// When we run a hlp file, it will lead to the creation of a GID file:
	_TCHAR pszGidPath[MAX_PATH];
	_tcscpy_s(pszGidPath, MAX_PATH, pszTempPath);
	_tcscat_s(pszGidPath, MAX_PATH, _T(".gid"));
	// Add file extension:
	_tcscat_s(pszTempPath, MAX_PATH, _T("."));
	_tcscat_s(pszTempPath, MAX_PATH, pszExtn);
	FILE * f;
	if (_tfopen_s(&f, pszTempPath, _T("wb")) != 0)
	{
		MessageBox(NULL, pszErrorAccess, _T(""), MB_OK);
		delete[] pszStem;
		return 0;
	}

	for (int i = 0; i < cbSize; i++)
		fputc(pbBuffer[i] ^ 0xAA, f); // Reverse the minimal encryption.
	fclose(f);
	f = NULL;

	// We need to display the file in a process we control, so we can kill it if need be.
	// The ShellExecuteEx() API method enables us to do this. However, if the file extension
	// is .hlp, we need to run the WinHlp32.exe file in the Windows directory.
	// If we don't specify the directory, the system will likely run the one in
	// System32. If this happens, two processes get created, and the process id returned to
	// us isn't the one we can use to kill it later!
	HANDLE hProcess = NULL;
	if (_tcsicmp(pszExtn, _T("hlp")) == 0)
	{
		// Use the WinHlp32.exe file in the Windows directory:
		_TCHAR pszWindowsDirectory[MAX_PATH];
		unsigned int nWinDirRet = GetWindowsDirectory(pszWindowsDirectory, MAX_PATH);
		if (nWinDirRet > 0 && nWinDirRet < MAX_PATH)
		{
			// Don't allow the path to end with a backslash:
			RemoveTrailingBackslashes(pszWindowsDirectory);

			// Construct our command string:
			const _TCHAR * pszHelpCmd = _T("WinHlp32.exe");
			size_t cch = _tcslen(pszWindowsDirectory) + _tcslen(pszHelpCmd) + _tcslen(pszTempPath) + 7;
			_TCHAR * pszCmd = new _TCHAR [cch];
			_stprintf_s(pszCmd, cch, _T("\"%s\\%s\" %s"), pszWindowsDirectory, pszHelpCmd, pszTempPath);

			// Set up data for creating new process:
			BOOL bReturnVal = false;
			STARTUPINFO si;
			DWORD dwExitCode =	0;
			PROCESS_INFORMATION	process_info;

			ZeroMemory(&si,	sizeof(si));
			si.cb =	sizeof(si);

			// Launch new process:
			bReturnVal = CreateProcess(NULL, (LPTSTR)pszCmd, NULL, NULL, false, 0, NULL, NULL, &si,
				&process_info);
			delete[] pszCmd;
			pszCmd = NULL;
			if (bReturnVal)
			{
				CloseHandle(process_info.hThread);
				hProcess = process_info.hProcess;
			}
		}
		if (!hProcess)
		{
			MessageBox(NULL, pszErrorAccess, _T(""), MB_OK);
			delete[] pszStem;
			return 0;
		}
	}
	else // Not an .hlp file
	{
		SHELLEXECUTEINFO ExecInfo;
		ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ExecInfo.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NO_CONSOLE;
		ExecInfo.hwnd = NULL;
		ExecInfo.lpVerb = "open";
		ExecInfo.lpFile = pszTempPath;
		ExecInfo.lpParameters = NULL;
		ExecInfo.lpDirectory = NULL;
		ExecInfo.nShow = SW_SHOW;
		ExecInfo.hProcess = NULL;
		ShellExecuteEx(&ExecInfo);
		if (__int64(ExecInfo.hInstApp) > 32)
		{
			hProcess = ExecInfo.hProcess;
		}
	}

	if (hProcess)
	{
		// Wait till exit or shut-down event:
		HANDLE handles[2];
		handles[0] = htd->hShutdownEvent;
		handles[1] = hProcess;
		
		DWORD nRet = WaitForMultipleObjects(2, handles, false, INFINITE);
		if (nRet - WAIT_OBJECT_0 == 0)
		{
			// Main shutdown signalled, so kill our help process:
			TerminateProcess(hProcess, 0);
		}
		CloseHandle(hProcess);
	}
	DeleteFile(pszTempPath);
	DeleteFile(pszGidPath);
	delete[] pszStem;
	delete htd; // Created by thread-creator.
	return 0;
}

HANDLE Help(const _TCHAR * pszStem, HANDLE hShutdownEvent)
{
	// Carry out our work in a new thread:
	HelpThreadData * htd = new HelpThreadData; // deleted in thread proc.
	htd->hShutdownEvent = hShutdownEvent;
	htd->pszStem = pszStem;

	DWORD dwThreadId;
	HANDLE hReturn = CreateThread(NULL, 0, HelpThread, (LPVOID)htd, 0, &dwThreadId);

	return hReturn;
}


#ifdef _DEBUG

#include <conio.h>

void CALLBACK TestFromRunDll32(HWND hwnd, HINSTANCE hinst, LPTSTR lpCmdLine, int nCmdShow)
{
	HANDLE hShutdownEvent = ::CreateEvent(NULL, true, false, NULL);
	HANDLE hThread = Help(lpCmdLine, hShutdownEvent);
	bool fDieNow = false;
	while (!fDieNow) Sleep(100);
	::SetEvent(hShutdownEvent);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hShutdownEvent);
	CloseHandle(hThread);
}

#endif
