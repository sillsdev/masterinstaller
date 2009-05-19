#pragma once

#include <tchar.h>

#include "TestIfSqlServerUpgradeNeeded.cpp"
#include "MSXML6SP2Killer.cpp"

// If a current version exists and is 9.00.2047.00 (SQL Server 2005 SP1) or lower, then an
// upgrade will be needed:
int SQLServer2005Preinstall(const TCHAR * pszCriticalFile)
{
	TestIfSqlServerUpgradeNeeded(pszCriticalFile);

	MSXML6SP2Killer(pszCriticalFile);

	return 0;
}