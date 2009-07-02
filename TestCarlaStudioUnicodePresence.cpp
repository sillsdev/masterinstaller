#pragma once

#include <tchar.h>

#include "TestCarlaStudioPresence.cpp"

// Test for Carla Studio installation. Uses the path in the registry for product uninstallation
// to get the version of the main .exe file
bool TestCarlaStudioUnicodePresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
									SoftwareProduct * Product)
{
	return TestCarlaStudioPresence(true, pszMinVersion, pszMaxVersion, Product);
}
