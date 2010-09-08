/*
	Copyright (c) 2003 SIL International.

	Master installer auxilary functions file.
*/

#define _WIN32_WINNT 0x0501

#include <Windows.h>
#include <Sddl.h>
#include <stdio.h>
#include <shlobj.h>
#include <tchar.h>

#include "UsefulStuff.h"
#include "Globals.h"
#include "LogFile.h"
#include "Dialogs.h"
#include "resource.h"
#include "ErrorHandler.h"


const int cchActiveProcessDescription = 100;
_TCHAR g_rgchActiveProcessDescription[cchActiveProcessDescription] = _T("");

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
// The Windows current path used in the child process is set to the pszCurrentDir value, or
// the location of the main .exe file if pszCurrentDir == "".
// If pszDescription is set, its value is copied to a global variable which can be used in a
// status window when attempting to quit the created process.
// If pszStatusWindowControl is set, it contains a string describing what should happen to the
// status window: show it, hide it or remove it if the new process has any windows visible.
// If pszEnvironment is set, the defined environment will be passed on to CreateProcess.
DWORD ExecCmd(LPCTSTR pszCmd, const _TCHAR * pszCurrentDir, bool fWaitTillExit,
			  const _TCHAR * pszDescription, const _TCHAR * pszStatusWindowControl,
			  const _TCHAR * pszEnvironment, STARTUPINFO * pStartupInfo)
{
	_TCHAR * pszNewCurrentDir = NULL;

	if (pszCurrentDir != NULL)
	{
		if (_tcscmp(pszCurrentDir, _T("")) == 0)
		{
			// Set pszNewCurrentDir to the path where current process was launched from:
			pszNewCurrentDir = NewGetExeFolder();
			g_Log.Write(_T("Setting current directory as per setup.exe: %s"), pszNewCurrentDir);
		}
		else
		{
			pszNewCurrentDir = my_strdup(pszCurrentDir);
			g_Log.Write(_T("Setting current directory to %s"), pszNewCurrentDir);
		}
	}
		
	// Set up data for creating new process:
	BOOL bReturnVal = false;
	DWORD dwExitCode =  0;
	PROCESS_INFORMATION process_info;

	STARTUPINFO siBlank;
	ZeroMemory(&siBlank, sizeof(siBlank));
	siBlank.cb = sizeof(siBlank);
	STARTUPINFO * psiInUse = pStartupInfo ? pStartupInfo : &siBlank;

	// Launch new process. The CREATE_SEPARATE_WOW_VDM should be ignored for 32-bit programs,
	// and also when running on Windows 98, but it is essential for 16-bit programs running on
	// Windows 2000 or later, else we cannot easily monitor when termination occurs:
	bReturnVal = CreateProcess(NULL, (LPTSTR)pszCmd, NULL, NULL, false,
		CREATE_SEPARATE_WOW_VDM, (LPVOID)pszEnvironment, pszNewCurrentDir, psiInUse,
		&process_info);

	delete[] pszNewCurrentDir;
	pszNewCurrentDir = NULL;

	if (bReturnVal)
	{
		CloseHandle(process_info.hThread);

		HANDLE hMonitorThread = NULL;
		MonitorThreadData_t MonitorThreadData;
		bool fTerminateMonitor = false;

		// See if we need to do anything with the status window:
		if (pszStatusWindowControl)
		{
			if (_tcsicmp(pszStatusWindowControl, _T("show")) == 0)
			{
				// Show the window:
				ShowStatusDialog();
			}
			else if (_tcsicmp(pszStatusWindowControl, _T("hide")) == 0)
			{
				// Hide the window:
				HideStatusDialog();
			}
			if (_tcsnicmp(pszStatusWindowControl, _T("monitor"), 7) == 0)
			{
				// Start the window monitoring thread:
				MonitorThreadData.m_dwProcessId = process_info.dwProcessId;
				MonitorThreadData.m_pfTerminationFlag = &fTerminateMonitor;
				MonitorThreadData.nInitialDelayMs = _tstoi(&pszStatusWindowControl[7]);
				
				// MSDN says you can pass NULL instead of this, but you can't on Win98:
				DWORD nThreadId;
				hMonitorThread = CreateThread(NULL, 0, StatusDlgMonitorThreadEntry,
					LPVOID(&MonitorThreadData), 0, &nThreadId);
			}
		}

		if (fWaitTillExit)
		{
			if (pszDescription)
			{
				_tcscpy_s(g_rgchActiveProcessDescription, cchActiveProcessDescription,
					pszDescription);
			}
			else
			{
				_tcscpy_s(g_rgchActiveProcessDescription, cchActiveProcessDescription,
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

	g_Log.Write(_T("Exit code = %d"), dwExitCode);

	return dwExitCode;
}

// Sets a new PATH environment variable made up of the pre-existing one plus the
// specified path. This only affects our own process.
void AddToPathEnvVar(_TCHAR * pszExtraPath)
{
	// Fetch the current PATH environment variable:
	g_Log.Write(_T("Adding %s to PATH environment variable: "), pszExtraPath);
	DWORD dwRet, dwErr;
	DWORD cchPath = 500;
	_TCHAR * pszPath = new _TCHAR [cchPath];
	pszPath[0] = 0;
	const _TCHAR * kpszPathName = _T("PATH");

	dwRet = GetEnvironmentVariable(kpszPathName, pszPath, cchPath);

	if (0 == dwRet)
	{
		dwErr = GetLastError();
		if (ERROR_ENVVAR_NOT_FOUND == dwErr)
			g_Log.Write(_T("Environment variable PATH does not yet exist."));
	}
	else if (cchPath < dwRet)
	{
		 // We didn't allocate a big enough buffer, so try again:
		delete[] pszPath;
		cchPath = dwRet;
		pszPath = new _TCHAR [cchPath];
		pszPath[0] = 0;
		g_Log.Write(_T("Reallocating PATH buffer with %d."), cchPath);

		dwRet = GetEnvironmentVariable(kpszPathName, pszPath, cchPath);
		if (!dwRet)
			g_Log.Write(_T("GetEnvironmentVariable failed (%d)."), GetLastError());
	}
	g_Log.Write(_T("Retrieved PATH = %s."), pszPath);

	// Add the pszExtraPath to the existing PATH:
	_TCHAR * pszNewPath = new_sprintf(_T("%s;%s"), pszPath, pszExtraPath);
	delete[] pszPath;
	pszPath = NULL;
	g_Log.Write(_T("Attempting to set PATH to %s."), pszNewPath);

	if (!SetEnvironmentVariable(kpszPathName, pszNewPath)) 
		g_Log.Write(_T("SetEnvironmentVariable failed (%d)."), GetLastError());

	delete[] pszNewPath;
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

	g_Log.Write(_T("Reboot initiated."));

	try
	{
		if (g_OSVersion >= OSVersion_t::Win2k)
		{
			// Get the current process token handle so we can get shutdown privilege.
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
				&hToken))
			{
				throw _T("");
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
				throw _T("");
			}
		}
		if (!ExitWindowsEx(EWX_REBOOT, 0))
		{
			g_Log.Write(_T("Automatic reboot failed - asking user for manual reboot."));
			throw _T("");
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

// Reboot, allowing the user time to react.
void FriendlyReboot()
{
	HideStatusDialog();

	if (!g_fSilent)
	{
		if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_REBOOT_COUNTDOWN), NULL,
			DlgProcRebootCountdown) == 0)
		{
			g_Log.Write(_T("User chose to quit rather than reboot."));
			throw UserQuitException;
		}
	}
	g_Log.Write(_T("Rebooting."));
	Reboot();
}

bool TestResultForRebootRequest(DWORD dwResult)
{
	if (dwResult == ERROR_SUCCESS_REBOOT_REQUIRED
		|| dwResult == ERROR_SUCCESS_RESTART_REQUIRED
		|| dwResult == 0x80070BC2)
	{
		return true;
	}
	return false;
}

// Fetches string resource. Returns string if OK, empty string if error.
_TCHAR * FetchString(int stid)
{
	const int kcchBuffer = 2000;
	static _TCHAR sz[kcchBuffer];
	if (!LoadString(GetModuleHandle(NULL), stid, sz, kcchBuffer))
		return _T("");
	return sz;
}


#include "StringFunctions.cpp"
#include "VersionFunctions.cpp"

// Returns a new dynamically allocated string contaning the folder where our .exe is located.
_TCHAR * NewGetExeFolder()
{
	int cchExeDir = MAX_PATH;
	_TCHAR * pszExeDir = NULL;

	// Try to get full path to our .exe, doubling buffer size each time we don't have enough:
	while (!pszExeDir)
	{
		pszExeDir = new _TCHAR [cchExeDir];
		if (0 == GetModuleFileName(NULL, pszExeDir, MAX_PATH))
			HandleError(kFatal, true, IDC_ERROR_INTERNAL);

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			delete[] pszExeDir;
			pszExeDir = NULL;
			cchExeDir *= 2;
		}
	}

	// Find last backslash, and remove it and any text after it:
	RemoveLastPathSection(pszExeDir);
	return pszExeDir;
}

// Writes the given text to the Clipboard
bool WriteClipboardText(const _TCHAR * pszText)
{
	int cbyte = (1 + (int)_tcslen(pszText)) * sizeof(_TCHAR);
	int nRet;

	// Open clipboard for our use. Note this requires us to show the status dialog if it is not
	// already shown, so we can pass a window handle to the API function:
	bool fCloseStatusDialog;
	BOOL fClipBoardOpened = my_OpenClipboard(&fCloseStatusDialog);
	if (!fClipBoardOpened)
		return false;
	nRet = GetLastError();
	EmptyClipboard();
	nRet = GetLastError();

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, cbyte);
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		if (fCloseStatusDialog)
			HideStatusDialog();
		return false;
	}

	// Lock the handle and copy the text to the buffer.
	LPVOID lptstrCopy = GlobalLock(hglbCopy);
	memcpy(lptstrCopy, pszText, cbyte);
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard.
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
	SetClipboardData(CF_TEXT, hglbCopy);
