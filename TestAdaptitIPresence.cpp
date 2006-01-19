#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitIPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						  const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It", "Indonesian_AdaptIt.exe");
}
