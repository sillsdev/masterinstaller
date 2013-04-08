/*
	Copyright (c) 2003 SIL International.

	Master installer auxiliary functions header file.
*/

#pragma once

#include <windef.h>
#include <AccCtrl.h>
#include <Aclapi.h>

extern DWORD ExecCmd(LPCTSTR pszCmd, const _TCHAR * pszCurrentDir, bool fWaitTillExit = true,
					 const _TCHAR * pszDescription = NULL,
					 const _TCHAR * pszStatusWindowControl = NULL,
					 const _TCHAR * pszEnvironment = NULL,
					 STARTUPINFO * pStartupInfo = NULL);
extern void AddToPathEnvVar(_TCHAR * pszExtraPath);
extern _TCHAR * GetFolderPathNew(int csidlFolder);
extern bool Reboot();
extern void FriendlyReboot();
extern bool TestResultForRebootRequest(DWORD dwResult);
extern HRESULT GetFileVersion (LPTSTR filename, VS_FIXEDFILEINFO *pvsf);
extern _TCHAR * new_sprintf(const _TCHAR * pszFormat, ...);
extern _TCHAR * new_vsprintf(const _TCHAR * pszFormat, const va_list arglist);
extern _TCHAR * new_ind_sprintf(int nIndent, const _TCHAR * pszFormat, ...);
extern _TCHAR * FetchString(int stid);
extern void new_sprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline,
							   const _TCHAR * pszAddendumFmt, ...);
extern void new_vsprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline,
								const _TCHAR * pszAddendumFmt, const va_list arglist);
extern _TCHAR * MakePath(const _TCHAR * pszFolder, const _TCHAR * pszFile, bool fQuoted = false);
extern void RemoveLastPathSection(_TCHAR * pszFilePath);
extern void NewConcatenatePath(_TCHAR *& pszFolder, const _TCHAR * pszFile);
extern void RemoveTrailingBackslashes(_TCHAR * pszFolder);
extern _TCHAR * NewGetExeFolder();
extern _TCHAR * NewRegString(HKEY hKeyRoot, const _TCHAR * pszSubKey, const _TCHAR * pszValueName = NULL);
extern _TCHAR * GetNewEnvironmentValue(_TCHAR * pszEnvVar);
extern __int64 GetHugeVersion(const _TCHAR * pszVersion);
extern _TCHAR * GenVersionText(__int64 nHugeVersion);
extern bool VersionInRange(__int64 nVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool VersionInRange(const _TCHAR * pszVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool VersionInRangeEx(__int64 nVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool VersionInRangeEx(const _TCHAR * pszVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool WriteClipboardText(const _TCHAR * pszText);
extern _TCHAR * CreateAccountNameFromWellKnownSidIndex(int SidIndex);
extern _TCHAR * GenerateHangingWindowsReport();

extern _TCHAR * my_strdup(const _TCHAR * pszOriginal);

extern _TCHAR g_rgchActiveProcessDescription[];

// Define function types for the AdvApi32 functions we want to use dynamically:
typedef BOOL (WINAPI * CheckTokenMembershipFn)(HANDLE, PSID, PBOOL);
extern CheckTokenMembershipFn _CheckTokenMembership;

typedef BOOL (WINAPI * ConvertStringSidToSidFn)(LPCTSTR StringSid, PSID* Sid);
extern ConvertStringSidToSidFn _ConvertStringSidToSid;

typedef BOOL (WINAPI * CreateWellKnownSidFn)(WELL_KNOWN_SID_TYPE, PSID, PSID, DWORD*);
extern CreateWellKnownSidFn _CreateWellKnownSid;

typedef DWORD (WINAPI * GetNamedSecurityInfoFn)(LPTSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION,
												PSID*, PSID*, PACL*, PACL*,
												PSECURITY_DESCRIPTOR*);
extern GetNamedSecurityInfoFn _GetNamedSecurityInfo;

typedef BOOL (WINAPI * LookupAccountSidFn)(LPCTSTR, PSID, LPTSTR, LPDWORD, LPTSTR, LPDWORD,
										   PSID_NAME_USE);
extern LookupAccountSidFn _LookupAccountSid;

typedef BOOL (WINAPI * QueryServiceStatusExFn)(SC_HANDLE, SC_STATUS_TYPE, LPBYTE, DWORD,
											   LPDWORD);
extern QueryServiceStatusExFn _QueryServiceStatusEx;

typedef DWORD (WINAPI * SetEntriesInAclFn)(ULONG, PEXPLICIT_ACCESS, PACL, PACL*);
extern SetEntriesInAclFn _SetEntriesInAcl;

typedef DWORD (WINAPI * SetNamedSecurityInfoFn)(LPTSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION,
												PSID, PSID, PACL, PACL);
extern SetNamedSecurityInfoFn _SetNamedSecurityInfo;

extern void InitAdvancedApi();
extern void DropAdvancedApi();

//extern bool IsFolderCompressed(const _TCHAR * pszFolderPath);
//extern bool IsFolderCompressed(int csidlFolder);
