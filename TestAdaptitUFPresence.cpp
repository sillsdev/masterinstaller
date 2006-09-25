#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUFPresence(const TCHAR * /*pszMinVersion*/, const TCHAR * /*pszMaxVersion*/,
						 const TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(_T("French_AdaptItU.exe"), true, NULL, NULL);
}
