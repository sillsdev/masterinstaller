#pragma once

#include <tchar.h>

// Checks if BART 5 Beta is present, and warns user to uninstall it if so, halting
// this installation of a later BART 5.
int QuitIfBart5BetaPresent(const TCHAR * /*pszCriticalFile*/)
{
	g_Log.Write(_T("Checking for BART 5 Beta..."));
	// See if the installation is present:
	INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(_T("{2E38FB46-1985-4112-9727-B8C012A317A6}"));
	bool fInstalled = false;
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
		g_Log.Write(_T("BART 5 Beta found. Informing user."));

		MessageBox(NULL, _T("Setup has detected an earlier version of BART 5 already installed on your computer. Please use Add/Remove programs to uninstall your current version before installing BART 5.2. This installation will now quit."), g_pszTitle, MB_ICONSTOP | MB_OK);
		return 5;
	}
	g_Log.Write(_T("...Done."));

	return 0;
}
