#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitIPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It"), _TEXT("Indonesian_AdaptIt.exe"));
}
