#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUMPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						   const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It Unicode", "Mandarin_AdaptItU.exe");
}
