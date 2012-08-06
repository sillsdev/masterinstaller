#pragma once

#include <tchar.h>

#include "TestKeyman7Presence.cpp"

// Tests for the presence of Tavultesoft's Keyman Standard flavor.
bool TestKeymanDesktopLightPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
									SoftwareProduct * /*Product*/)
{
	return TestKeyman7Presence(pszMinVersion, pszMaxVersion, _T("desktop_light"));
}
