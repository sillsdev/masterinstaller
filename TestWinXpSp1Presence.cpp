// Tests for presence of Windows XP Service Pack 1.
// If current operating system is not even Windows XP, returns true anyway.
bool TestWinXpSp1Presence(const char * pszMinVersion, const char * pszMaxVersion, const char * /*pszCriticalFile*/)
{
	OSVERSIONINFOEX osvi;

	// Get operating system info:
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)(&osvi)) == 0)
	{
		// We'll assume the error is because we can't handle this call, therefore we can't be
		// running Win XP:
		return true;
	}

	g_Log.Write("Found Windows version %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);

	if (osvi.dwMajorVersion == 5)
	{
		if (osvi.dwMinorVersion == 1)
		{
			// We're running on XP, so check the service pack number:
			if (osvi.wServicePackMajor < 1)
				return false;
		}
	}
	// Either Service Pack 1 or later is present, or we're not running Win XP anyway:
	return true;
}
