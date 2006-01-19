/*
	Copyright (c) 2003 SIL International.

	Master installer main file.
*/

#include "Control.h"
#include "PersistantProgress.h"
#include "Globals.h"

// Application entry point
int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine,
					 int /*nCmdShow*/)
{
	// See if user is requesting a log file be produced:
	char * pszLog = strstr(lpCmdLine, "-log:");
	if (pszLog)
		g_Log.SetActiveWriting(&(pszLog[5]));

	// See if user is requesting access to all third-party software:
	if (strstr(lpCmdLine, "-manual") != NULL)
		g_fManualInstall = true;

	g_ProgRecord.SetCmdLine(lpCmdLine);

	MasterInstaller_t MasterInstaller;
	MasterInstaller.Run();
}

// Global instantiation of log file mechanism:
#include "LogFile.h"
LogFile g_Log;

// Global instantiation of disk manager:
#include "DiskManager.h"
DiskManager_t g_DiskManager;
