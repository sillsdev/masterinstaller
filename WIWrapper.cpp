/*
	Copyright (c) 2003 SIL International.

	Master installer wrapper for Windows Installer functions.

	Using this module allows delayed dynamic linking to msi.dll, so we can at least start up the
	master installer without having Windows Installer present.
*/

#include <windows.h>
#include <tchar.h>

#include "WIWrapper.h"
#include "UsefulStuff.h"
#include "Globals.h"

// Global instance:
WindowsInstallerWrapper WindowsInstaller;


// Returns a dynamically allocated string containing the path of the directory where the Windows
// Installer .exe file is located.
// Returns NULL is there is an error (such as Windows Installer not installed).
// Caller must delete[] the returned string.
_TCHAR * GetInstallerLocation()
{
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"), NULL, KEY_READ, &hKey);

	// we don't proceed unless the call above succeeds
	if (lResult != ERROR_SUCCESS)
		return NULL;

	// Get length of required buffer:
	DWORD dwBufLen = 0;
	lResult = RegQueryValueEx(hKey, _T("InstallerLocation"), NULL, NULL, NULL, &dwBufLen);
	if (dwBufLen == 0)
		return NULL;

	_TCHAR * pszLocation = new _TCHAR [dwBufLen];
	lResult = RegQueryValueEx(hKey, _T("InstallerLocation"), NULL, NULL, (LPBYTE)pszLocation,
			&dwBufLen);
	RegCloseKey(hKey);
	hKey = NULL;

	// If we receive an error, quit:
	if (lResult != ERROR_SUCCESS)
	{
		delete[] pszLocation;
		return NULL;
	}

	return pszLocation;
}

WindowsInstallerWrapper::WindowsInstallerWrapper()
{
	hmodMsi = NULL;
	_MsiQueryProductState = NULL;
	_MsiQueryFeatureState = NULL;
	_MsiGetProductInfo = NULL;
}

WindowsInstallerWrapper::~WindowsInstallerWrapper()
{
	if (hmodMsi)
		FreeLibrary(hmodMsi);
}

bool WindowsInstallerWrapper::Init()
// Attempt to load msi.dll. If successfull, pointers to the two functions we use will be set up.
{
	if (hmodMsi)
		return true;

	_TCHAR * pszInstallerLocation = GetInstallerLocation();
	if (!pszInstallerLocation)
		return false; // Windows Installer not present.

	// Form the full path to the DLL we want:
	_TCHAR * pszLibraryPath = new_sprintf(_T("%s\\msi.dll"), pszInstallerLocation);
	delete[] pszInstallerLocation;
	pszInstallerLocation = NULL;

	// Try to load the DLL:
	hmodMsi = LoadLibrary(pszLibraryPath);
	delete[] pszLibraryPath;
	pszLibraryPath = NULL;

	// Check if we were successful:
	if (!hmodMsi)
		return false;

	// Now get pointers to the functions we want to use:
#ifdef UNICODE
	_MsiQueryProductState = (MsiQueryProductStateFn)GetProcAddress(hmodMsi,
		"MsiQueryProductStateW");
	_MsiQueryFeatureState = (MsiQueryFeatureStateFn)GetProcAddress(hmodMsi,
		"MsiQueryFeatureStateW");
	_MsiGetProductInfo = (MsiGetProductInfoFn)GetProcAddress(hmodMsi,
		"MsiGetProductInfoW");
#else
	_MsiQueryProductState = (MsiQueryProductStateFn)GetProcAddress(hmodMsi,
		"MsiQueryProductStateA");
	_MsiQueryFeatureState = (MsiQueryFeatureStateFn)GetProcAddress(hmodMsi,
		"MsiQueryFeatureStateA");
	_MsiGetProductInfo = (MsiGetProductInfoFn)GetProcAddress(hmodMsi,
		"MsiGetProductInfoA");
#endif

	if (!_MsiQueryProductState || !_MsiQueryFeatureState || !_MsiGetProductInfo)
	{
		// Now we're in trouble! The required functions aren't in the DLL.
		MessageBox(NULL,
			_T("Error - Windows Installer module does not contain required functions."), g_pszTitle,
			MB_ICONSTOP | MB_OK);
		return false;
	}
	return true;
}

INSTALLSTATE WindowsInstallerWrapper::MsiQueryProductState(LPCTSTR pszProduct)
// Use our internal pointer to the msi.dll version of this function.
{
	if (!hmodMsi)
		Init();

	if (!_MsiQueryProductState)
		return INSTALLSTATE_UNKNOWN;

	return _MsiQueryProductState(pszProduct);
}

INSTALLSTATE WindowsInstallerWrapper::MsiQueryFeatureState(LPCTSTR pszProduct,
														   LPCTSTR pszFeature)
// Use our internal pointer to the msi.dll version of this function.
{
	if (!hmodMsi)
		Init();

	if (!_MsiQueryFeatureState)
		return INSTALLSTATE_UNKNOWN;

	return _MsiQueryFeatureState(pszProduct, pszFeature);
}

UINT WindowsInstallerWrapper::MsiGetProductInfo(LPCTSTR szProduct, LPCTSTR szProperty,
												LPTSTR lpValueBuf, DWORD* pcchValueBuf)
// Use our internal pointer to the msi.dll version of this function.
{
	if (!hmodMsi)
		Init();

	if (!_MsiGetProductInfo)
		return ERROR_UNKNOWN_PRODUCT;

	return _MsiGetProductInfo(szProduct, szProperty, lpValueBuf, pcchValueBuf);
}
