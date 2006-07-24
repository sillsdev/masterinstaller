#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It Unicode"), _TEXT("English_AdaptItU.exe"));
}