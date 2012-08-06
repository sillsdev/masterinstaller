#pragma once

#include <windows.h>

class ProductKeyHandler_t
{
public:
	ProductKeyHandler_t();
	~ProductKeyHandler_t();

	int TestKey(const _TCHAR * pszKey, DWORD * pfUnlocked, DWORD nProdId, const _TCHAR * pszVersion);
	int GetKeyAltTitle(_TCHAR * pszKey, bool fAlwaysAsk, const _TCHAR * pszTitleMain,
		const _TCHAR * pszTitleDlg, HICON hIcon);

protected:
	// DLL functions types:
	typedef int (* TestKeyFn)(const _TCHAR * pszKey, DWORD * pfUnlocked, DWORD nProdId,
		const _TCHAR * pszVersion);
	typedef int (* GetKeyAltTitleFn)(_TCHAR * pszKey, bool fAlwaysAsk, const _TCHAR * pszTitleMain,
		const _TCHAR * pszTitleDlg, HICON hIcon);
	GetKeyAltTitleFn m_pfGetKeyAltTitle;
	TestKeyFn m_pfTestKey;

	HMODULE m_hmodSecurityDll;

	void Init();
};
