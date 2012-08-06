#pragma once

#include <tchar.h>

// Installs a couple of MS updates just for Vista, Vista's unique wusa.exe update installer.
// Unfortunately, there is no simple way to detect if these updates are present, hence the
// need for this function, which always returns zero, as if nothing went wrong.
int VistaKB958481andKB958483(SoftwareProduct * /*Product*/)
{
	g_Log.Write(_T("Investigating need for KB958481 and KB958483 for Vista..."));

	// Test that we are running Vista:
	if (g_OSVersion < OSVersion_t::Vista || g_OSVersion >= OSVersion_t::W7)
	{
		g_Log.Write(_T("...not running Vista."));
		return 0;
	}

	DWORD dwResult;
	// Launch wusa.exe with the KB958481 update:
	g_Log.Write(_T("Launching update installer with KB958481..."));
	ShowStatusDialog();
	DisplayStatusText(0, _T("Installing Hotfix KB958481 (prerequisite of the .NET Compatibility Update)"));
	dwResult = ExecCmd(_T("wusa.exe \"Utilities\\DotNet 3.5 SP1\\Windows6.0-KB958481-x86.msu\" /quiet /norestart"), NULL);

	if (TestResultForRebootRequest(dwResult))
	{
		g_Log.Write(_T("KB958481 requested a reboot."));
		g_fRebootPending = true;
	}

	// Launch wusa.exe with the KB958483 update:
	g_Log.Write(_T("Launching update installer with KB958483..."));
	DisplayStatusText(0, _T("Installing Hotfix KB958483 (prerequisite of the .NET Compatibility Update)"));
	dwResult = ExecCmd(_T("wusa.exe \"Utilities\\DotNet 3.5 SP1\\Windows6.0-KB958483-x86.msu\" /quiet /norestart"), NULL);

	if (TestResultForRebootRequest(dwResult))
	{
		g_Log.Write(_T("KB958483 requested a reboot."));
		g_fRebootPending = true;
	}

	g_Log.Write(_T("...Done."));

	return 0;
}