#endif
	nRet = GetLastError();
	CloseClipboard();
	nRet = GetLastError();

	if (fCloseStatusDialog)
		HideStatusDialog();

	return true;
}

// Returns the Domain\Acoount name string formed from the Well Known SID whose index
// is specified.
// see ms-help://MS.VSCC.v80/MS.MSDN.vAug06.en/secauthz/security/well_known_sid_type.htm
// for valid index list.
_TCHAR * CreateAccountNameFromWellKnownSidIndex(int SidIndex)
{
	_TCHAR * ReturnVal = NULL;

	if (_CreateWellKnownSid && _LookupAccountSid)
	{
		DWORD SidSize = SECURITY_MAX_SID_SIZE;
		PSID TheSID;

		// Allocate enough memory for the largest possible SID.
		if(!(TheSID = LocalAlloc(LMEM_FIXED, SidSize)))
			return NULL;

		// Create a SID for the given index on the local computer.
		if (!_CreateWellKnownSid(WELL_KNOWN_SID_TYPE(SidIndex), NULL, TheSID, &SidSize))
			return NULL;

		_TCHAR * pszName = NULL;
		DWORD cchName = 0;
		_TCHAR * pszDomain = NULL;
		DWORD cchDomain = 0;
		SID_NAME_USE Use;
		_LookupAccountSid(NULL, TheSID, pszName, &cchName, pszDomain, &cchDomain, &Use);
		pszName = new _TCHAR [cchName];
		pszDomain = new _TCHAR [cchDomain];
		_LookupAccountSid(NULL, TheSID, pszName, &cchName, pszDomain, &cchDomain, &Use);

		// When done, free the memory used.
		LocalFree(TheSID);

		ReturnVal = new_sprintf(_T("%s\\%s"), pszDomain, pszName);
		delete[] pszName;
		delete[] pszDomain;
	}

	return ReturnVal;
}

