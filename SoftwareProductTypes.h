/*
	SoftwareProductTypes.h

	These are the types needed in the SoftwareProduct Class.
	They are separated out into this file so that if edited, the Report system will be kept up
	to date.
*/

typedef bool (* pfnAuxTestPresence)(const char * pszMinVersion, const char * pszMaxVersion, const char * pszCriticalFile);
typedef int (* pfnPreInstallation)(const char * pszCriticalFile);
typedef DWORD (* pfnInstall)(bool fFlag, const char * pszCriticalFile);
typedef int (* pfnPostInstallation)(const char * pszCriticalFile);
