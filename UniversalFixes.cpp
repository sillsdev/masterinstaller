#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

#include "ErrorHandler.h"
#include "Globals.h"
#include "LogFile.h"
#include "Resource.h"
#include "UsefulStuff.h"
#include "UniversalFixes.h"

// Calls any functions needed to patch up general errors made in the past. Sigh.
void DoUniversalFixes()
{
	g_Log.Write(_T("Doing universal fixes..."));
	g_Log.Indent();
	
	DoFW40UninstallFix();

	g_Log.Unindent();
	g_Log.Write(_T("...done universal fixes."));
}

// Tests to see if a particluar registry key exists. If not, creates it with given
// default data.
// phkeyReturn is a holder for the return of the HKEY of the key in question, but only if created
// by this function. Caller should set parameter to NULL if key not needed, otherwise caller must
// close key handle.
void CheckRegData(HKEY hkeyRoot, TCHAR * pszSubKey, TCHAR * pszDefaultData, HKEY * phkeyReturn = NULL)
{
	LONG lResult;
	HKEY hKey = NULL;

	if (phkeyReturn)
		*phkeyReturn = NULL;

	lResult = RegOpenKeyEx(hkeyRoot, pszSubKey, NULL, KEY_READ, &hKey);

	// Get text of key root for logging purposes:
	TCHAR * pszKeyRoot = _T("");
	if (hkeyRoot == HKEY_CLASSES_ROOT)
		pszKeyRoot = _T("HKEY_CLASSES_ROOT");
	else if (hkeyRoot == HKEY_CURRENT_USER)
		pszKeyRoot = _T("HKEY_CURRENT_USER");
	else if (hkeyRoot == HKEY_LOCAL_MACHINE)
		pszKeyRoot = _T("HKEY_LOCAL_MACHINE");
	else if (hkeyRoot == HKEY_USERS)
		pszKeyRoot = _T("HKEY_USERS");

	if (ERROR_SUCCESS == lResult)
	{
		RegCloseKey(hKey);
		g_Log.Write(_T("%s\\%s is present."), pszKeyRoot, pszSubKey);
	}
	else
	{
		// Key was missing:
		g_Log.Write(_T("%s\\%s was missing."), pszKeyRoot, pszSubKey);

		// Create it and any subkeys and data:
		lResult = RegCreateKeyEx(hkeyRoot, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, NULL, &hKey, NULL);

		if (ERROR_SUCCESS == lResult)
		{
			
			g_Log.Write(_T("Created %s\\%s."), pszKeyRoot, pszSubKey);

			lResult = RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)pszDefaultData,
				(DWORD)(_tcslen(pszDefaultData) * sizeof(TCHAR)));

			if (ERROR_SUCCESS == lResult)
				g_Log.Write(_T("Written default value [%s]."), pszDefaultData);
			else
				g_Log.Write(_T("Unable to write default value [%s]."), pszDefaultData);

			if (phkeyReturn)
				*phkeyReturn = hKey;
			else
				RegCloseKey(hKey);
		}
		else
			g_Log.Write(_T("Could not create %s\\%s."), pszKeyRoot, pszSubKey);
	}
}

// When FieldWorks 4.0 is uninstalled, it removes a handful of system registry keys.
// This function tests to see if any are missing, and replaces those that are.
void DoFW40UninstallFix()
{
	g_Log.Write(_T("Testing for missing system registry keys following uninstallation of FW 4.0..."));
	g_Log.Indent();

	CheckRegData(HKEY_CLASSES_ROOT, _T("CLSID\\{00000320-0000-0000-C000-000000000046}"), _T("PSFactoryBuffer"));

	HKEY hkeyPSFactoryBuffer = NULL;
	_TCHAR * pszSystemFolderPath = GetFolderPathNew(CSIDL_SYSTEM);
	
	if (pszSystemFolderPath)
	{
		_TCHAR * pszInprocServer32 = new_sprintf(_T("%s\\ole32.dll"), pszSystemFolderPath);

		delete[] pszSystemFolderPath;
		pszSystemFolderPath = NULL;

		CheckRegData(HKEY_CLASSES_ROOT, _T("CLSID\\{00000320-0000-0000-C000-000000000046}\\InprocServer32"),
			pszInprocServer32, &hkeyPSFactoryBuffer);

		delete[] pszInprocServer32;
		pszInprocServer32 = NULL;
	}

	if (hkeyPSFactoryBuffer)
	{
		if (ERROR_SUCCESS == RegSetValueEx(hkeyPSFactoryBuffer, _T("ThreadingModel"), 0, REG_SZ, (BYTE *)_T("Both"), 4 * sizeof(TCHAR)))
			g_Log.Write(_T("ThreadingModel set to Both."));
		else
			g_Log.Write(_T("Unable to set ThreadingModel to Both."));
		RegCloseKey(hkeyPSFactoryBuffer);
	}

	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000B-0000-0000-C000-000000000046}"), _T("IStorage"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000B-0000-0000-C000-000000000046}\\NumMethods"), _T("18"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000B-0000-0000-C000-000000000046}\\ProxyStubClsid32"), _T("{00000320-0000-0000-C000-000000000046}"));

	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000C-0000-0000-C000-000000000046}"), _T("IStream"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000C-0000-0000-C000-000000000046}\\NumMethods"), _T("14"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000C-0000-0000-C000-000000000046}\\ProxyStubClsid32"), _T("{00000320-0000-0000-C000-000000000046}"));

	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000D-0000-0000-C000-000000000046}"), _T("IEnumSTATSTG"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000D-0000-0000-C000-000000000046}\\NumMethods"), _T("7"));
	CheckRegData(HKEY_CLASSES_ROOT, _T("Interface\\{0000000D-0000-0000-C000-000000000046}\\ProxyStubClsid32"), _T("{00000320-0000-0000-C000-000000000046}"));

	g_Log.Unindent();
	g_Log.Write(_T("...done FW 4.0 uninstall fix."));
}
