// Tests for presence of given Service Pack on Windows 2000.
// If current operating system is not even Windows 2000, returns true anyway.
bool TestWin2kSpPresence(const char * pszMinVersion, const char * pszMaxVersion,
						 const char * /*pszCriticalFile*/)
{
	OSVERSIONINFOEX osvi;

	// Get operating system info:
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)(&osvi)) == 0)
	{
		// We'll assume the error is because we can't handle this call, therefore we can't be
		// running Win 2000:
		return true;
	}

	g_Log.Write("Found Windows version %d.%d, Service Pack %d", osvi.dwMajorVersion, 
		osvi.dwMinorVersion, (int)(osvi.wServicePackMajor));

	if (osvi.dwMajorVersion == 5)
	{
		if (osvi.dwMinorVersion == 0)
		{
			// We're running on 2000, so check the service pack number:
			if (!pszMinVersion)
				pszMinVersion = "0";
			if (!pszMaxVersion)
				pszMaxVersion = "32767";

			if (osvi.wServicePackMajor < atoi(pszMinVersion) || 
				osvi.wServicePackMajor > atoi(pszMaxVersion))
			{
				return false;
			}
		}
	}
	// Either the Service Pack is present, or we're not running Win 2000 anyway:
	return true;
}
