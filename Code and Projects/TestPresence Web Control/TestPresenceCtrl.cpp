// TestPresenceCtrl.cpp : Implementation of the CTestPresenceCtrl ActiveX Control class.

#include "stdafx.h"

#include <tchar.h>

#include "TestPresence Web Control.h"
#include "TestPresenceCtrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CTestPresenceCtrl, COleControl)

// Dispatch map
BEGIN_DISPATCH_MAP(CTestPresenceCtrl, COleControl)
	DISP_FUNCTION_ID(CTestPresenceCtrl, "TestPresence", dispidTestPresence, TestPresence, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION_ID(CTestPresenceCtrl, "GetOsVersionString", dispidGetOsVersionString, GetOsVersionString, VT_BSTR, VTS_NONE)
END_DISPATCH_MAP()

// Initialize class factory and guid
//IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.1",
//	0xd40f2c18, 0x4fc2, 0x4351, 0x95, 0x68, 0x3, 0xa5, 0xc1, 0xd1, 0x21, 0x60)
//IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.2",
//	0x5B4F948B, 0xDDB7, 0x47f3, 0xA2, 0x1B, 0x6B, 0xF3, 0xC7, 0x6C, 0x3F, 0xCD)
//IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.3",
//	0x9B721D13, 0x6A40, 0x4582, 0x80, 0x30, 0x26, 0x02, 0xB7, 0x72, 0x0A, 0x28)
//IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.4",
//	0x13642917, 0xACB7, 0x4791, 0x9C, 0x64, 0xC3, 0xEB, 0x9D, 0x9D, 0xCE, 0x9D)
IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.5",
	0x3BF3E3EB, 0x97DB, 0x40d4, 0x93, 0x7E, 0x4E, 0x54, 0x8D, 0x1F, 0xCE, 0x90)


// Type library ID and version
IMPLEMENT_OLETYPELIB(CTestPresenceCtrl, _tlid, _wVerMajor, _wVerMinor)

// Interface IDs
const IID BASED_CODE IID_DTestPresenceWebControl =
//		{ 0x3AEF8419, 0xE198, 0x417B, { 0xBD, 0xDF, 0xE0, 0x24, 0xFE, 0xFF, 0x33, 0x1D } };
		{ 0xF1749565, 0xCE5C, 0x472c, { 0x80, 0x41, 0x8F, 0x48, 0x64, 0xC2, 0x4A, 0x57 } };

// Control type information
static const DWORD BASED_CODE _dwTestPresenceWebControlOleMisc =
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CTestPresenceCtrl, IDS_TESTPRESENCEWEBCONTROL, _dwTestPresenceWebControlOleMisc)


// CTestPresenceCtrl::CTestPresenceCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CTestPresenceCtrl
BOOL CTestPresenceCtrl::CTestPresenceCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_TESTPRESENCEWEBCONTROL,
			IDB_TESTPRESENCEWEBCONTROL,
			afxRegApartmentThreading,
			_dwTestPresenceWebControlOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

// Include some dummy declarations to get test methods to compile:
_TCHAR * NewGetExeFolder() { _TCHAR * p = new _TCHAR [1]; *p = 0; return p; }
_TCHAR * MakePath(const _TCHAR * pszFolder, const _TCHAR * pszFile, bool fQuoted = false);
extern _TCHAR * NewRegString(HKEY hKeyRoot, const _TCHAR * pszSubKey, const _TCHAR * pszValueName = NULL);
class SoftwareProduct
{
public:
	const _TCHAR * GetCriticalFile() { return _T(""); }
};

// Include some functions from the main Setup project, which are called by some TestPresence
// functions:
#include "StringFunctions.cpp"
#include "VersionFunctions.cpp"
// Include a minimal version of the Windows Installer wrapper, which is used by some
// TestPresence functions:
#include "WIWrapperMinimal.cpp"

// Definition of a TestPresence function:
typedef bool (* pfnAuxTestPresence)(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
									SoftwareProduct * Product);
// Definition of the data structure holding essential data to test the presence of any
// product:
typedef struct
{
	const _TCHAR * Tag;
	const _TCHAR * ProductCode;
	pfnAuxTestPresence TestPresence;
} Product_t;


// Add in all the TestPresence functions, automatically created by parsing the
// _Master Installer.xml file:
#include "ConfigTestMethods.cpp"

// CTestPresenceCtrl message handlers
VARIANT_BOOL CTestPresenceCtrl::TestPresence(LPCTSTR Tag, LPCTSTR MinVersion, LPCTSTR MaxVersion)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bool fFound = false;

	// Search for given tag in our product list:
	for (int i = 0 ; i < sizeof(Products) / sizeof(Products[0]); i++)
	{
		if (_tcscmp(Tag, Products[i].Tag) == 0)
		{
			// Found the product. See what TestPresence mechanism there is:
			if (Products[i].TestPresence)
				fFound = Products[i].TestPresence(MinVersion, MaxVersion, NULL);
			else if (Products[i].ProductCode)
			{
				switch (WindowsInstaller.MsiQueryProductState(Products[i].ProductCode))
				{
				case INSTALLSTATE_ADVERTISED:
				case INSTALLSTATE_LOCAL:
				case INSTALLSTATE_SOURCE:
				case INSTALLSTATE_DEFAULT:
					fFound = true;
				}
			}
			break;
		}
	}
	return fFound;
}

BSTR CTestPresenceCtrl::GetOsVersionString(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	OSVERSIONINFOEX OSversion;

	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	::GetVersionEx((LPOSVERSIONINFO)(&OSversion));

	const int cchBuf = 60;
	_TCHAR pszBuf[cchBuf];

	_stprintf_s(pszBuf, cchBuf, _T("%d.%d.%d.%d"), OSversion.dwMajorVersion,
		OSversion.dwMinorVersion, OSversion.wServicePackMajor, OSversion.wServicePackMinor);

	// We need to return a BSTR, so first make sure our version string is Unicode:
	wchar_t UnicodeVersion[cchBuf];
#ifndef UNICODE
	UnicodeVersion[0] = 0;
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszBuf, -1, UnicodeVersion, cchBuf);
#else
	wcscpy_s(UnicodeVersion, cchBuf, pszBuf);
#endif

	return (BSTR)SysAllocString(UnicodeVersion);
}
