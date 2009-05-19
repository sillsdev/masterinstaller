#pragma once

#include <tchar.h>
#include <shlobj.h>

// Fixes Microsoft KB968749:
// http://support.microsoft.com/kb/968749
// Removes the registry settings that show MSXML 6.0 SP2 is installed, as if the
// Windows Installer Cleanup utility had been used, which is the Microsoft recommended
// fix for the SQL Server 2005 installer error.
int MSXML6SP2Killer(const TCHAR * /*pszCriticalFile*/)
{
#if !defined NOLOGGING
	g_Log.Write(_T("Started KB968749 fix."));
#endif

	// This fix is only needed in Windows XP SP3, so test for that:
	if (g_OSversion.dwMajorVersion != 5 || g_OSversion.dwMinorVersion != 1 ||
		g_OSversion.wServicePackMajor != 3)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("Not using Windows XP SP3, so quitting KB968749 fix."));
#endif
		return 0;
	}

#if !defined NOLOGGING
	g_Log.Write(_T("Attempting to delete MSXML 6.0 SP2 installation registry settings (MS KB968749)."));
#endif

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\Features\\096825A1D2A65CB41B34C8A48E1DD969"));

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\SourceList\\Media"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\SourceList\\Net"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\SourceList"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\Products\\096825A1D2A65CB41B34C8A48E1DD969"));

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Installer\\UpgradeCodes\\7AB711B11CB5E91428E0D7F4F314C2B7"));

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UpgradeCodes\\7AB711B11CB5E91428E0D7F4F314C2B7"));

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Components\\6148585497E2D2543B1DC58552AC800C"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Components\\991ED76854051E94F91904447CE302ED"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Components\\DDAF9AF5501EADB469A15D122D743961"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Components\\E5FBE3BD0FDC1574A95FE0E464B0CD8E"));

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\Features"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\InstallProperties"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\Patches"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\096825A1D2A65CB41B34C8A48E1DD969\\Usage"));
	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\096825A1D2A65CB41B34C8A48E1DD969"));

	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs"), 0, KEY_ALL_ACCESS,
		&hKey))
	{
		_TCHAR * pszSystemFolderPath = GetFolderPathNew(CSIDL_SYSTEM);
		if (pszSystemFolderPath)
		{
			_TCHAR * pszVal1 = new_sprintf(_T("%s\\msxml6.dll"), pszSystemFolderPath);
			RegDeleteValue(hKey, pszVal1);
			delete[] pszVal1;
			pszVal1 = NULL;

			_TCHAR * pszVal2 = new_sprintf(_T("%s\\msxml6r.dll"), pszSystemFolderPath);
			RegDeleteValue(hKey, pszVal2);
			delete[] pszVal2;
			pszVal2 = NULL;

			delete[] pszSystemFolderPath;
			pszSystemFolderPath = NULL;
		}
		RegCloseKey(hKey);
		hKey = NULL;
	}

	RegDeleteKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{1A528690-6A2D-4BC5-B143-8C4AE8D19D96}"));

#if !defined NOLOGGING
	g_Log.Write(_T("Exiting KB968749 fix."));
#endif

	return 0;
}
