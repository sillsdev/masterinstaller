#include <windows.h>
#include <tchar.h>

#include "Globals.h"

// Global instantiations:
bool g_fLessThanWin2k = false;
DWORD g_langidWindowsLanguage = 0;
bool g_fAdministrator = false;
bool g_fStopRequested = false;
bool g_fRebootPending = false;
bool g_fManualInstall = false;
UserQuitException_t UserQuitException;

#include "ConfigGeneral.cpp"

#include "Resource.h"
#include "ErrorHandler.h"

HANDLE hStopRequestInProgress = NULL;

void CheckIfStopRequested()
{
	if (hStopRequestInProgress)
		WaitForSingleObject(hStopRequestInProgress, INFINITE);

	if (g_fStopRequested)
		HandleError(kUserAbort, true, IDC_ERROR_USER_ABORT);
}

const int knVisibleOnlyOffset = 0xFFFF;