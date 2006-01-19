#include "TestAdaptitGenericPresence.cpp"

bool TestAdaptitPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/,
						 const char * /*pszCriticalFile*/)
{
	return TestAdaptitGenericPresence("Adapt It", "English_AdaptIt.exe");
}
