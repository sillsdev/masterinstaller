/*
	Copyright (c) 2003 SIL International.

	Master installer auxilary functions file.
*/

#include <Windows.h>
#include <stdio.h>

#include "UsefulStuff.h"
#include "Globals.h"
#include "LogFile.h"
#include "Dialogs.h"
#include "resource.h"
#include "ErrorHandler.h"

char g_rgchActiveProcessDescription[100] = "";

// Forward declarations:
DWORD WINAPI StatusDlgMonitorThreadEntry(LPVOID);
struct MonitorThreadData_t
{
	DWORD m_dwProcessId;
	bool * m_pfTerminationFlag;
	int nInitialDelayMs;
	MonitorThreadData_t() : m_dwProcessId(0), m_pfTerminationFlag(NULL), nInitialDelayMs(0) { }
};

// Executes the command in the given string and, if fWaitTillExit is true, waits for
// the launched process to exit.
// If fUseCurrentDir is true, the Windows current path is temporarily set to the same path as
// the location of the main .exe file
// If pszDescription is set, its value is copied to a global variable which can be used in a
// status window when attempting to quit the created process.
// If pszStatusWindowControl is set, it contains a string describing what should happen to the
// status window: show it, hide it or remove it if the new process has any windows visible.
DWORD ExecCmd(LPCTSTR pszCmd, bool fUseCurrentDir, bool fWaitTillExit,
			  const char * pszDescription, const char * pszStatusWindowControl)
{
	// Preserve current directory:
	const int knLen = MAX_PATH;
	char szOldPath[knLen];
	::GetCurrentDirectory(knLen, szOldPath);

	if (fUseCurrentDir)
	{
		// Set current directory to the path where current process was launched from:
		char szPath[knLen];
		GetModuleFileName(NULL, szPath, knLen);
		char * ch = strrchr(szPath, '\\');
		if (ch)
			ch++;
		else
			ch = szPath;
		*ch = 0;
		::SetCurrentDirectory(szPath);
	}

	// Make copy of command:
	char * pszCmdCopy = strdup(pszCmd);

	// If command contains "msiexec" then we will prefix this with the path to msiexec.exe:
	char * pszMsiExec = strstr(pszCmdCopy, "msiexec");
	if (!pszMsiExec)
		pszMsiExec = strstr(pszCmdCopy, "MSIEXEC");
	if (!pszMsiExec)
		pszMsiExec = strstr(pszCmdCopy, "MsiExec");
	if (!pszMsiExec)
		pszMsiExec = strstr(pszCmdCopy, "MSIExec");
	if (pszMsiExec)
	{
		// Find location of installer from registry:
		char * szLoc = GetInstallerLocation();
		if (szLoc)
		{
			char * pszNew = new char [2 + strlen(pszCmdCopy) + strlen(szLoc)];
			char ch = *pszMsiExec;
			*pszMsiExec = 0;
			strcpy(pszNew, pszCmdCopy);
			strcat(pszNew, szLoc);
			__int64 len = strlen(pszNew);
			if (len > 1)
				if (pszNew[len - 1] != '\\')
					strcat(pszNew, "\\");
			*pszMsiExec = ch;
			strcat(pszNew, pszMsiExec);
			delete[] pszCmdCopy;
			pszCmdCopy = pszNew;
		}
		// If installer location cannot be found by this method, we will attempt to run the
		// command anyway.
	}

	// Set up data for creating new process:
	BOOL bReturnVal = false;
	STARTUPINFO si;
	DWORD dwExitCode =  0;
	PROCESS_INFORMATION process_info;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	g_Log.Write("Adjusted cmd to \"%s\"", pszCmdCopy);

	// Launch new process. The CREATE_SEPARATE_WOW_VDM should be ignored for 32-bit programs,
	// and also when running on Windows 98, but it is essential for 16-bit programs running on
	// Windows 2000 or later, else we cannot easily monitor when termination occurs:
	bReturnVal = CreateProcess(NULL, (LPTSTR)pszCmdCopy, NULL, NULL, false,
		CREATE_SEPARATE_WOW_VDM, NULL, NULL, &si, &process_info);

	// Retore original current directory:
	::SetCurrentDirectory(szOldPath);

	if (bReturnVal)
	{
		CloseHandle(process_info.hThread);

		HANDLE hMonitorThread = NULL;
		MonitorThreadData_t MonitorThreadData;
		bool fTerminateMonitor = false;

		// See if we need to do anything with the status window:
		if (pszStatusWindowControl)
		{
			if (stricmp(pszStatusWindowControl, "show") == 0)
			{
				// Show the window:
				ShowStatusDialog();
			}
			else if (stricmp(pszStatusWindowControl, "hide") == 0)
			{
				// Hide the window:
				HideStatusDialog();
			}
			if (strnicmp(pszStatusWindowControl, "monitor", 7) == 0)
			{
				// Start the window monitoring thread:
				MonitorThreadData.m_dwProcessId = process_info.dwProcessId;
				MonitorThreadData.m_pfTerminationFlag = &fTerminateMonitor;
				MonitorThreadData.nInitialDelayMs = atoi(&pszStatusWindowControl[7]);
				
				// MSDN says you can pass NULL instead of this, but you can't on Win98:
				DWORD nThreadId;
				hMonitorThread = CreateThread(NULL, 0, StatusDlgMonitorThreadEntry,
					LPVOID(&MonitorThreadData), 0, &nThreadId);
			}
		}

		if (fWaitTillExit)
		{
			if (pszDescription)
				strcpy(g_rgchActiveProcessDescription, pszDescription);
			else
			{
				strcpy(g_rgchActiveProcessDescription,
					FetchString(IDC_MESSAGE_GENERIC_INSTALLER));
			}
			if (fWaitTillExit)
			{
				// New code based on Microsoft support article 824042:
				// http://support.microsoft.com/kb/824042
				WaitForInputIdle(process_info.hProcess, INFINITE);

				WaitForSingleObject(process_info.hProcess, INFINITE);
			}

			g_rgchActiveProcessDescription[0] = 0;
			if (hMonitorThread)
			{
				// Get the Monitor thread to close down:
				fTerminateMonitor = true;
				WaitForSingleObject(hMonitorThread, INFINITE);
				CloseHandle(hMonitorThread);
				hMonitorThread = NULL;
			}
			// Get the exit code:
			GetExitCodeProcess(process_info.hProcess, &dwExitCode);

			// If the result of executing the process was that a reboot was triggered, we'll
			// inform the user and just wait:
			if (ERROR_SUCCESS_REBOOT_INITIATED == dwExitCode)
				PauseForReboot();
		}
		CloseHandle(process_info.hProcess);
	}
	else
	{
		return GetLastError();
	}

	delete[] pszCmdCopy;
	pszCmdCopy = NULL;
	return dwExitCode;
}

