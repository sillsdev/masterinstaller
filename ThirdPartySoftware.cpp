#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "UsefulStuff.h"
#include "Globals.h"
#include "WIWrapper.h"
#include "ErrorHandler.h"
#include "Resource.h"
#include "LogFile.h"
#include "Dialogs.h"

// Forward declaration:
bool GetFileVersion(const _TCHAR * pszFilePath, __int64 & nVersion);

#include "ConfigFunctions.cpp"
