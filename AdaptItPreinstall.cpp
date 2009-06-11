#pragma once

#include <tchar.h>

#include "TestAdaptitGenericPresence.cpp"

// Perform necessary admin prior to installing a version of Adapt It.
int AdaptItPreinstall(SoftwareProduct * /*Product*/)
{
	// If there is a previously-existing installation of Adapt It, we must remove its
	// Books.xml file:
	g_Log.Write(_T("Attempting to locate previous Adapt It installations to remove books.xml..."));
	_TCHAR * pszPreviousPath = MakePathToAdaptIt(false);
	if (pszPreviousPath)
	{
		_TCHAR * pszPreviousBooksXml = MakePath(pszPreviousPath, _T("books.xml"));
		delete[] pszPreviousPath;
		pszPreviousPath = NULL;
		g_Log.Write(_T("Attempting delete %s"), pszPreviousBooksXml);
		DeleteFile(pszPreviousBooksXml);
		delete[] pszPreviousBooksXml;
		pszPreviousBooksXml = NULL;
	}
	// Try again for any Unicode version
	pszPreviousPath = MakePathToAdaptIt(true);
	if (pszPreviousPath)
	{
		TCHAR * pszPreviousBooksXml = MakePath(pszPreviousPath, _T("books.xml"));
		delete[] pszPreviousPath;
		pszPreviousPath = NULL;
		g_Log.Write(_T("Attempting delete %s"), pszPreviousBooksXml);
		DeleteFile(pszPreviousBooksXml);
		delete[] pszPreviousBooksXml;
		pszPreviousBooksXml = NULL;
	}
	g_Log.Write(_T("...Done."));
	return 0;
}
