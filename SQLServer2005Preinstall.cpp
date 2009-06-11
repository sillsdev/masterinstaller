#pragma once

#include <tchar.h>

#include "TestIfSqlServerUpgradeNeeded.cpp"
#include "MSXML6SP2Killer.cpp"

// If a current version exists and is 9.00.2047.00 (SQL Server 2005 SP1) or lower, then an
// upgrade will be needed:
int SQLServer2005Preinstall(SoftwareProduct * Product)
{
	TestIfSqlServerUpgradeNeeded(Product);

	MSXML6SP2Killer(Product);

	return 0;
}