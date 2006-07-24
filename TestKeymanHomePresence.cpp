#pragma once

#include <tchar.h>

#include "TestKeymanPresence.cpp"

// Tests for the presence of Tavultesoft's Keyman Free Home Use flavor.
bool TestKeymanHomePresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
							const TCHAR * /*pszCriticalFile*/)
{
	return TestKeymanPresence(pszMinVersion, pszMaxVersion, _T("FreeHomeUse"));
}
