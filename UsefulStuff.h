/*
	Copyright (c) 2003 SIL International.

	Master installer auxilary functions header file.
*/

#pragma once

#include <windef.h>

extern DWORD ExecCmd(LPCTSTR pszCmd, const _TCHAR * pszCurrentDir, bool fWaitTillExit = true,
					 const _TCHAR * pszDescription = NULL,
					 const _TCHAR * pszStatusWindowControl = NULL,
					 const _TCHAR * pszEnvironment = NULL);
extern void AddToPathEnvVar(_TCHAR * pszExtraPath);
extern _TCHAR * GetFolderPathNew(int csidlFolder);
extern bool Reboot();
extern void FriendlyReboot();
extern HRESULT GetFileVersion (LPTSTR filename, VS_FIXEDFILEINFO *pvsf);
extern _TCHAR * new_sprintf(const _TCHAR * pszFormat, ...);
extern _TCHAR * new_vsprintf(const _TCHAR * pszFormat, const va_list arglist);
extern _TCHAR * new_ind_sprintf(int nIndent, const _TCHAR * pszFormat, ...);
extern _TCHAR * FetchString(int stid);
extern void new_sprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline,
							   const _TCHAR * pszAddendumFmt, ...);
extern void new_vsprintf_concat(_TCHAR *& rpszMain, int ctInsertNewline,
								const _TCHAR * pszAddendumFmt, const va_list arglist);
extern __int64 GetHugeVersion(const _TCHAR * pszVersion);
extern _TCHAR * GenVersionText(__int64 nHugeVersion);
extern bool VersionInRange(__int64 nVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool VersionInRange(const _TCHAR * pszVersion, const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
extern bool WriteClipboardText(const _TCHAR * pszText);
extern _TCHAR * CreateAccountNameFromWellKnownSidIndex(int SidIndex);
extern _TCHAR * GenerateHangingWindowsReport();

extern _TCHAR * my_strdup(const _TCHAR * pszOriginal);

extern _TCHAR g_rgchActiveProcessDescription[];