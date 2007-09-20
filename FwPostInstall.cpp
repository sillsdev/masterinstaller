#pragma once

#include <tchar.h>

#include "InitFwData.cpp"
#include "TestSqlSILFWPresence.cpp"
#include "InitEC.cpp"

int FwPostInstall(const _TCHAR * /*pszCriticalFile*/)
{
	// If SQL Server 2005 is already present, then we need to initialize the FW data
	// now. Otherwise, this needs to wait until after SQL Server is installed.
	// (See SqlServer2005PostInstall.cpp for that case.)
	if (TestSqlSILFWPresence(_T("9.0.0.0"), _T("9.0.32767.32767"), NULL))
		return InitFwData();

	InitEC();

	return 0;
}
