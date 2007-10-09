#pragma once

#include <tchar.h>

static TCHAR * pszUninstallWarning = NULL;

// Uninstall specified product.
int Uninstall(const TCHAR * pszProductCode, const TCHAR * pszStatus)
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
		if (pszUninstallWarning)
		{
			g_Log.Write(_T("Issuing uninstall warning: %s"), pszUninstallWarning);
			if (MessageBox(NULL, pszUninstallWarning, g_pszTitle, MB_ICONSTOP | MB_OKCANCEL)
				!= IDOK)
			{
				g_Log.Write(_T("User chose to cancel."));
				return 3;
			}
			g_Log.Write(_T("User chose to continue with uninstall."));
		}
		TCHAR * pszCmd = new_sprintf(_T("msiexec /qb /x %s"), pszProductCode);
		dwResult = ExecCmd(pszCmd, NULL, true, NULL, _T("Show"));
		delete[] pszCmd;
	}
	else
	{
		g_Log.Write(_T("Product not installed."));
	}

	return int(dwResult != 0);
}

