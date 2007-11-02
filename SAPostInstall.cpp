#pragma once

#include <tchar.h>

#include "InitEC.cpp"


int SAPostInstall(const _TCHAR * /*pszCriticalFile*/)
{
	InitEC();
	return 0;
}