// Callback function used by StatusDlgMonitorThreadEntry() to see if any windows belong to a
// specified process.
int CALLBACK StatusEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// Process the lParam argument:
	if (!lParam)
		return 1;
	DWORD dwRequiredProcessId = *((DWORD *)lParam);

	// Get process ID from candidate window:
	DWORD dwProcessId;
	GetWindowThreadProcessId(hwnd, &dwProcessId);

	// See if we have found a window belonging to the specified process:
	if (dwProcessId == dwRequiredProcessId)
	{
		// We have, so hide the status window:
		HideStatusDialog();

		return 0; // No need to continue enumerating windows.
	}
	return 1; // Continue enumerating windows.
}

// Monitors a given process, and whenever that process displays a window, removes the status
// window.
DWORD WINAPI StatusDlgMonitorThreadEntry(LPVOID pData)
{
	// Process the pData argument:
	if (!pData)
		return 1;
	MonitorThreadData_t * pMonitorThreadData = (MonitorThreadData_t *)pData;
	DWORD dwRequiredProcessId = pMonitorThreadData->m_dwProcessId;
	bool * pfTerminate = pMonitorThreadData->m_pfTerminationFlag;

	Sleep(pMonitorThreadData->nInitialDelayMs);

	while(!(*pfTerminate)) // Loop until flag is set by thread owner.
	{
		if (EnumWindows(StatusEnumWindowsProc, (LPARAM)(&dwRequiredProcessId)) != 0)
		{
			// We didn't find a window beloning to the given process, so we'll make sure the
			// status window is visible:
			ShowStatusDialog();
		}
		// Sleep for quarter of a second before trying again:
		Sleep(250);
	}

	// Return status dialog, now that monitoring is over:
	ShowStatusDialog();

	return 0;
}


