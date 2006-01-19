// Check if the WW installation of the first alhpa release of LexText is present,
// and remove it if so.
int RemoveFirstLexTextAlpha(const char * /*pszCriticalFile*/)
{
	int dwResult = 0;

	// See if the separate WW installation is present:
	const char * pszProductCodeLT = "{D8D2DBAB-7487-4A6E-B369-7F1932B3BDFE}";
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
		g_Log.Write(DisplayStatusText(0, "Removing old LexText Alpha version."));
		g_Log.Write(DisplayStatusText(1,
			"This may take a few minutes."));
		g_Log.Write(DisplayStatusText(2, ""));

		// Remove the WW installation:
		char * pszCmd = new_sprintf("msiexec /qb /x %s", pszProductCodeLT);
		dwResult = ExecCmd(pszCmd, false, true, NULL, "Show");
		delete[] pszCmd;
	}
	return int(dwResult != 0);
}
