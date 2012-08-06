#pragma once

#include <tchar.h>

#include "Uninstall.cpp"

// Deals with things that we need to do to clear the way for Speech Analyzer.
int SAPreInstall(SoftwareProduct * /*Product*/)
{
	int nResult = 0;

	// Remove some previous version(s) of SA.
	// List of all previous SA product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{A87E80BE-9BB8-481B-AD60-CA8AD8642C3D}"), // SA 3.0
	};

	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write(_T("Uninstalling earlier SA (Product code %s)."), pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], _T("Removing obsolete Speech Analyzer programs"));
		if (dwResult == 0)
			g_Log.Write(_T("Uninstall succeeded."));
		else
		{
			g_Log.Write(_T("Uninstall failed with error code %d."), dwResult);
			nResult = (int)dwResult;
			return nResult;
		}
	}
	return nResult;
}