#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitSPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Spanish_AdaptIt.exe"), false, pszMinVersion, pszMaxVersion);
}
