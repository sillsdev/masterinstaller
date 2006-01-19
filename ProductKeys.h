#pragma once

#include <windows.h>

class ProductKeyHandler_t
{
public:
	ProductKeyHandler_t();
	~ProductKeyHandler_t();

	int TestKey(const char * pszKey, DWORD * pfUnlocked, DWORD nProdId, const char * pszVersion);
	int GetKeyAltTitle(char * pszKey, bool fAlwaysAsk, const char * pszTitleMain,
		const char * pszTitleDlg, HICON hIcon);

protected:
	// DLL functions types:
	typedef int (* TestKeyFn)(const char * pszKey, DWORD * pfUnlocked, DWORD nProdId,
		const char * pszVersion);
	typedef int (* GetKeyAltTitleFn)(char * pszKey, bool fAlwaysAsk, const char * pszTitleMain,
		const char * pszTitleDlg, HICON hIcon);
	GetKeyAltTitleFn m_pfGetKeyAltTitle;
	TestKeyFn m_pfTestKey;

	HMODULE m_hmodSecurityDll;

	void Init();
};