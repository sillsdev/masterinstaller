#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						 const _TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(false, pszMinVersion, pszMaxVersion);
}
