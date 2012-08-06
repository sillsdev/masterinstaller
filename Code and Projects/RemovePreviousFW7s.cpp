#pragma once

#include <tchar.h>

#include "Uninstall.cpp"
#include "SearchOtherUsersInstallations.cpp"

// Remove any existing version of FieldWorks 7:
int RemovePreviousFW7s(SoftwareProduct * /*Product*/)
{
	int nResult = 0;

	// List of all previous FW 7 product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{256A50FA-761A-488b-B282-3543C1DDAE76}"), // FW 7 alpha and beta
	};
	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write(_T("Uninstalling earlier FW (Product code %s)."), pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], _T("Preparing for new FieldWorks programs"));
		if (dwResult == 0)
			g_Log.Write(_T("Uninstall succeeded."));
		else
		{
			g_Log.Write(_T("Uninstall failed with error code %d."), dwResult);
			nResult = (int)dwResult;
			return nResult;
		}
	}

	// See if any of the above products were installed under other user's accounts:
	g_Log.Write(_T("Searching for installations in other user accounts..."));
	g_Log.Indent();
	int cIssues = 0; // Number of issues found.
	TCHAR * pszReport = SearchOtherUsersInstallations(pszProductCode,
		sizeof(pszProductCode) / sizeof(pszProductCode[0]),
		_T("Earlier version of SIL FieldWorks"), cIssues);
	g_Log.Unindent();
	g_Log.Write(_T("...done."));
	if (pszReport)
	{
		TCHAR * pszMessage = NULL;
		if (cIssues == 1)
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there is a program installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under another user's account which is incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user, then uninstall the program "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		else
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there are programs installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under other users' accounts which are incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user in each case, then uninstall the programs "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		new_sprintf_concat(pszMessage, 0, _T(" If this is not possible, check out alternatives in the "));
		new_sprintf_concat(pszMessage, 0, _T("Installation Issues section of the SetupFW.rtf "));
		new_sprintf_concat(pszMessage, 0, _T("file in the FieldWorks folder of this CD (or web download)."));

		new_sprintf_concat(pszMessage, 1, _T("%s"), pszReport);
		delete[] pszReport;
		new_sprintf_concat(pszMessage, 2, _T("This installation will now terminate."));
		HideStatusDialog();
		MessageBox(NULL, pszMessage, g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Found installations in other user accounts - reported: %s"), pszMessage);
		nResult = -2;
	}

	return nResult;
}
