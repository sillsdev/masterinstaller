// TestPresenceCtrl.cpp : Implementation of the CTestPresenceCtrl ActiveX Control class.

#include "stdafx.h"
#include "TestPresence Web Control.h"
#include "TestPresenceCtrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CTestPresenceCtrl, COleControl)

// Dispatch map
BEGIN_DISPATCH_MAP(CTestPresenceCtrl, COleControl)
	DISP_FUNCTION_ID(CTestPresenceCtrl, "TestPresence", dispidTestPresence, TestPresence, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION_ID(CTestPresenceCtrl, "RunningWin2KOrBetter", dispidRunningWin2KOrBetter, RunningWin2KOrBetter, VT_BOOL, VTS_NONE)
END_DISPATCH_MAP()

// Initialize class factory and guid
IMPLEMENT_OLECREATE_EX(CTestPresenceCtrl, "SIL.TestPresenceCtrl.1",
	0xd40f2c18, 0x4fc2, 0x4351, 0x95, 0x68, 0x3, 0xa5, 0xc1, 0xd1, 0x21, 0x60)

// Type library ID and version
IMPLEMENT_OLETYPELIB(CTestPresenceCtrl, _tlid, _wVerMajor, _wVerMinor)

// Interface IDs
const IID BASED_CODE IID_DTestPresenceWebControl =
		{ 0x3AEF8419, 0xE198, 0x417B, { 0xBD, 0xDF, 0xE0, 0x24, 0xFE, 0xFF, 0x33, 0x1D } };

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


// Include some functions from the main Setup project, which are called by some TestPresence
// functions:
#include "StringFunctions.cpp"
#include "VersionFunctions.cpp"
// Include a minimal version of the Windows Installer wrapper, which is used by some
// TestPresence functions:
#include "WIWrapperMinimal.cpp"

// Definition of a TestPresence function:
typedef bool (* pfnAuxTestPresence)(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
									const _TCHAR * pszCriticalFile);
// Definition of the data structure holding essential data to test the presence of any
// product:
typedef struct
{
	const TCHAR * Tag;
	const TCHAR * ProductCode;
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

VARIANT_BOOL CTestPresenceCtrl::RunningWin2KOrBetter(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// See if we're running on less than Windows 2000:
	OSVERSIONINFO OSversion;
	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&OSversion);
	if (OSversion.dwMajorVersion < 5)
		return VARIANT_FALSE;

	return VARIANT_TRUE;
}
