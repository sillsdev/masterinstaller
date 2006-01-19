#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUSPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						   const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It Unicode", "Spanish_AdaptItU.exe");
}
