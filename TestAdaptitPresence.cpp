#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It"), _TEXT("English_AdaptIt.exe"));
}
