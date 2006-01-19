/*
	Copyright (c) 2003 SIL International.

	Master installer wrapper for Windows Installer functions.

	Using this module allows delayed dynamic linking to msi.dll, so we can at least start up the
	master installer without having Windows Installer present.
*/

#pragma once

#include <windows.h>
#include <msi.h>

class WindowsInstallerWrapper
{
public:
	WindowsInstallerWrapper();
	~WindowsInstallerWrapper();
	bool Init();

	INSTALLSTATE MsiQueryProductState(LPCSTR pszProduct);
	INSTALLSTATE MsiQueryFeatureState(LPCSTR pszProduct, LPCSTR pszFeature);
	UINT MsiGetProductInfo(LPCTSTR szProduct, LPCTSTR szProperty, LPTSTR lpValueBuf,
		DWORD * pcchValueBuf);

protected:
	HMODULE hmodMsi;
	typedef INSTALLSTATE (WINAPI * MsiQueryProductStateFn)(LPCSTR pszProduct);
	MsiQueryProductStateFn _MsiQueryProductState;
	typedef INSTALLSTATE (WINAPI * MsiQueryFeatureStateFn)(LPCSTR pszProduct,
		LPCSTR pszFeature);
	MsiQueryFeatureStateFn _MsiQueryFeatureState;
	typedef UINT (WINAPI * MsiGetProductInfoFn)(LPCTSTR szProduct, LPCTSTR szProperty,
		LPTSTR lpValueBuf, DWORD * pcchValueBuf);
	MsiGetProductInfoFn _MsiGetProductInfo;
};

extern WindowsInstallerWrapper WindowsInstaller;
