#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUIPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It Unicode"), _TEXT("Indonesian_AdaptItU.exe"));
}
