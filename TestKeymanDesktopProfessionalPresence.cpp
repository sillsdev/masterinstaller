#pragma once

#include <tchar.h>

#include "TestKeyman7Presence.cpp"

// Tests for the presence of Tavultesoft's Keyman Standard flavor.
bool TestKeymanDesktopProfessionalPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
							const TCHAR * /*pszCriticalFile*/)
{
	return TestKeyman7Presence(pszMinVersion, pszMaxVersion, _T("desktop_pro"));
}
