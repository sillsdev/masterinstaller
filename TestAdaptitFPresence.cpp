#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitFPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						 const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It", "French_AdaptIt.exe");
}