// Force a system reboot.
// Returns false if unable to do so.
// Returns true and shuts down asynchronously if successful.
bool Reboot()
{
	HANDLE hToken;              // handle to process token
	TOKEN_PRIVILEGES tkp;       // pointer to token structure

	g_Log.Write("Reboot initiated.");

	try
	{
		if (!g_fLessThanWin2k)
		{
			// Get the current process token handle so we can get shutdown privilege.
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
				&hToken))
			{
				throw "";
			}

			// Get the LUID for shutdown privilege.
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

			tkp.PrivilegeCount = 1;  // one privilege to set
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			// Get shutdown privilege for this process.
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

			// Cannot test the return value of AdjustTokenPrivileges.
			if (GetLastError() != ERROR_SUCCESS)
			{
				throw "";
			}
		}
		if (!ExitWindowsEx(EWX_REBOOT, 0))
		{
			g_Log.Write("Automatic reboot failed - asking user for manual reboot.");
			throw "";
		}
	}
	catch (...)
	{
		HandleError(kNonFatal, false, IDC_ERROR_REBOOT_FAILED);
	}

	PauseForReboot();
	// Should never get here:
	return false;
}


// Returns a dynamically allocated string containing the path of the directory where the Windows
// Installer .exe file is located.
// Returns NULL is there is an error (such as Windows Installer not installed).
// Caller must delete[] the returned string.
char * GetInstallerLocation()
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer", NULL, KEY_READ, &hKey);

	// we don't proceed unless the call above succeeds
	if (lResult != ERROR_SUCCESS)
		return NULL;

	// Get length of required buffer:
	DWORD dwBufLen = 0;
	lResult = RegQueryValueEx(hKey, "InstallerLocation", NULL, NULL, NULL, &dwBufLen);
	if (dwBufLen == 0)
		return NULL;

	char * szLocation = new char [dwBufLen];
	lResult = RegQueryValueEx(hKey, "InstallerLocation", NULL, NULL, (LPBYTE)szLocation,
			&dwBufLen);
	RegCloseKey(hKey);
	hKey = NULL;

	// If we receive an error, quit:
	if (lResult != ERROR_SUCCESS)
	{
		delete[] szLocation;
		return NULL;
	}

	return szLocation;
}

// Fetches string resource. Returns string if OK, empty string if error.
char * FetchString(int stid)
{
	const int kcchBuffer = 2000;
	static char sz[kcchBuffer];
	if (!LoadString(GetModuleHandle(NULL), stid, sz, kcchBuffer))
		return "";
	return sz;
}

// Acts the same way as sprintf(), but creates a buffer to hold the text that is at least
// as big as the minimum needed.
// Caller must delete[] the return value.
char * new_sprintf(const char * pszFormat, ...)
{
	// We will be passing on the variable arguments to the new_vsprintf() function:
	va_list arglist;
	va_start(arglist, pszFormat);

	char * pszResult = new_vsprintf(pszFormat, arglist);
	return pszResult;
}

// Acts the same way as new_sprintf() above, except the variable arguments have already been
// collected.
// Caller must delete[] the return value.
char * new_vsprintf(const char * pszFormat, const va_list arglist)
{
	int cchWksp = 100; // First guess at size needed.
	char * szWksp = new char [1 + cchWksp];

	// Format it with variable arguments, repeating until Wksp is big enough:
	int cch = _vsnprintf(szWksp, cchWksp, pszFormat, arglist);
	// If the reported number of characters written is the same as the size of our buffer, then
	// the terminating zero will have been missed off!
	while (cch == -1 || cch == cchWksp)
	{
		delete[] szWksp;
		cchWksp *= 2;
		szWksp = new char [1 + cchWksp];
		cch = _vsnprintf(szWksp, cchWksp, pszFormat, arglist);
	}
	return szWksp;
}

// Write nIndent spaces at the start of the returned string, then treat the rest of the
// arguments as in a new_sprintf() call.
// Caller must delete[] the return value;
char * new_ind_sprintf(int nIndent, const char * pszFormat, ...)
{
	char * pszWksp = NULL;

	for (int n = 0; n < nIndent; n++)
		new_sprintf_concat(pszWksp, 0, " ");

	// We will be passing on the variable arguments to the new_vsprintf_concat() function:
	va_list arglist;
	va_start(arglist, pszFormat);

	new_vsprintf_concat(pszWksp, 0, pszFormat, arglist);

	return pszWksp;
}

