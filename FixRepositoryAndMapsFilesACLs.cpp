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
	// Because the several API functions used here do not exist on Windows 98 or lower, we must
	// not assume they are present. Instead, we must interrogate the Advapi32.dll.

	// Define function types for the functions we want to use:
	typedef BOOL (WINAPI * CreateWellKnownSidFn)(WELL_KNOWN_SID_TYPE, PSID, PSID, DWORD*);
	typedef DWORD (WINAPI * GetNamedSecurityInfoFn)(LPTSTR, SE_OBJECT_TYPE,
		SECURITY_INFORMATION, PSID*, PSID*, PACL*, PACL*, PSECURITY_DESCRIPTOR*);
	typedef DWORD (WINAPI * SetEntriesInAclFn)(ULONG, PEXPLICIT_ACCESS, PACL, PACL*);
	typedef DWORD (WINAPI * SetNamedSecurityInfoFn)(LPTSTR, SE_OBJECT_TYPE,
		SECURITY_INFORMATION, PSID, PSID, PACL, PACL);

	// Initialize pointers to the functions we want to use:
	CreateWellKnownSidFn _CreateWellKnownSid = NULL;
	GetNamedSecurityInfoFn _GetNamedSecurityInfo = NULL;
	SetEntriesInAclFn _SetEntriesInAcl = NULL;
	SetNamedSecurityInfoFn _SetNamedSecurityInfo = NULL;

	// Initialise pointer to Advapi32 DLL:
	HMODULE hmodAdvapi32 = NULL;

	// Get Windows system folder path:
	_TCHAR * pszSystemFolder = GetFolderPathNew(CSIDL_SYSTEM);
	if (!pszSystemFolder)
	{
		g_Log.Write(_T("Could not get Windows system directory."));
		return false;
	}
	g_Log.Write(_T("Windows system directory = %s."), pszSystemFolder);

	// Remove any terminating backslash:
	int cch = (int)_tcslen(pszSystemFolder);
	if (cch > 1)
		if (pszSystemFolder[cch - 1] == _TCHAR('\\'))
			pszSystemFolder[cch - 1] = 0;

	// Generate full path of Advapi32.dll:
	_TCHAR * pszAdvapi32Dll = new_sprintf(_T("%s\\Advapi32.dll"), pszSystemFolder);

	delete[] pszSystemFolder;
	pszSystemFolder = NULL;

	// Get a handle to the DLL:
	hmodAdvapi32 = LoadLibrary(pszAdvapi32Dll);

	// If we were not successful, report it and quit:
	if (!hmodAdvapi32)
	{
		g_Log.Write(_T("Could not get handle to %s."), pszAdvapi32Dll);

		delete[] pszAdvapi32Dll;
		pszAdvapi32Dll = NULL;

		return false;
	}

	delete[] pszAdvapi32Dll;
	pszAdvapi32Dll = NULL;

	// Get the addresses of the functions we want to use:
	_CreateWellKnownSid = (CreateWellKnownSidFn)GetProcAddress(hmodAdvapi32,
		"CreateWellKnownSid");

	// The following functions have different versions for ANSI and Unicode:
#ifdef UNICODE
	_GetNamedSecurityInfo = (GetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
		"GetNamedSecurityInfoW");
	_SetEntriesInAcl = (SetEntriesInAclFn)GetProcAddress(hmodAdvapi32,
		"SetEntriesInAclW");
	_SetNamedSecurityInfo = (SetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
		"SetNamedSecurityInfoW");
#else
	_GetNamedSecurityInfo = (GetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
		"GetNamedSecurityInfoA");
	_SetEntriesInAcl = (SetEntriesInAclFn)GetProcAddress(hmodAdvapi32,
		"SetEntriesInAclA");
	_SetNamedSecurityInfo = (SetNamedSecurityInfoFn)GetProcAddress(hmodAdvapi32,
		"SetNamedSecurityInfoA");
#endif

	// If we could not get all the functions' addresses, report it and quit:
	if (!_CreateWellKnownSid || !_GetNamedSecurityInfo || !_SetEntriesInAcl || !_SetNamedSecurityInfo)
	{
		if (!_CreateWellKnownSid)
			g_Log.Write(_T("Could not get address of function CreateWellKnownSid."));
		if (!_GetNamedSecurityInfo)
			g_Log.Write(_T("Could not get address of function GetNamedSecurityInfo."));
		if (!_SetEntriesInAcl)
			g_Log.Write(_T("Could not get address of function SetEntriesInAcl."));
		if (!_SetNamedSecurityInfo)
			g_Log.Write(_T("Could not get address of function SetNamedSecurityInfo."));

		FreeLibrary(hmodAdvapi32);

		return false;
	}

	// Initialize variables for creating the required user designator (SID):
	DWORD SidSize = SECURITY_MAX_SID_SIZE;
	PSID TheSID = NULL;

	// Allocate enough memory for the largest possible SID.
	if(!(TheSID = LocalAlloc(LMEM_FIXED, SidSize)))
	{
		g_Log.Write(_T("Could not allocate memory for largest possible SID."));

		FreeLibrary(hmodAdvapi32);
		return false;
	}

	// Create a SID for the given index on the local computer.
	if (!_CreateWellKnownSid(WELL_KNOWN_SID_TYPE(SidIndex), NULL, TheSID, &SidSize))
	{
		g_Log.Write(_T("Could not create SID with index %d."), SidIndex);

		FreeLibrary(hmodAdvapi32);
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

	FreeLibrary(hmodAdvapi32);
	hmodAdvapi32 = NULL;
	
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

	g_Log.Write(_T("Attemping in Repository folder %s..."), pszRepositoryFolder);
	g_Log.Indent();
	fRet = AmendFilesPermissions(pszRepositoryFolder, _T("*.*"), WinAuthenticatedUserSid, GENERIC_ALL);
	g_Log.Unindent();
	g_Log.Write(_T("...Done - returned %s."), fRet? _T("OK") : _T("an error"));

	g_Log.Write(_T("Attemping in MapsTables folder %s..."), pszMapsTablesFolder);
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
