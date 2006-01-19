/*
	Copyright (c) 2003 SIL International.

	Master installer main header file.
*/

#pragma once

class DependencyListInterface;
extern DependencyListInterface * g_pdliDependencies;
extern const char * g_szTitle;
extern const char * g_kszRegProductKeys;
extern bool g_fStopRequested;
extern void HandleError(vErrorType vType, bool fCheckDependentStatus, int nErrorTextId, ...);
extern HANDLE g_hShutDown;
extern void AddThreadHandle(HANDLE handle);
typedef HANDLE (* HelpFn)(const char * pszStem, HANDLE hShutdownEvent);
extern HelpFn Help;
extern bool g_fLessThanWin2k;