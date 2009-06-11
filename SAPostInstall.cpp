#pragma once

#include <tchar.h>

#include "InitEC.cpp"


int SAPostInstall(SoftwareProduct * /*Product*/)
{
	InitEC();
	return 0;
}
