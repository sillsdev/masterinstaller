#pragma once

#include <tchar.h>

// Tests for presence of given Service Pack on Windows Vista.
// If current operating system is not even Windows Vista, returns true anyway.
bool TestWinVistaSpPresence(const TCHAR * pszMinVersion, const TCHAR * pszMaxVersion,
						 const TCHAR * /*pszCriticalFile*/)
{
	OSVERSIONINFOEX osvi;

	// Get operating system info:
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)(&osvi)) == 0)
	{
		// We'll assume the error is because we can't handle this call, therefore we can't be
		// running Windows Vista:
		return true;
	}

#if !defined NOLOGGING
	g_Log.Write(_T("Found Windows version %d.%d, Service Pack %d"), osvi.dwMajorVersion, osvi.dwMinorVersion, (int)(osvi.wServicePackMajor));
#endif

	if (osvi.dwMajorVersion == 6)
	{
		if (osvi.dwMinorVersion == 0)
		{
			// We're running on Vista, so check the service pack number:
			if (!pszMinVersion)
				pszMinVersion = _T("0");
			if (!pszMaxVersion)
				pszMaxVersion = _T("32767");

			if (osvi.wServicePackMajor < _tstoi(pszMinVersion) || 
				osvi.wServicePackMajor > _tstoi(pszMaxVersion))
			{
				return false;
			}
		}
	}
	// Either the Service Pack is present, or we're not running Windows Vista anyway:
	return true;
}
