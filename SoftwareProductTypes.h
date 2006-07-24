/*
	SoftwareProductTypes.h

	These are the types needed in the SoftwareProduct Class.
	They are separated out into this file so that if edited, the Report system will be kept up
	to date.
*/

typedef bool (* pfnAuxTestPresence)(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion, const _TCHAR * pszCriticalFile);
typedef int (* pfnPreInstallation)(const _TCHAR * pszCriticalFile);
typedef DWORD (* pfnInstall)(bool fFlag, const _TCHAR * pszCriticalFile);
typedef int (* pfnPostInstallation)(const _TCHAR * pszCriticalFile);