// Instantiate pointers to dynamic functions in AdvApi32.dll:
static 	HMODULE hmodAdvapi32 = NULL; // Pointer to the DLL
CheckTokenMembershipFn _CheckTokenMembership;
ConvertStringSidToSidFn _ConvertStringSidToSid;
CreateWellKnownSidFn _CreateWellKnownSid;
GetNamedSecurityInfoFn _GetNamedSecurityInfo;
LookupAccountSidFn _LookupAccountSid;
QueryServiceStatusExFn _QueryServiceStatusEx;
SetEntriesInAclFn _SetEntriesInAcl;
SetNamedSecurityInfoFn _SetNamedSecurityInfo;

// Loads the AdvApi32.dll and initializes pointers to functions within it that don't exist
// on Windows 98 or earlier. This is so that the master installer can still run on Windows 98
// (although applications that need a later OS will be grayed out).
// Pointers to functions that don't exist will be set to NULL.
void InitAdvancedApi()
{
	// Begin by initializing all the dynamic function pointers to NULL:
	g_Log.Write(_T("Initializing dynamic function pointers..."));
	g_Log.Indent();
	_CheckTokenMembership = NULL;
	_ConvertStringSidToSid = NULL;
	_CreateWellKnownSid = NULL;
	_GetNamedSecurityInfo = NULL;
	_LookupAccountSid = NULL;
	_QueryServiceStatusEx = NULL;
	_SetEntriesInAcl = NULL;
	_SetNamedSecurityInfo = NULL;

	// Get Windows system folder path:
	_TCHAR * pszSystemFolder = GetFolderPathNew(CSIDL_SYSTEM);
	if (pszSystemFolder)
	{
		g_Log.Write(_T("System folder is %s"), pszSystemFolder);

		// Remove any terminating backslash:
		RemoveTrailingBackslashes(pszSystemFolder);

		// Generate full path of Advapi32.dll:
		_TCHAR * pszAdvapi32Dll = MakePath(pszSystemFolder, _T("Advapi32.dll"));

		// Release memory used to hold path to System32 folder:
		delete[] pszSystemFolder;
		pszSystemFolder = NULL;

		// Get a handle to the DLL:
		hmodAdvapi32 = LoadLibrary(pszAdvapi32Dll);

		// If we were successful, find addresses of functions we need:
		if (hmodAdvapi32)
		{
			g_Log.Write(_T("Loaded %s"), pszAdvapi32Dll);

			_CheckTokenMembership = (CheckTokenMembershipFn)GetProcAddress(hmodAdvapi32,
				"CheckTokenMembership");
			g_Log.Write(_T("Address of CheckTokenMembership: %s"), (_CheckTokenMembership? _T("found") : _T("NULL")));

			_CreateWellKnownSid = (CreateWellKnownSidFn)GetProcAddress(hmodAdvapi32,
				"CreateWellKnownSid");
			g_Log.Write(_T("Address of CreateWellKnownSid: %s"), (_CreateWellKnownSid?  _T("found") : _T("NULL")));

			_QueryServiceStatusEx = (QueryServiceStatusExFn)GetProcAddress(hmodAdvapi32,
				"QueryServiceStatusEx");
			g_Log.Write(_T("Address of QueryServiceStatusEx: %s"), (_QueryServiceStatusEx?  _T("found") : _T("NULL")));


			// The following functions have different versions for ANSI and Unicode:
#ifdef UNICODE
			_ConvertStringSidToSid = (ConvertStringSidToSidFn)GetProcAddress(hmodAdvapi32,
				"ConvertStringSidToSidW");
			g_Log.Write(_T("Address of ConvertStringSidToSidW: %s"), (_ConvertStringSidToSid?  _T("found") : _T("NULL")));

			_GetNamedSecurityInfo = (GetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
				"GetNamedSecurityInfoW");
			g_Log.Write(_T("Address of GetNamedSecurityInfoW: %s"), (_GetNamedSecurityInfo?  _T("found") : _T("NULL")));

			_LookupAccountSid = (LookupAccountSidFn)GetProcAddress(hmodAdvapi32,
				"LookupAccountSidW");
			g_Log.Write(_T("Address of LookupAccountSidW: %s"), (_LookupAccountSid?  _T("found") : _T("NULL")));

			_SetEntriesInAcl = (SetEntriesInAclFn)GetProcAddress(hmodAdvapi32,
				"SetEntriesInAclW");
			g_Log.Write(_T("Address of SetEntriesInAclW: %s"), (_SetEntriesInAcl?  _T("found") : _T("NULL")));

			_SetNamedSecurityInfo = (SetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
				"SetNamedSecurityInfoW");
			g_Log.Write(_T("Address of SetNamedSecurityInfoW: %s"), (_SetNamedSecurityInfo?  _T("found") : _T("NULL")));

#else
			_ConvertStringSidToSid = (ConvertStringSidToSidFn)GetProcAddress(hmodAdvapi32,
				"ConvertStringSidToSidA");
			g_Log.Write(_T("Address of ConvertStringSidToSidA: %s"), (_ConvertStringSidToSid?  _T("found") : _T("NULL")));

			_GetNamedSecurityInfo = (GetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
				"GetNamedSecurityInfoA");
			g_Log.Write(_T("Address of GetNamedSecurityInfoA: %s"), (_GetNamedSecurityInfo?  _T("found") : _T("NULL")));

			_LookupAccountSid = (LookupAccountSidFn)GetProcAddress(hmodAdvapi32,
				"LookupAccountSidA");
			g_Log.Write(_T("Address of LookupAccountSidA: %s"), (_LookupAccountSid?  _T("found") : _T("NULL")));

			_SetEntriesInAcl = (SetEntriesInAclFn)GetProcAddress(hmodAdvapi32,
				"SetEntriesInAclA");
			g_Log.Write(_T("Address of CheckTokenMembership: %s"), (_SetEntriesInAcl?  _T("found") : _T("NULL")));

			_SetNamedSecurityInfo = (SetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
				"SetNamedSecurityInfoA");
			g_Log.Write(_T("Address of SetNamedSecurityInfoA: %s"), (_SetNamedSecurityInfo?  _T("found") : _T("NULL")));

#endif
		}
		else
			g_Log.Write(_T("Could not load %s"), pszAdvapi32Dll);

		// Release memory used to hold path to Advapi32.dll:
		delete[] pszAdvapi32Dll;
		pszAdvapi32Dll = NULL;
	}
	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

// Releases any resources allocated in InitAdvancedApi().
void DropAdvancedApi()
{
	if (hmodAdvapi32)
	{
		FreeLibrary(hmodAdvapi32);
		hmodAdvapi32 = NULL;
	}

	_CheckTokenMembership = NULL;
	_CreateWellKnownSid = NULL;
	_GetNamedSecurityInfo = NULL;
	_LookupAccountSid = NULL;
	_QueryServiceStatusEx = NULL;
	_SetEntriesInAcl = NULL;
	_SetNamedSecurityInfo = NULL;
}

/*
// Tests to see if the specified folder is compressed.
bool IsFolderCompressed(const _TCHAR * pszFolderPath)
{
	g_Log.Write("Testing if folder '%s' is compressed...", pszFolderPath);
	g_Log.Indent();

	bool fResult = false;

	DWORD dwAttr = GetFileAttributes(pszFolderPath);
	if (dwAttr == 0xffffffff)
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_FILE_NOT_FOUND)
		{
			// file not found
			g_Log.Write("File not found!");
			return false;
		}
		else if (dwError == ERROR_PATH_NOT_FOUND)
		{
			// path not found
			g_Log.Write("Path not found!");
			return false;
		}
		else if (dwError == ERROR_ACCESS_DENIED)
		{
			// file or directory exists, but access is denied
			g_Log.Write("Access is denied.");
			return false;
		}
		else
		{
			// some other error has occured
			g_Log.Write("Error %d - could not get attributes.", dwError);
			return false;
		}
	}
	else
	{
		if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			// this is a directory
			if (dwAttr & FILE_ATTRIBUTE_COMPRESSED)
			{
				// Directory is compressed
				fResult = true;
			}
		}
	}

	g_Log.Unindent();
	g_Log.Write("...Done. Folder '%s' is %scompressed.", pszFolderPath, fResult? "" : "not ");

	return fResult;
}

bool IsFolderCompressed(int csidlFolder)
{
	g_Log.Write("Testing if folder with CSIDL %d is compressed...", csidlFolder);
	g_Log.Indent();

	_TCHAR * pszAppDataFolder = GetFolderPathNew(csidlFolder);
	bool fResult = IsFolderCompressed(pszAppDataFolder);
	delete[] pszAppDataFolder;
	pszAppDataFolder = NULL;

	g_Log.Unindent();
	g_Log.Write("...Done. Folder with CSIDL %d is %scompressed.", csidlFolder, fResult? "" : "not ");

	return fResult;
}
*/

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
_TCHAR * GenerateHangingWindowsReport()
{
	_TCHAR * pszReport = NULL;
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
			new_sprintf_concat(pszReport, 1, _T("%d) "), i + 1);

			HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
			MODULEENTRY32 me32;

			// Take a snapshot of all modules in the specified process.
			hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, WindowInfo->dwProcessId);
			if (hModuleSnap != INVALID_HANDLE_VALUE)
			{
				// Set the size of the structure before using it.
				me32.dwSize = sizeof( MODULEENTRY32 );

				// Retrieve information about the first module:
				// (When compiled in Unicode mode, this function causes the whole program to fail
				// on Win98, as there is no Module32FirstW in Kernel32.dll on Win98)
				if (Module32First(hModuleSnap, &me32))
				{
					// Get "official" name of product:
					_TCHAR * pszProductName = NULL;
					_TCHAR * pszCompanyName = NULL;
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
							VerQueryValue(Block, _T("\\VarFileInfo\\Translation"),
								(LPVOID*)&lpTranslate, &cbTranslate);

							// Read the file description for first language and code page.
							for (unsigned int i = 0; 
								i < min(1, (cbTranslate/sizeof(struct LANGANDCODEPAGE))); i++)
							{
								_TCHAR * pszSubBlock = new_sprintf(
									_T("\\StringFileInfo\\%04x%04x\\ProductName"),
									lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
								_TCHAR * pszOutput;
								UINT uDummy;

								// Retrieve file description for language and code page "i". 
								if (0 != VerQueryValue(Block, pszSubBlock, (void ** )&pszOutput,
									&uDummy))
								{
									pszProductName = my_strdup(pszOutput);
								}
								delete[] pszSubBlock;

								pszSubBlock = new_sprintf(
									_T("\\StringFileInfo\\%04x%04x\\CompanyName"),
									lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

								// Retrieve file description for language and code page "i". 
								if (0 != VerQueryValue(Block, pszSubBlock, (void ** )&pszOutput,
									&uDummy))
								{
									pszCompanyName = my_strdup(pszOutput);
								}
								delete[] pszSubBlock;
							} // Next Language Code-page
						} // End if GetFileVersionInfo() succeeded
						
						delete[] Block;

					} // End if GetFileVersionInfoSize() succeeded.

					if (pszProductName)
					{
						new_sprintf_concat(pszReport, 0, _T(" %s"), pszProductName);
						if (pszCompanyName)
							new_sprintf_concat(pszReport, 0, _T(" by %s"), pszCompanyName);
					}
					else
						new_sprintf_concat(pszReport, 0, _T(" %s"), me32.szModule);

					const _TCHAR * pszNoWindowName = _T("[Unnamed Window]");
					int cchWndText = GetWindowTextLength(WindowInfo->hwnd);
					_TCHAR * pszWndText;
					if (cchWndText)
					{
						pszWndText = new _TCHAR [1 + cchWndText];
						GetWindowText(WindowInfo->hwnd, pszWndText, 1 + cchWndText);
					}
					else
						pszWndText = my_strdup(pszNoWindowName);

					new_sprintf_concat(pszReport, 1, _T("        Window title: %s"), pszWndText);

					delete[] pszWndText;
					pszWndText = NULL;

					new_sprintf_concat(pszReport, 1, _T("        File path: %s"), me32.szExePath);

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