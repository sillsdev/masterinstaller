#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitFPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("French_AdaptIt.exe"), false, pszMinVersion, pszMaxVersion);
}
