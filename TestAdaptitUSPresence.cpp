#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUSPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It Unicode"), _TEXT("Spanish_AdaptItU.exe"));
}
