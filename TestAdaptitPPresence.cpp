#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Portuguese_AdaptIt.exe"), false, pszMinVersion, pszMaxVersion);
}
