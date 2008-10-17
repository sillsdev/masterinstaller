// TestPresence Web Control.cpp : Implementation of CTestPresenceWebControlApp and DLL registration.

#include "stdafx.h"
#include "TestPresence Web Control.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "comcat.h"
#include "strsafe.h"
#include "objsafe.h"
 
// CLSID_SafeItem - Necessary for safe ActiveX control
// Id taken from IMPLEMENT_OLECREATE_EX function in xxxCtrl.cpp
 
const CATID CLSID_SafeItem =
//{ 0xd40f2c18, 0x4fc2, 0x4351,{ 0x95, 0x68, 0x3, 0xa5, 0xc1, 0xd1, 0x21, 0x60}};
//{ 0x5B4F948B, 0xDDB7, 0x47f3,{ 0xA2, 0x1B, 0x6B, 0xF3, 0xC7, 0x6C, 0x3F, 0xCD}};
//{ 0x9B721D13, 0x6A40, 0x4582,{ 0x80, 0x30, 0x26, 0x02, 0xB7, 0x72, 0x0A, 0x28}};
{ 0x13642917, 0xACB7, 0x4791,{ 0x9C, 0x64, 0xC3, 0xEB, 0x9D, 0x9D, 0xCE, 0x9D}};

// HRESULT CreateComponentCategory - Used to register ActiveX control as safe
 
HRESULT CreateComponentCategory(CATID catid, wchar_t *catDescription)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (FAILED(hr))
        return hr;
 
    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered.
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english
    size_t len;
    // Make sure the provided description is not too long.
    // Only copy the first 127 _TCHARacters if it is.
    // The second parameter of StringCchLength is the maximum
    // number of _TCHARacters that may be read into catDescription.
    // There must be room for a NULL-terminator. The third parameter
    // contains the number of _TCHARacters excluding the NULL-terminator.
    hr = StringCchLength(catDescription, STRSAFE_MAX_CCH, &len);
    if (SUCCEEDED(hr))
        {
        if (len>127)
          {
            len = 127;
          }
        }   
    else
        {
          // TODO: Write an error handler;
        }
    // The second parameter of StringCchCopy is 128 because you need 
    // room for a NULL-terminator.
    hr = StringCchCopy(catinfo.szDescription, len + 1, catDescription);
    // Make sure the description is null terminated.
    catinfo.szDescription[len + 1] = '\0';
 
    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();
 
    return hr;
}
 
// HRESULT RegisterCLSIDInCategory -
//      Register your component categories information
 
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
// Register your component categories information.
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Register this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }
 
    if (pcr != NULL)
        pcr->Release();
            
    return hr;
}
 
// HRESULT UnRegisterCLSIDInCategory - Remove entries from the registry
 
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
 
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Unregister this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }
 
    if (pcr != NULL)
        pcr->Release();
 
    return hr;
}

CTestPresenceWebControlApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xAE8C2C7F, 0x401F, 0x4BAD, { 0x8F, 0x9F, 0x4E, 0xA4, 0x6E, 0x65, 0xC2, 0xE3 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 1;



// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	HRESULT hr;    // HResult used by Safety Functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	// Mark the control as safe for initializing:
	hr = CreateComponentCategory(CATID_SafeForInitializing, 
		 L"Controls safely initializable from persistent data!");
	if (FAILED(hr))
	  return hr;

	hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
		 CATID_SafeForInitializing);
	if (FAILED(hr))
		return hr;

	// Mark the control as safe for scripting:
	hr = CreateComponentCategory(CATID_SafeForScripting, 
								 L"Controls safely scriptable!");
	if (FAILED(hr))
		return hr;

	hr = RegisterCLSIDInCategory(CLSID_SafeItem, 
						CATID_SafeForScripting);
	if (FAILED(hr))
		return hr;

	return NOERROR;
}



// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	HRESULT hr;    // HResult used by Safety Functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	// Remove entries from the registry:
	hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, 
					 CATID_SafeForInitializing);
	if (FAILED(hr))
	  return hr;

	hr=UnRegisterCLSIDInCategory(CLSID_SafeItem, 
						CATID_SafeForScripting);
	if (FAILED(hr))
	  return hr;

	return NOERROR;
}
