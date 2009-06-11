#pragma once

#include <tchar.h>

// Global instantiation:
bool g_fSqlServerUpgradeNeeded = false;

// If a current version exists and is 9.00.2047.00 (SQL Server 2005 SP1) or lower, then an
// upgrade will be needed:
int TestIfSqlServerUpgradeNeeded(SoftwareProduct * /*Product*/)
{
	if (TestSqlSILFWPresence(NULL, _T("9.00.2047.00"), NULL))
	{
		g_fSqlServerUpgradeNeeded = true;
#if !defined NOLOGGING
		g_Log.Write(_T("SQL Server will need an upgrade"));
#endif	
	}
	else
	{
		g_fSqlServerUpgradeNeeded = false;
#if !defined NOLOGGING
		g_Log.Write(_T("SQL Server will not need an upgrade"));
#endif	
	}
	return 0;
}
