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
	else
		g_Log.SetActiveWriting(); // Force log file anyway, in Temp folder

	// See if user is requesting access to all third-party software:
	if (_tcsstr(pszCmdLine, _T("-manual")) != NULL)
		g_fManualInstall = true;

	// See if user is requesting unattended install:
	if (_tcsstr(pszCmdLine, _T("-silent")) != NULL)
	{
		g_fSilent = true;
		g_Log.Write(_T("Silent installation requested."));
		g_pCmdLineProductSelection = new MainSelectionReturn_t;
		g_pCmdLineProductSelection->m_fInstallRequiredSoftware = true;

		// See if user has preselected which items to install:
		_TCHAR * pszSelection = _tcsstr(pszCmdLine, _T("-select:"));
		if (pszSelection)
		{
			while (pszSelection)
			{
				int iSelection = _tstoi(&(pszSelection[8]));
				g_Log.Write(_T("Command line product selection index: %d."), iSelection);
				g_pCmdLineProductSelection->m_rgiChosen.Add(iSelection);
				pszSelection = _tcsstr(&(pszSelection[8]), _T("-select:"));
			}
		}
		else
		{
			// Select the first main item by default:
			g_pCmdLineProductSelection->m_rgiChosen.Add(0);
			g_Log.Write(_T("No products selected on command line. Defaulting to selection of first product in implied list."));
		}
	}

	g_ProgRecord.SetCmdLine(pszCmdLine);

	// Initialize dynamically allocated functions:
	InitAdvancedApi();

	MasterInstaller_t MasterInstaller;
	MasterInstaller.Run();

	// Release dynamically allocated functions:
	DropAdvancedApi();

	if (g_pCmdLineProductSelection)
		delete g_pCmdLineProductSelection;

	delete[] pszCmdLine;
}

// Global instantiation of log file mechanism:
#include "LogFile.h"
LogFile g_Log;

// Global instantiation of OS Version object:
OSVersion_t g_OSVersion;

// Global instantiation of disk manager:
#include "DiskManager.h"
DiskManager_t g_DiskManager;
