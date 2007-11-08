#pragma once

#include <tchar.h>

#include "TestKeyman6Presence.cpp"

// Tests for the presence of Tavultesoft's Keyman Free Home Use flavor.
bool TestKeymanHomePresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
							const TCHAR * /*pszCriticalFile*/)
{
	return TestKeyman6Presence(pszMinVersion, pszMaxVersion, _T("FreeHomeUse"));
}