// Acts the same as new_sprintf() above, only it appends the formatted string to rpszMain,
// having inserted ctInsertNewline newlines.
void new_sprintf_concat(char *& rpszMain, int ctInsertNewline, const char * pszAddendumFmt, ...)
{
	// We will be passing on the variable arguments to the new_vsprintf_concat() function:
	va_list arglist;
	va_start(arglist, pszAddendumFmt);

	new_vsprintf_concat(rpszMain, ctInsertNewline, pszAddendumFmt, arglist);
}

// Acts the same as new_vsprintf() above, only it appends the formatted string to rpszMain,
// having inserted ctInsertNewline newlines.
void new_vsprintf_concat(char *& rpszMain, int ctInsertNewline, const char * pszAddendumFmt,
						 const va_list arglist)
{
	char * pszWksp;

	for (int n = 0; n < ctInsertNewline; n++)
	{
		pszWksp = new_sprintf("%s\r\n", rpszMain ? rpszMain : "");
		delete[] rpszMain;
		rpszMain = pszWksp;
	}
	pszWksp = new_vsprintf(pszAddendumFmt, arglist);
	char * pszWksp2 = new_sprintf("%s%s", rpszMain ? rpszMain : "", pszWksp);
	delete[] pszWksp;
	pszWksp = NULL;

	delete[] rpszMain;
	rpszMain = pszWksp2;
}

// Parses a (potentially) 4-part version number string (e.g. 2.0.2600.1106) and returns a 64-bit
// integer comprising all 4 parts, so a simple comparison can be made (e.g. 0x000200000A280452).
__int64 GetHugeVersion(const char * pszVersion)
{
	if (!pszVersion)
		return 0;

	char * pszVersionCopy = strdup(pszVersion);
	__int64 nValue = 0;

	char * pszNextSegment = strtok(pszVersionCopy, ".");
	int ctSegments = 1;
	while (pszNextSegment && ctSegments <= 4)
	{
		__int64 nSegment = atoi(pszNextSegment);
		nValue |= (nSegment << ((4 - ctSegments) * 16));
		ctSegments++;
		pszNextSegment = strtok(NULL, ".");
	}
	delete[] pszVersionCopy;
	pszVersionCopy = NULL;

	return nValue;
}

// Splits a 64-bit version number into the 4 constituent parts as a text string.
// Caller must delete[] the return value.
char * GenVersionText(__int64 nHugeVersion)
{
	return new_sprintf("%d.%d.%d.%d", int(nHugeVersion >> 48), 
		int((nHugeVersion >> 32) & 0xFFFF), int((nHugeVersion >> 16) & 0xFFFF),
		int(nHugeVersion & 0xFFFF));
}


// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
bool VersionInRange(__int64 nVersion, const char * pszMinVersion, 
					const char * pszMaxVersion)
{
	// Create 64-bit numbers from the min and max arguments:
	__int64 nMinVersion;
	__int64 nMaxVersion;

	if (pszMinVersion)
		nMinVersion = GetHugeVersion(pszMinVersion);
	else
		nMinVersion = 0;

	if (pszMaxVersion)
		nMaxVersion = GetHugeVersion(pszMaxVersion);
	else
		nMaxVersion = 0x7FFFFFFFFFFFFFFF;

	// See if the discovered version is within range:
	return (nVersion >= nMinVersion && nVersion <= nMaxVersion);
}

// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
bool VersionInRange(const char * pszVersion, const char * pszMinVersion, 
					const char * pszMaxVersion)
{
	if (!pszVersion)
		return false;

	// Create 64-bit number from the pszVersion argument:
	__int64 nVersion = GetHugeVersion(pszVersion);

	return VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
}

bool WriteClipboardText(const char * pszText)
{
	int nLen = (int)strlen(pszText);
	int nRet;

	// Open clipboard for our use:
	if (!OpenClipboard(NULL))
		return false;
	nRet = GetLastError();
	EmptyClipboard();
	nRet = GetLastError();

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, nLen);
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return false;
	}

	// Lock the handle and copy the text to the buffer.
	LPVOID lptstrCopy = GlobalLock(hglbCopy);
	memcpy(lptstrCopy, pszText, nLen);
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard.
	SetClipboardData(CF_TEXT, hglbCopy);
	nRet = GetLastError();
	CloseClipboard();
	nRet = GetLastError();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////
