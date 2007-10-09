#pragma once

#include <TCHAR.h>

// Check if the WW installation of the first alhpa release of LexText is present,
// and remove it if so.
int RemoveFirstLexTextAlpha(const _TCHAR * /*pszCriticalFile*/)
{
	int dwResult = 0;

	// See if the separate WW installation is present:
	const TCHAR * pszProductCodeLT = _T("{D8D2DBAB-7487-4A6E-B369-7F1932B3BDFE}");
	INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(pszProductCodeLT);
	bool fInstalled = false;
	switch (state)
	{
	case INSTALLSTATE_ADVERTISED:
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
	case INSTALLSTATE_DEFAULT:
		// WW installation was found:
		fInstalled = true;
	}
	if (fInstalled)
	{
		// Display status message, and write progress to log file:
		ShowStatusDialog();
		g_Log.Write(DisplayStatusText(0, _T("Removing old LexText Alpha version.")));
		g_Log.Write(DisplayStatusText(1,
			_T("This may take a few minutes.")));
		g_Log.Write(DisplayStatusText(2, _T("")));

		// Remove the WW installation:
		TCHAR * pszCmd = new_sprintf(_T("msiexec /qb /x %s"), pszProductCodeLT);
		dwResult = ExecCmd(pszCmd, NULL, true, NULL, _T("Show"));
		delete[] pszCmd;
	}
	return int(dwResult != 0);
}
