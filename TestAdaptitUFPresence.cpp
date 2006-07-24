#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUFPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It Unicode"), _TEXT("French_AdaptItU.exe"));
}
