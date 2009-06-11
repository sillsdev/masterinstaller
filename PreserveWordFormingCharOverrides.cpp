#pragma once

#include <tchar.h>

// Instantiate path of preserved WordFormingCharOverrides.xml file:
_TCHAR * pszWordFormingCharOverridesPath = NULL;

void PreserveWordFormingCharOverrides()
{
	g_Log.Write(_T("Attempting to preserve WordFormingCharOverrides.xml..."));
	g_Log.Indent();

	g_Log.Write(_T("Looking up FW installation folder (4.2 or earlier)..."));

	_TCHAR * pszFwFolder = NewRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"),
		_T("RootDir"));

	if (!pszFwFolder)
	{
		g_Log.Write(_T("...Not found. Trying 4.9 or later..."));

		pszFwFolder = NewRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"),
			_T("RootCodeDir"));

		if (!pszFwFolder)
			g_Log.Write(_T("...Still not found."));

	}
	if (pszFwFolder)
	{
		// Remove any trailing backslash from the root folder:
		RemoveTrailingBackslashes(pszFwFolder);

		g_Log.Write(_T("...Previous FW installation folder = '%s'."), pszFwFolder);

		_TCHAR * pszWordFormingCharOverridesSource = MakePath(pszFwFolder,
			_T("WordFormingCharOverrides.xml"));
		pszWordFormingCharOverridesPath = MakePath(pszFwFolder,
			_T("WordFormingCharOverrides.xml.original"));

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

		g_Log.Write(_T("Looking up FW installation folder..."));

		_TCHAR * pszFwFolder = NewRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SIL\\FieldWorks"),
			_T("RootCodeDir"));

		if (pszFwFolder)
		{
			// Remove any trailing backslash from the root folder:
			RemoveTrailingBackslashes(pszFwFolder);

			g_Log.Write(_T("...FW installation folder = '%s'."), pszFwFolder);
		}
		else
			g_Log.Write(_T("...RootCodeDir could not be read."));

		if (pszFwFolder)
		{
			_TCHAR * pszWordFormingCharOverridesDestination = MakePath(pszFwFolder,
				_T("WordFormingCharOverrides.xml"));

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
