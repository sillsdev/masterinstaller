#pragma once

#include <tchar.h>

// Global instantiation:
bool g_fSqlServerUpgradeNeeded = false;

// Check if an older version of MSDE (SILFW) is present (anything before version 9 - SQL Server 2005).
int TestIfSqlServerUpgradeNeeded(const TCHAR * /*pszCriticalFile*/)
{
	if (TestSqlSILFWPresence(NULL, _T("8.9999"), NULL))
		g_fSqlServerUpgradeNeeded = true;

	return 0;
}
