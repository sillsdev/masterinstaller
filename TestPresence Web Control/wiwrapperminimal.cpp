// This file is included in other C++ files, as needed, so that other definitions and includes
// can be tailored to the particular project using it.

#include <msi.h>

// Wrapper class for Windows Installer. Does not contain much functionality, but allows
// a program to contain calls to Windows Installer, and check at runtime if Windows Installer
// is present, without losing control to the MS runtime error code.
class WindowsInstaller_t
{
public:
	WindowsInstaller_t();
	~WindowsInstaller_t();
	INSTALLSTATE MsiQueryProductState(LPCTSTR pszProduct);
	const TCHAR * GetLocation() { return m_pszLocation; }

protected:
	HMODULE m_hmodMsi;
	typedef INSTALLSTATE (WINAPI * MsiQueryProductStateFn)(LPCTSTR pszProduct);
	MsiQueryProductStateFn _MsiQueryProductState;
	TCHAR * m_pszLocation;
};

WindowsInstaller_t::WindowsInstaller_t()
{
	m_hmodMsi = NULL;
	_MsiQueryProductState = NULL;
	LONG lResult;
	HKEY hKey = NULL;

	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"), NULL, KEY_READ, &hKey);

	// we don't proceed unless the call above succeeds
	if (lResult != ERROR_SUCCESS)
		return;

	// Get length of required buffer:
	DWORD dwBufLen = 0;
	lResult = RegQueryValueEx(hKey, _T("InstallerLocation"), NULL, NULL, NULL, &dwBufLen);
	if (dwBufLen == 0)
		return;

	m_pszLocation = new _TCHAR [dwBufLen];
	lResult = RegQueryValueEx(hKey, _T("InstallerLocation"), NULL, NULL, (LPBYTE)m_pszLocation,
			&dwBufLen);
	RegCloseKey(hKey);
	hKey = NULL;

	// If we receive an error, quit:
	if (lResult != ERROR_SUCCESS)
	{
		delete[] m_pszLocation;
		m_pszLocation = NULL;
		return;
	}

	// Form the full path to the DLL we want:
	_TCHAR * pszLibraryPath = new_sprintf(_T("%s\\msi.dll"), m_pszLocation);

	// Try to load the DLL:
	m_hmodMsi = LoadLibrary(pszLibraryPath);
	delete[] pszLibraryPath;
	pszLibraryPath = NULL;

	// Check if we were successful:
	if (!m_hmodMsi)
		return;

	_MsiQueryProductState = (MsiQueryProductStateFn)GetProcAddress(m_hmodMsi,
		"MsiQueryProductStateW");
}

WindowsInstaller_t::~WindowsInstaller_t()
{
	if (m_hmodMsi)
		FreeLibrary(m_hmodMsi);
	delete[] m_pszLocation;
	m_pszLocation = NULL;

}

INSTALLSTATE WindowsInstaller_t::MsiQueryProductState(LPCTSTR pszProduct)
// Use our internal pointer to the msi.dll version of this function.
{
	if (!_MsiQueryProductState)
		return INSTALLSTATE_UNKNOWN;

	return _MsiQueryProductState(pszProduct);
}
WindowsInstaller_t WindowsInstaller;

TCHAR * GetInstallerLocation()
{
	return my_strdup(WindowsInstaller.GetLocation());
}
