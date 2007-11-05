#pragma once

#include <tchar.h>

// Instantiate path of preserved WordFormingCharOverrides.xml file:
_TCHAR * pszWordFormingCharOverridesPath = NULL;

void PreserveWordFormingCharOverrides()
{
	g_Log.Write(_T("Attempting to preserve WordFormingCharOverrides.xml..."));
	g_Log.Indent();

	_TCHAR * pszFwFolder = NULL;
	LONG lResult;
	HKEY hKey = NULL;

	g_Log.Write(_T("Looking up FW installation folder (4.2 or earlier)..."));
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"), NULL, KEY_READ,
		&hKey);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD cbData = 0;
		_TCHAR * pszFwRootKeyName = _T("RootDir");

		// Get required buffer size:
		lResult = RegQueryValueEx(hKey, pszFwRootKeyName, NULL, NULL, NULL, &cbData);

		if (ERROR_SUCCESS != lResult)
		{
			g_Log.Write(_T("...Not found. Trying 4.9 or later..."));

			pszFwRootKeyName = _T("RootCodeDir");
			lResult = RegQueryValueEx(hKey, pszFwRootKeyName, NULL, NULL, NULL, &cbData);
		}
		if (ERROR_SUCCESS != lResult)
		{
			g_Log.Write(_T("...Still not found."));
		}
		else
		{
			pszFwFolder = new _TCHAR [cbData];

			// Retrieve folder path:
			lResult = RegQueryValueEx(hKey, pszFwRootKeyName, NULL, NULL,
				(LPBYTE)pszFwFolder, &cbData);

			if (ERROR_SUCCESS == lResult)
			{
				// Remove any trailing backslash from the root folder:
				if (pszFwFolder[_tcslen(pszFwFolder) - 1] == '\\')
					pszFwFolder[_tcslen(pszFwFolder) - 1] = 0;

				g_Log.Write(_T("...Previous FW installation folder = '%s'."), pszFwFolder);
			}
			else
				g_Log.Write(_T("...%s could not be read."), pszFwRootKeyName);
		}
		RegCloseKey(hKey);
	}
	else
		g_Log.Write(_T("...FieldWorks registry hive not found."));

	if (pszFwFolder)
	{
		_TCHAR * pszWordFormingCharOverridesSource =
			new_sprintf(_T("%s\\WordFormingCharOverrides.xml"), pszFwFolder);
		pszWordFormingCharOverridesPath =
			new_sprintf(_T("%s\\WordFormingCharOverrides.xml.original"), pszFwFolder);

		delete[] pszFwFolder;
		pszFwFolder = NULL;
		
		if (MoveFile(pszWordFormingCharOverridesSource, pszWordFormingCharOverridesPath))
		{
			g_Log.Write(_T("Renamed %s to %s."), pszWordFormingCharOverridesSource,
				pszWordFormingCharOverridesPath);
		}
		else
		{
			g_Log.Write(_T("Failed to rename %s to %s. Error = %d"), 
				pszWordFormingCharOverridesSource, pszWordFormingCharOverridesPath, GetLastError());
			delete[] pszWordFormingCharOverridesSource;
			pszWordFormingCharOverridesSource = NULL;
		}
	}

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

void ReinstantiateWordFormingCharOverrides()
{
	if (pszWordFormingCharOverridesPath)
	{
		g_Log.Write(_T("Attempting to reinstate former WordFormingCharOverrides.xml..."));
		g_Log.Indent();

		_TCHAR * pszFwFolder = NULL;
		LONG lResult;
		HKEY hKey = NULL;

		g_Log.Write(_T("Looking up FW installation folder..."));
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"), NULL, KEY_READ,
			&hKey);
		if (ERROR_SUCCESS == lResult)
		{
			DWORD cbData = 0;
			_TCHAR * pszFwRootKeyName = _T("RootCodeDir");

			// Get required buffer size:
			lResult = RegQueryValueEx(hKey, pszFwRootKeyName, NULL, NULL, NULL, &cbData);

			if (ERROR_SUCCESS != lResult)
				g_Log.Write(_T("...Cannot get required buffer size."));
			else
			{
				pszFwFolder = new _TCHAR [cbData];

				// Retrieve folder path:
				lResult = RegQueryValueEx(hKey, pszFwRootKeyName, NULL, NULL,
					(LPBYTE)pszFwFolder, &cbData);

				if (ERROR_SUCCESS == lResult)
				{
					// Remove any trailing backslash from the root folder:
					if (pszFwFolder[_tcslen(pszFwFolder) - 1] == '\\')
						pszFwFolder[_tcslen(pszFwFolder) - 1] = 0;

					g_Log.Write(_T("...FW installation folder = '%s'."), pszFwFolder);
				}
				else
					g_Log.Write(_T("...%s could not be read."), pszFwRootKeyName);
			}
			RegCloseKey(hKey);
		}
		else
			g_Log.Write(_T("...FieldWorks registry hive not found."));

		if (pszFwFolder)
		{
			_TCHAR * pszWordFormingCharOverridesDestination =
				new_sprintf(_T("%s\\WordFormingCharOverrides.xml"), pszFwFolder);

			delete[] pszFwFolder;
			pszFwFolder = NULL;

			// Check that the target file already exists:
			FILE * f;
			if (_tfopen_s(&f, pszWordFormingCharOverridesDestination, _T("r")) == 0)
			{
				// It does exist:
				fclose(f);

				// Reset the read-only flag from the target file:
				g_Log.Write(_T("Resetting read-only attribute on %s."),
					pszWordFormingCharOverridesDestination);

				DWORD dwAttrib = GetFileAttributes(pszWordFormingCharOverridesDestination);
				SetFileAttributes(pszWordFormingCharOverridesDestination,
					(dwAttrib & ~FILE_ATTRIBUTE_READONLY));

				// Delete the newly-installed target file:
				g_Log.Write(_T("Deleting %s."), pszWordFormingCharOverridesDestination);

				if (DeleteFile(pszWordFormingCharOverridesDestination))
					g_Log.Write(_T("Done."));
				else
				{
					g_Log.Write(_T("Failed to delete %s. Error = %d"), 
						pszWordFormingCharOverridesDestination, GetLastError());
				}
			} // End if target file already exists.
			else
			{
				g_Log.Write(_T("%s did not previously exist."),
					pszWordFormingCharOverridesDestination);
			}

			if (MoveFile(pszWordFormingCharOverridesPath, pszWordFormingCharOverridesDestination))
			{
				g_Log.Write(_T("Moved %s to %s."), pszWordFormingCharOverridesPath,
					pszWordFormingCharOverridesDestination);
			}
			else
			{
				g_Log.Write(_T("Failed to move %s to %s. Error = %d"), 
					pszWordFormingCharOverridesPath, pszWordFormingCharOverridesDestination,
					GetLastError());
				delete[] pszWordFormingCharOverridesPath;
				pszWordFormingCharOverridesPath = NULL;
				delete[] pszWordFormingCharOverridesDestination;
				pszWordFormingCharOverridesDestination = NULL;
			}
		}
		g_Log.Unindent();
		g_Log.Write(_T("...Done."));
	}
	else
		g_Log.Write(_T("WordFormingCharOverrides.xml was not preserved, so not reinstating it."));
}
