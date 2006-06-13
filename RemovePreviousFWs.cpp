// Remove any existing version of FieldWorks

bool Uninstall(const char * pszProductCode, const char * pszStatus)
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
		// WW installation was found:
		fInstalled = true;
	}
	if (fInstalled)
	{
		// Display status message, and write progress to log file:
		ShowStatusDialog();
		g_Log.Write(DisplayStatusText(0, pszStatus));
		g_Log.Write(DisplayStatusText(1, "This may take a few minutes."));
		g_Log.Write(DisplayStatusText(2, ""));

		// Remove the installation:
		char * pszCmd = new_sprintf("msiexec /qb /x %s", pszProductCode);
		dwResult = ExecCmd(pszCmd, false, true, NULL, "Show");
		delete[] pszCmd;
	}
	return int(dwResult != 0);
}

int RemovePreviousFWs(const char * /*pszCriticalFile*/)
{
	int nResult = 0;

	// List of all previous FW product codes:
	const char * pszProductCode[] = 
	{
		"{E161291A-BD0C-46F6-9F39-6B165BDD4CCC}", // FW 0.8.0, 1.3 and 1.4 ("Aussie", Disks 3 and 4)
		"{2D424459-8B40-41F3-94F7-4D02BFAC39E7}", // FW 2.0 (Disk 5)
		"{D8D2DBAB-7487-4A6E-B369-7F1932B3BDFE}", // old WW installer for Harvwest alpha
		"{59EDA034-26E5-4B7A-9924-335BA200B461}", // FW 3.0
		"{823B00C2-18DB-451B-93F6-068E96A90BFD}", // FW 3.1
	};
	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write("Uninstalling earlier FW (Product code %s).", pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], "Preparing for new FieldWorks programs");
		if (dwResult == 0)
			g_Log.Write("Uninstall succeeded.");
		else
		{
			g_Log.Write("Uninstall failed with error code %d.", dwResult);
			nResult = (int)dwResult;
		}
	}
	return nResult;
}
