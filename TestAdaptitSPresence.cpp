#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitSPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It"), _TEXT("Spanish_AdaptIt.exe"));
}
