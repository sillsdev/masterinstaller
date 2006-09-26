#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUMPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						   const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("Mandarin_AdaptItU.exe"), true, pszMinVersion, pszMaxVersion);
}
