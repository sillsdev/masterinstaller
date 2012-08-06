#pragma once

#include <tchar.h>

#include "TestKeyman6Presence.cpp"

// Tests for the presence of Tavultesoft's Keyman Standard flavor.
bool TestKeymanStandardPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
							SoftwareProduct * /*Product*/)
{
	return TestKeyman6Presence(pszMinVersion, pszMaxVersion, _T("Standard"));
}
