#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUIPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Indonesian_AdaptItU.exe"), true, pszMinVersion, pszMaxVersion);
}
