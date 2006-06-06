/*
	Copyright (c) 2003 SIL International.

	Master installer wrapper for Windows Installer functions.

	Using this module allows delayed dynamic linking to msi.dll, so we can at least start up the
	master installer without having Windows Installer present.
*/
#include <windows.h>
#include "WIWrapper.h"
#include "UsefulStuff.h"
#include "Globals.h"

// Global instance:
WindowsInstallerWrapper WindowsInstaller;

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

	char * pszInstallerLocation = GetInstallerLocation();
	if (!pszInstallerLocation)
		return false; // Windows Installer not present.

	// Form the full path to the DLL we want:
	char * pszLibraryPath = new_sprintf("%s\\msi.dll", pszInstallerLocation);
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
	_MsiQueryProductState = (MsiQueryProductStateFn)GetProcAddress(hmodMsi,
		"MsiQueryProductStateA");
	_MsiQueryFeatureState = (MsiQueryFeatureStateFn)GetProcAddress(hmodMsi,
		"MsiQueryFeatureStateA");
	_MsiGetProductInfo = (MsiGetProductInfoFn)GetProcAddress(hmodMsi,
		"MsiGetProductInfoA");

	if (!_MsiQueryProductState || !_MsiQueryFeatureState || !_MsiGetProductInfo)
	{
		// Now we're in trouble! The required functions aren't in the DLL.
		MessageBox(NULL,
			"Error - Windows Installer module does not contain required functions.", g_pszTitle,
			MB_ICONSTOP | MB_OK);
		return false;
	}
	return true;
}

INSTALLSTATE WindowsInstallerWrapper::MsiQueryProductState(LPCSTR pszProduct)
// Use our internal pointer to the msi.dll version of this function.
{
	if (!hmodMsi)
		Init();

	if (!_MsiQueryProductState)
		return INSTALLSTATE_UNKNOWN;

	return _MsiQueryProductState(pszProduct);
}

INSTALLSTATE WindowsInstallerWrapper::MsiQueryFeatureState(LPCSTR pszProduct, LPCSTR pszFeature)
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