/*
The following section checks the system for top level windows that are not responding to
Windows messages.
*/

#include <shellapi.h>
#include <tlhelp32.h>


// Structure to hold data on each hanging top level window and its thread:
struct WindowInfo_t
{
	HWND hwnd;
	HANDLE hCallbackThread;
	DWORD dwProcessId;
};

// Class to handle array of hanging window data:
class WindowsInfo_t
{
public:
	WindowsInfo_t();
	~WindowsInfo_t();

	bool AlreadyLoggedProcess(DWORD dwProcessId);
	void Add(HWND hwnd, HANDLE hThread, DWORD dwProcessId);
	int GetNumEntries(void) { return cwin; }
	const WindowInfo_t * operator [] (int index);

protected:
	WindowInfo_t ** WindowsInfo;
	int cwin;
};

// Constructor.
WindowsInfo_t::WindowsInfo_t()
{
	WindowsInfo = NULL;
	cwin = 0;
}

// Destructor.
WindowsInfo_t::~WindowsInfo_t()
{
	// Delete dynamic array:
	for (int i = 0; i < cwin; i++)
		delete WindowsInfo[i];
	delete[] WindowsInfo;
}

// See if we've recorded a given process as hanging.
bool WindowsInfo_t::AlreadyLoggedProcess(DWORD dwProcessId)
{
	for (int i = 0; i < cwin; i++)
		if (WindowsInfo[i]->dwProcessId == dwProcessId)
			return true;

	return false;
}

// Add new hanging window data to array.
void WindowsInfo_t::Add(HWND hwnd, HANDLE hThread, DWORD dwProcessId)
{
	// First check that we haven't logged the given process before:
	if (AlreadyLoggedProcess(dwProcessId))
		return;

	// Create new data object for array:
	WindowInfo_t * info = new WindowInfo_t;
	info->hwnd = hwnd;
	info->hCallbackThread = hThread;
	info->dwProcessId = dwProcessId;

	// Add new item to array:
	WindowInfo_t ** temp = new WindowInfo_t * [1 + cwin];
	for (int i = 0; i < cwin; i++)
		temp[i] = WindowsInfo[i];
	delete[] WindowsInfo;
	WindowsInfo = temp;
	WindowsInfo[cwin++] = info;
}

// Get data for given array index.
const WindowInfo_t * WindowsInfo_t::operator [] (int index)
{
	if (index < 0 || index >= cwin)
		throw; // TODO
	
	return WindowsInfo[index];
}

// Thread entry function, to deal with each top level window:
DWORD WINAPI TestHangingThreadEntry(LPVOID Data)
{
	// Convert generic parameter:
	HWND hwnd = (HWND)Data;

	// Send a harmless message to the window:
	SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);

	// Now (assuming we got this far) quit this thread:
	return 0;
}


// Callback function, which will be called for each top-level window.
BOOL CALLBACK HangingWindowsEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// Convert generic parameter:
	WindowsInfo_t * WindowsInfo = (WindowsInfo_t *)lParam;

	// Get process ID of given window:
	DWORD dwWinProc;
	GetWindowThreadProcessId(hwnd, &dwWinProc);

	// Check if we've already found a window that hangs in the same process:
	if (WindowsInfo->AlreadyLoggedProcess(dwWinProc))
		return true; // Don't bother testing this window.

	// Create a new thread to handle each window asynchronously:
	DWORD nThreadId; // MSDN says you can pass NULL instead of this, but you can't on Win98.
	HANDLE hThread = CreateThread(NULL, 0, TestHangingThreadEntry, (LPVOID)hwnd, 0, &nThreadId);

	// See if the new thread hangs:
	if (WaitForSingleObject(hThread, 5000) == WAIT_TIMEOUT)
	{
		// Record the details of this window, its process and the new thread:
		WindowsInfo->Add(hwnd, hThread, dwWinProc);
	}
	else
		CloseHandle(hThread);

	return true;
}

