#include "TestKeymanPresence.cpp"

// Tests for the presence of Tavultesoft's Keyman Free Home Use flavor.
bool TestKeymanHomePresence(const char * pszMinVersion, const char * pszMaxVersion,
							const char * /*pszCriticalFile*/)
{
	return TestKeymanPresence(pszMinVersion, pszMaxVersion, "FreeHomeUse");
}
