/*
	Copyright (c) 2003 SIL International.

	Master installer auxilary functions header file.
*/

#pragma once

#include <windef.h>

extern DWORD ExecCmd(LPCTSTR pszCmd, bool fUseCurrentDir, bool fWaitTillExit = true,
					 const char * pszDescription = NULL,
					 const char * pszStatusWindowControl = NULL);
extern bool Reboot();
extern void FriendlyReboot();
extern HRESULT GetFileVersion (LPTSTR filename, VS_FIXEDFILEINFO *pvsf);
extern char * new_sprintf(const char * pszFormat, ...);
extern char * new_vsprintf(const char * pszFormat, const va_list arglist);
extern char * new_ind_sprintf(int nIndent, const char * pszFormat, ...);
extern char * FetchString(int stid);
extern void new_sprintf_concat(char *& rpszMain, int ctInsertNewline,
							   const char * pszAddendumFmt, ...);
extern void new_vsprintf_concat(char *& rpszMain, int ctInsertNewline,
								const char * pszAddendumFmt, const va_list arglist);
extern char * GetInstallerLocation();
extern __int64 GetHugeVersion(const char * pszVersion);
extern char * GenVersionText(__int64 nHugeVersion);
extern bool VersionInRange(__int64 nVersion, const char * pszMinVersion, const char * pszMaxVersion);
extern bool VersionInRange(const char * pszVersion, const char * pszMinVersion, const char * pszMaxVersion);
extern bool WriteClipboardText(const char * pszText);
extern char * GenerateHangingWindowsReport();

extern char * my_strdup(const char * pszOriginal);

extern char g_rgchActiveProcessDescription[];