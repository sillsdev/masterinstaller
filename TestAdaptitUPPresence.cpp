#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Portuguese_AdaptItU.exe"), true, pszMinVersion, pszMaxVersion);
}
