#pragma once

#include <tchar.h>

#include "RemovePreviousFWs.cpp"
#include "RemovePreviousECs.cpp"

// Deals with a bunch of things that we need to do to clear the way for FieldWorks 4.0
int CleanStartFw40(const TCHAR * pszCriticalFile)
{
	int nResult = 0;

	g_Log.Write(_T("Preparing clean start for FW 4.0..."));
	g_Log.Indent();

	g_Log.Write(_T("Removing previous versions of Encoding Converters..."));
	g_Log.Indent();
	nResult = RemovePreviousECs(pszCriticalFile);
	g_Log.Unindent();

	if (nResult == 0)
	{
		g_Log.Write(_T("...Done removing previous versions of Encoding Converters."));

		g_Log.Write(_T("Removing previous versions of FW..."));
		g_Log.Indent();
		nResult = RemovePreviousFWs(pszCriticalFile);
		g_Log.Unindent();

		if (nResult == 0)
			g_Log.Write(_T("...Done removing previous versions of FW."));
		else
			g_Log.Write(_T("...Failed to remove previous versions of FW."));
		}
	else
		g_Log.Write(_T("...Failed to remove previous versions of Encoding Converters."));

	g_Log.Unindent();

	if (nResult == 0)
		g_Log.Write(_T("...Done preparing clean start for FW 4.0"));
	else
		g_Log.Write(_T("...Failed to prepare clean start for FW 4.0..."));

	return nResult;
}
