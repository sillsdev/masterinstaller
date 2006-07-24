#pragma once

#include <tchar.h>

#include "TestKeymanPresence.cpp"

// Tests for the presence of Tavultesoft's Keyman Standard flavor.
bool TestKeymanStandardPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
							const TCHAR * /*pszCriticalFile*/)
{
	return TestKeymanPresence(pszMinVersion, pszMaxVersion, _T("Standard"));
}
