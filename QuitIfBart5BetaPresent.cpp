#pragma once

#include <tchar.h>

// Checks if an earlier BART 5 is present, and warns user to uninstall it if so, halting
// this installation of a later BART 5.
int QuitIfBart5BetaPresent(const TCHAR * /*pszCriticalFile*/)
{
	g_Log.Write(_T("Checking for BART 5 Beta..."));
	bool fInstalled = false;
	// See if the installation is present:
	INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(_T("{2E38FB46-1985-4112-9727-B8C012A317A6}"));
	switch (state)
	{
	case INSTALLSTATE_ADVERTISED:
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
	case INSTALLSTATE_DEFAULT:
		fInstalled = true;
	}
	g_Log.Write(_T("Checking for BART 5.2..."));
	state = WindowsInstaller.MsiQueryProductState(_T("{24D88271-9AA4-4B51-B54D-AC2E744C34B7}"));
	switch (state)
	{
	case INSTALLSTATE_ADVERTISED:
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
	case INSTALLSTATE_DEFAULT:
		fInstalled = true;
	}
	if (fInstalled)
	{
		// Display status message, and write progress to log file:
		HideStatusDialog();
		g_Log.Write(_T("Earlier BART 5 found. Informing user."));

		MessageBox(NULL, _T("Setup has detected an earlier version of BART 5 already installed on your computer. Please use Add/Remove programs to uninstall your current version before installing BART 5.2. This installation will now quit."), g_pszTitle, MB_ICONSTOP | MB_OK);
		return 5;
	}
	g_Log.Write(_T("...Done."));

	return 0;
}
