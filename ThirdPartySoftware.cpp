#include <windows.h>
#include <stdio.h>

#include "UsefulStuff.h"
#include "Globals.h"
#include "WIWrapper.h"
#include "ErrorHandler.h"
#include "Resource.h"
#include "LogFile.h"
#include "Dialogs.h"

// Gets the version of the given file.
// If file does not exist or some other error occurs, returns false; otherwise true.
bool GetFileVersion(const char * pszFilePath, __int64 & nVersion)
{
	bool fResult = false;

	// Find out how much space we need to store the version information block.
	DWORD dwVersionInfoSize;
	DWORD dwZero; // Temp variable.
	dwVersionInfoSize = GetFileVersionInfoSize(pszFilePath, &dwZero);
	if (dwVersionInfoSize)
	{
		//Allocate space to store the version info.
		void * pVersionInfo = malloc(dwVersionInfoSize);

		// Use GetFileVersionInfo to copy the version info block into pVersion info.
		if (GetFileVersionInfo(pszFilePath, 0, dwVersionInfoSize, pVersionInfo))
		{
			// Use VerQueryValue to parse the version information data block and get a pointer
			// to the VS_FIXEDFILEINFO structure.
			VS_FIXEDFILEINFO * pFixedFileInfo;
			UINT nBytesReturned;
			if (VerQueryValue(pVersionInfo, "\\", (void **)&pFixedFileInfo, &nBytesReturned))
			{
				nVersion = ((__int64(pFixedFileInfo->dwFileVersionMS)) << 32) |
					pFixedFileInfo->dwFileVersionLS;
				fResult = true;
			}
			free(pVersionInfo);
			pVersionInfo = NULL;
		}
	}
	return fResult;
}


#include "ConfigFunctions.cpp"

#if 0
/*
void UninstallMsde()
{
	// Get the installer product code:
	LONG lResult;
	HKEY hKey = NULL;

	try
	{
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\Setup", NULL, KEY_READ, &hKey);

		// We don't proceed unless the call above succeeds:
		if (ERROR_SUCCESS != lResult && ERROR_FILE_NOT_FOUND != lResult)
		{
			throw HRESULT_FROM_WIN32(lResult);
		}

		if (ERROR_SUCCESS == lResult)
		{
			const int knProductCodeLen = 256;
			char szProductCode[knProductCodeLen];
			DWORD cbData = knProductCodeLen;

			lResult = RegQueryValueEx(hKey, "ProductCode", NULL, NULL, (LPBYTE)szProductCode,
				&cbData);
			RegCloseKey(hKey);

			if (ERROR_SUCCESS == lResult)
			{
				char szCommand[100];
				sprintf(szCommand, "MsiExec.exe /X%s /qb-", szProductCode);
				ExecCmd(szCommand, false);
				return;
			}
		}
	}
	catch (...)
	{
	}
	// report error?
}*/

/*
void UninstallAcrobat50()
{
	// Get the uninstall string:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Adobe Acrobat 5.0",
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knStringLen = 512;
		char szUninstallString[knStringLen];
		DWORD cbData = knStringLen;

		lResult = RegQueryValueEx(hKey, "UninstallString", NULL, NULL,
			(LPBYTE)szUninstallString, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			ExecCmd(szUninstallString, false);
		}
	}
}*/

/*
void UninstallKeyman()
{
	// Get the uninstall string:
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Tavultesoft Keyman 6.0",
		NULL, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lResult)
	{
		const int knStringLen = 512;
		char szUninstallString[knStringLen];
		DWORD cbData = knStringLen;

		lResult = RegQueryValueEx(hKey, "UninstallString", NULL, NULL,
			(LPBYTE)szUninstallString, &cbData);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == lResult)
		{
			ExecCmd(szUninstallString, false);
		}
	}
}*/
#endif
