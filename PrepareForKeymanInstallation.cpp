#pragma once

#include <tchar.h>

// To prepare for Keyman installation, we have to close down any instance that is already
// running.
int PrepareForKeymanInstallation(const TCHAR * /*pszCriticalFile*/)
{
	// Close down any instance that is already running:
	HWND hwnd = FindWindow(_T("Keyman50"), _T("Keyman50"));
	if(hwnd)
	{
		// Keyman is still active
		SendMessage(hwnd, WM_USER+403, 0, 0);
		// internal Keyman message to shut down -
		// or just use DestroyWindow(hwnd);
		Sleep(1000); // Give Keyman time to shut down
		PostMessage(HWND_BROADCAST, WM_NULL, 0, 0); // Release all locks to Keyman files
		Sleep(1000); // Give locks time for release
	}
	return 0;
}
