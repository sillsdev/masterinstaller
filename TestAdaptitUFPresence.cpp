#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUFPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						 const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It Unicode", "French_AdaptItU.exe");
}
