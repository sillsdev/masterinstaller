#pragma once

#include <tchar.h>

#include "InitEC.cpp"


int PAPostInstall(const _TCHAR * /*pszCriticalFile*/)
{
	InitEC();
	return 0;
}
