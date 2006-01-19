#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitUIPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						   const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It Unicode", "Indonesian_AdaptItU.exe");
}
