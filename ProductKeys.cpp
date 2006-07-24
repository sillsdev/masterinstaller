#include <tchar.h>

#include "ProductKeys.h"
#include "ErrorHandler.h"
#include "LogFile.h"
#include "Resource.h"

ProductKeyHandler_t::ProductKeyHandler_t()
{
	m_pfGetKeyAltTitle = NULL;
	m_pfTestKey = NULL;
	m_hmodSecurityDll = NULL;
}

ProductKeyHandler_t::~ProductKeyHandler_t()
{
	// Free up resources:
	FreeLibrary(m_hmodSecurityDll);
	m_hmodSecurityDll = NULL;
}

void ProductKeyHandler_t::Init()
{
	// Load up the DLL that handles security:
	m_hmodSecurityDll = LoadLibrary(_T("InstallerHelp.dll"));
	if (m_hmodSecurityDll)
	{
		m_pfGetKeyAltTitle = (GetKeyAltTitleFn)GetProcAddress(m_hmodSecurityDll,
			"GetKeyAltTitle");
		m_pfTestKey = (TestKeyFn)GetProcAddress(m_hmodSecurityDll, "TestKey");
	}
	if (!m_hmodSecurityDll || !m_pfGetKeyAltTitle || !m_pfTestKey)
	{
		if (m_hmodSecurityDll)
		{
			FreeLibrary(m_hmodSecurityDll);
			m_hmodSecurityDll = NULL;
			g_Log.Write(_T("Could not find one of the InstallerHelp.dll functions."));
		}
		else
			g_Log.Write(_T("Could not load InstallerHelp.dll."));
		HandleError(kFatal, false, IDC_ERROR_KEY_DLL);
	}
}

int ProductKeyHandler_t::TestKey(const _TCHAR * pszKey, DWORD * pfUnlocked, DWORD nProdId,
								 const _TCHAR * pszVersion)
{
	if (!m_pfTestKey)
		Init();
	return m_pfTestKey(pszKey, pfUnlocked, nProdId, pszVersion);
}

int ProductKeyHandler_t::GetKeyAltTitle(_TCHAR * pszKey, bool fAlwaysAsk,
										const _TCHAR * pszTitleMain, const _TCHAR * pszTitleDlg,
										HICON hIcon)
{
	if (!m_pfGetKeyAltTitle)
		Init();
	return m_pfGetKeyAltTitle(pszKey, fAlwaysAsk, pszTitleMain, pszTitleDlg, hIcon);
}
