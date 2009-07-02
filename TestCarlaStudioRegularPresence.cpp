#pragma once

#include <tchar.h>

#include "TestCarlaStudioPresence.cpp"

// Test for Carla Studio installation. Uses the path in the registry for product uninstallation
// to get the version of the main .exe file
bool TestCarlaStudioRegularPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
									SoftwareProduct * Product)
{
	return TestCarlaStudioPresence(false, pszMinVersion, pszMaxVersion, Product);
}
