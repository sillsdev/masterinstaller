#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitFPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It"), _TEXT("French_AdaptIt.exe"));
}
