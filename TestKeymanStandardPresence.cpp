#include "TestKeymanPresence.cpp"

// Tests for the presence of Tavultesoft's Keyman Standard flavor.
bool TestKeymanStandardPresence(const char * pszMinVersion, const char * pszMaxVersion,
							const char * /*pszCriticalFile*/)
{
	return TestKeymanPresence(pszMinVersion, pszMaxVersion, "Standard");
}
