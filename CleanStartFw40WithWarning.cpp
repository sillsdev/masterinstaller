#pragma once

#include <tchar.h>

#include "CleanStartFw40.cpp"

int CleanStartFw40WithWarning(SoftwareProduct * Product)
{
	pszUninstallWarning = _T("WARNING - In order to install this test version of FieldWorks, the existing version on this machine must be uninstalled first. Click OK if you have backed up your data and agree to have your existing FieldWorks version removed. Otherwise, click Cancel to quit immediately without installing.");
	return CleanStartFw40(Product);
}
