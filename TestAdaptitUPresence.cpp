#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						  const _TCHAR * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence(true, pszMinVersion, pszMaxVersion);
}