#pragma once

#include <tchar.h>

#include "InitEC.cpp"


int PAPostInstall(SoftwareProduct * /*Product*/)
{
	InitEC();
	return 0;
}
