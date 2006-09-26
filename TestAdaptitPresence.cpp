#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("English_AdaptIt.exe"), false, pszMinVersion, pszMaxVersion);
}
