#pragma once

#include <tchar.h>
#include <shlobj.h>
#include "msi.h"

int DotNet35SP1PostInstall(SoftwareProduct * /* Product */)
{
	MessageBox(NULL,
		_T("WARNING: Microsoft has released several updates (known as HotFixes) to the .NET 3.5 framework. After completing the installation of FieldWorks, you are strongly advised to use the Windows Update service to download and install them."),
		_T("Updates are required to your system"), MB_ICONSTOP | MB_OK);

	return 0;
}

