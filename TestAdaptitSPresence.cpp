#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitSPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						  const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It", "Spanish_AdaptIt.exe");
}
