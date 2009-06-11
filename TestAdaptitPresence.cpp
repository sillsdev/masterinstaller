#pragma once

#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						 SoftwareProduct * /*Product*/)
{
	return TestAdaptitGenericPresence(false, pszMinVersion, pszMaxVersion);
}
