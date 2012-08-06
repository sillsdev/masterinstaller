#pragma once

// TestPresenceCtrl.h : Declaration of the CTestPresenceCtrl ActiveX Control class.


// CTestPresenceCtrl : See TestPresenceCtrl.cpp for implementation.

class CTestPresenceCtrl : public COleControl
{
	DECLARE_DYNCREATE(CTestPresenceCtrl)

// Implementation
protected:
	DECLARE_OLECREATE_EX(CTestPresenceCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CTestPresenceCtrl)      // GetTypeInfo
	DECLARE_OLECTLTYPE(CTestPresenceCtrl)		// Type name and misc status

// Dispatch maps
	DECLARE_DISPATCH_MAP()

// Dispatch IDs
public:
	enum {
		dispidGetOsVersionString = 2L,
		dispidTestPresence = 1L
	};
protected:
	VARIANT_BOOL TestPresence(LPCTSTR Tag, LPCTSTR MinVersion, LPCTSTR MaxVersion);
	BSTR GetOsVersionString(void);
};
