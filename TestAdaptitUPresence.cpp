#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						  const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It Unicode", "English_AdaptItU.exe");
}