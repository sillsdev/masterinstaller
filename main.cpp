/*
	Copyright (c) 2003 SIL International.

	Master installer main file.
*/

#include <tchar.h>

#include "Control.h"
#include "PersistantProgress.h"
#include "Globals.h"
#include "UsefulStuff.h"

// Application entry point
int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
					 LPSTR lpCmdLine, int /*nCmdShow*/)
{
	// Get details of the version of Windows we're running on:
	g_OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	::GetVersionEx((LPOSVERSIONINFO)(&g_OSversion));

	_TCHAR * pszCmdLine;
#ifdef UNICODE
	int cch = 1 + (int)strlen(lpCmdLine);
	pszCmdLine = new _TCHAR [cch];
	pszCmdLine[0] = 0;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpCmdLine, -1, pszCmdLine, cch);
#else
	pszCmdLine = my_strdup(lpCmdLine);
#endif

	// See if user is requesting a log file be produced:
	_TCHAR * pszLog = _tcsstr(pszCmdLine, _T("-log:"));
	if (pszLog)
		g_Log.SetActiveWriting(&(pszLog[5]));

	// See if user is requesting access to all third-party software:
	if (_tcsstr(pszCmdLine, _T("-manual")) != NULL)
		g_fManualInstall = true;

	g_ProgRecord.SetCmdLine(pszCmdLine);

	// Initialize dynamically allocated functions:
	InitAdvancedApi();

	MasterInstaller_t MasterInstaller;
	MasterInstaller.Run();

	// Release dynamically allocated functions:
	DropAdvancedApi();

	delete[] pszCmdLine;
}

// Global instantiation of log file mechanism:
#include "LogFile.h"
LogFile g_Log;

// Global instantiation of disk manager:
#include "DiskManager.h"
DiskManager_t g_DiskManager;
