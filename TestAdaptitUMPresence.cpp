#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUMPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_TEXT("Adapt It Unicode"), _TEXT("Mandarin_AdaptItU.exe"));
}
