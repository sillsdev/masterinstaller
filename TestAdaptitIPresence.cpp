#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitIPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Indonesian_AdaptIt.exe"), false, NULL, NULL);
}