// Returns a string containing details of windows that do not respond to Windows messages.
char * GenerateHangingWindowsReport()
{
	char * pszReport = NULL;
	WindowsInfo_t HangingWindowsInfo;

	// Call our EnumWindowsProc for each top level window:
	BOOL fEnumResult = EnumWindows(HangingWindowsEnumWindowsProc, LPARAM(&HangingWindowsInfo));

	// Compile report on all the hanging windows:
	int cwin = HangingWindowsInfo.GetNumEntries();
	for (int i = 0; i < cwin; i++)
	{
		// Get current data:
		const WindowInfo_t * WindowInfo = HangingWindowsInfo[i];
		HANDLE hThread = WindowInfo->hCallbackThread;

		// Check if current thread has returned:
		DWORD dwExitCode;
		GetExitCodeThread(hThread, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
		{
			new_sprintf_concat(pszReport, 1, "%d) ", i + 1);

			HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
			MODULEENTRY32 me32;

			// Take a snapshot of all modules in the specified process.
			hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, WindowInfo->dwProcessId);
			if (hModuleSnap != INVALID_HANDLE_VALUE)
			{
				// Set the size of the structure before using it.
				me32.dwSize = sizeof( MODULEENTRY32 );

				// Retrieve information about the first module:
				if (Module32First(hModuleSnap, &me32))
				{
					// Get "official" name of product:
					char * pszProductName = NULL;
					char * pszCompanyName = NULL;
					DWORD dwDummy;
					DWORD dwLen = GetFileVersionInfoSize(me32.szExePath, &dwDummy);
					if (0 != dwLen)
					{
						BYTE * Block = new BYTE [dwLen];
						if (0 != GetFileVersionInfo(me32.szExePath, NULL, dwLen, Block))
						{
							// Get ProductName and CompanyName in first available language:
							struct LANGANDCODEPAGE
							{
								WORD wLanguage;
								WORD wCodePage;
							} *lpTranslate;
							UINT cbTranslate;

							// Read the list of languages and code pages:
							VerQueryValue(Block, TEXT("\\VarFileInfo\\Translation"),
								(LPVOID*)&lpTranslate, &cbTranslate);

							// Read the file description for first language and code page.
							for (unsigned int i = 0; 
								i < min(1, (cbTranslate/sizeof(struct LANGANDCODEPAGE))); i++)
							{
								char * pszSubBlock = new_sprintf(
									"\\StringFileInfo\\%04x%04x\\ProductName",
									lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
								char * pszOutput;
								UINT uDummy;

								// Retrieve file description for language and code page "i". 
								if (0 != VerQueryValue(Block, pszSubBlock, (void ** )&pszOutput,
									&uDummy))
								{
									pszProductName = strdup(pszOutput);
								}
								delete[] pszSubBlock;

								pszSubBlock = new_sprintf(
									"\\StringFileInfo\\%04x%04x\\CompanyName",
									lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

								// Retrieve file description for language and code page "i". 
								if (0 != VerQueryValue(Block, pszSubBlock, (void ** )&pszOutput,
									&uDummy))
								{
									pszCompanyName = strdup(pszOutput);
								}
								delete[] pszSubBlock;
							} // Next Language Code-page
						} // End if GetFileVersionInfo() succeeded
						
						delete[] Block;

					} // End if GetFileVersionInfoSize() succeeded.

					if (pszProductName)
					{
						new_sprintf_concat(pszReport, 0, " %s", pszProductName);
						if (pszCompanyName)
							new_sprintf_concat(pszReport, 0, " by %s", pszCompanyName);
					}
					else
						new_sprintf_concat(pszReport, 0, " %s", me32.szModule);

					const char * pszNoWindowName = "[Unnamed Window]";
					int cchWndText = GetWindowTextLength(WindowInfo->hwnd);
					char * pszWndText;
					if (cchWndText)
					{
						pszWndText = new char [1 + cchWndText];
						GetWindowText(WindowInfo->hwnd, pszWndText, 1 + cchWndText);
					}
					else
						pszWndText = strdup(pszNoWindowName);

					new_sprintf_concat(pszReport, 1, "        Window title: %s", pszWndText);

					delete[] pszWndText;
					pszWndText = NULL;

					new_sprintf_concat(pszReport, 1, "        File path: %s", me32.szExePath);

					delete[] pszProductName;
					delete[] pszCompanyName;
				}
				// Don't forget to clean up the snapshot object.
				CloseHandle(hModuleSnap);

				// Kill hanging thread:
				TerminateThread(hThread, 0);
				CloseHandle(hThread);
			}
		}
	}
	return pszReport;
}

/*
End of section dealing with hanging windows
*/
///////////////////////////////////////////////////////////////////////////////////////////////