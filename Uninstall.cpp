#pragma once

#include <tchar.h>

// Uninstall specified product.
bool Uninstall(const TCHAR * pszProductCode, const TCHAR * pszStatus)
{
	int dwResult = 0;

	// See if the installation is present:
	INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(pszProductCode);
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
		ShowStatusDialog();
		g_Log.Write(DisplayStatusText(0, pszStatus));
		g_Log.Write(DisplayStatusText(1, _T("This may take a few minutes.")));
		g_Log.Write(DisplayStatusText(2, _T("")));

		// Remove the installation:
		TCHAR * pszCmd = new_sprintf(_T("msiexec /qb /x %s"), pszProductCode);
		dwResult = ExecCmd(pszCmd, false, true, NULL, _T("Show"));
		delete[] pszCmd;
	}
	else
	{
		g_Log.Write(_T("Product not installed."));
	}

	return int(dwResult != 0);
}

