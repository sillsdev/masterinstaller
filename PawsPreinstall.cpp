#pragma once

#include <tchar.h>

#include "uninstall.cpp"

// Perform necessary admin prior to installing a version of PAWS.
int PawsPreinstall(const TCHAR * /*pszCriticalFile*/)
{
	// Uninstall any previous version:
	if (0 != Uninstall(_T("{C25A0D3E-32D8-4F91-A4DD-7CD5BCE21C3A}"), _T("Uninstalling previous version of PAWS")))
		return 1;

	return 0;
}