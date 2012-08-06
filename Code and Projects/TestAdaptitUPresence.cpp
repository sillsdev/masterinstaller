#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						  SoftwareProduct * /*Product*/)
{
	return TestAdaptitGenericPresence(true, pszMinVersion, pszMaxVersion);
}