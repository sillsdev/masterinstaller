#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUSPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Spanish_AdaptItU.exe"), true, pszMinVersion, pszMaxVersion);
}
