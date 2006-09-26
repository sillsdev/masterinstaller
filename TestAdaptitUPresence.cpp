#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						  const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("English_AdaptItU.exe"), true, pszMinVersion, pszMaxVersion);
}