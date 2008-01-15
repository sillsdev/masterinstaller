#pragma once

#include <tchar.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <shlobj.h>

// Alters the file permissions on all files in the given folder matching the given search
// pattern (which can be a specific file). The existing permissions are appended with the
// given permissions being granted to the given user.
// Returns true if all went well, otherwise false.
bool AmendFilesPermissions(_TCHAR * pszFolder, _TCHAR * pszSearchPattern, int SidIndex,
						   DWORD dwPermissions)
{
	// If we could not get all the functions' addresses, report it and quit:
	if (!_CreateWellKnownSid || !_GetNamedSecurityInfo || !_SetEntriesInAcl || !_SetNamedSecurityInfo)
	{
		g_Log.Write(_T("Dynamic functions not allocated."));
		return false;
	}

	// Initialize variables for creating the required user designator (SID):
	DWORD SidSize = SECURITY_MAX_SID_SIZE;
	PSID TheSID = NULL;

	// Allocate enough memory for the largest possible SID.
	if(!(TheSID = LocalAlloc(LMEM_FIXED, SidSize)))
	{
		g_Log.Write(_T("Could not allocate memory for largest possible SID."));
		return false;
	}

	// Create a SID for the given index on the local computer.
	if (!_CreateWellKnownSid(WELL_KNOWN_SID_TYPE(SidIndex), NULL, TheSID, &SidSize))
	{
		g_Log.Write(_T("Could not create SID with index %d."), SidIndex);

		return false;
	}

	// Set up the new permissions to add into the existing DACL:
	EXPLICIT_ACCESS eaPermissions;
	ZeroMemory(&eaPermissions, sizeof(EXPLICIT_ACCESS));

	// Set given permissions for given user:
	eaPermissions.grfAccessPermissions = dwPermissions;
	eaPermissions.grfAccessMode = SET_ACCESS;
	eaPermissions.grfInheritance = NO_INHERITANCE;
	eaPermissions.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	eaPermissions.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	eaPermissions.Trustee.ptstrName = (LPTSTR)TheSID;

	g_Log.Write(_T("Searching for files in folder %s matching %s."), pszFolder, pszSearchPattern);
	g_Log.Indent();

	// Search for all files matching specified pattern:
	WIN32_FIND_DATA wfd;
	_TCHAR * pszSearch = new_sprintf(_T("%s\\%s"), pszFolder, pszSearchPattern);
	HANDLE hFind = FindFirstFile(pszSearch, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				g_Log.Write(_T("Found %s."), wfd.cFileName);

				_TCHAR * pszFilePath = new_sprintf(_T("%s\\%s"), pszFolder, wfd.cFileName);

				// Get existing permissions:
				PACL pOldDACL = NULL;
				PSECURITY_DESCRIPTOR pSD = NULL;
				DWORD dwRes = _GetNamedSecurityInfo(pszFilePath, SE_FILE_OBJECT,
					DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDACL, NULL, &pSD);

				// We never needed the pSD:
				if(pSD != NULL) 
					LocalFree((HLOCAL) pSD); 

				if (dwRes != ERROR_SUCCESS)
					g_Log.Write(_T("Could not retrieve current security info. GetNamedSecurityInfo returned %d."), dwRes);
				else
				{
					// Merge existing permissions with our new permissions into a new ACL:
					PACL pACL = NULL;
					dwRes = _SetEntriesInAcl(1, &eaPermissions, pOldDACL, &pACL);

					if (dwRes != ERROR_SUCCESS)
						g_Log.Write(_T("Could not set security info structure. SetEntriesInAcl returned %d."), dwRes);
					else
					{
						// Try to modify the file's DACL:
						dwRes = _SetNamedSecurityInfo(pszFilePath, SE_FILE_OBJECT,
							DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);

						if (dwRes == ERROR_SUCCESS)
							g_Log.Write(_T("Set security info to %d for user index %d."), dwPermissions, SidIndex);
						else
							g_Log.Write(_T("Could not set security info on file. SetNamedSecurityInfo returned %d."), dwRes);
					}
				}

				delete[] pszFilePath;
				pszFilePath = NULL;
			} // End if found file is not really a directory
		} while (FindNextFile(hFind, &wfd));
			
		FindClose(hFind);
		hFind = NULL;

	} // End if first find was successful.

	g_Log.Unindent();
	g_Log.Write(_T("Finished search."));

	delete[] pszSearch;
	pszSearch = NULL;

	// When done, free the memory used.
	LocalFree(TheSID);
	TheSID = NULL;

	return true;
}

// Fixes a problem that occurred in Bob's AppDataMover for Encoding Converters and Repository,
// where moved files did not get write permissions. We add permissions to all files
// in the Repository and MapsTables directories for all authenticated users.
void FixRepositoryAndMapsFilesACLs()
{
	g_Log.Write(_T("Attempting to give authenticated users read/write/modify permissions on all files in the Repository and MapsTables folders..."));
	g_Log.Indent();

	// Form paths to the required folders, via the APPDATA folder:
	_TCHAR * pszAppDataFolder = GetFolderPathNew(CSIDL_COMMON_APPDATA);
	_TCHAR * pszRepositoryFolder = new_sprintf(_T("%s\\SIL\\Repository"), pszAppDataFolder);
	_TCHAR * pszMapsTablesFolder = new_sprintf(_T("%s\\SIL\\MapsTables"), pszAppDataFolder);
	delete[] pszAppDataFolder;
	pszAppDataFolder = NULL;

	bool fRet;

	g_Log.Write(_T("Attempting in Repository folder %s..."), pszRepositoryFolder);
	g_Log.Indent();
	fRet = AmendFilesPermissions(pszRepositoryFolder, _T("*.*"), WinAuthenticatedUserSid, GENERIC_ALL);
	g_Log.Unindent();
	g_Log.Write(_T("...Done - returned %s."), fRet? _T("OK") : _T("an error"));

	g_Log.Write(_T("Attempting in MapsTables folder %s..."), pszMapsTablesFolder);
	g_Log.Indent();
	fRet = AmendFilesPermissions(pszMapsTablesFolder, _T("*.*"), WinAuthenticatedUserSid, GENERIC_ALL);
	g_Log.Unindent();
	g_Log.Write(_T("...Done - returned %s."), fRet? _T("OK") : _T("an error"));

	delete[] pszRepositoryFolder;
	pszRepositoryFolder = NULL;
	delete[] pszMapsTablesFolder;
	pszMapsTablesFolder = NULL;

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}
