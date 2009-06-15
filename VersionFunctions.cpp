// This file is included in other C++ files, as needed, so that other definitions and includes
// can be tailored to the particular project using it.

// Parses a (potentially) 4-part version number string (e.g. 2.0.2600.1106) and returns a 64-bit
// integer comprising all 4 parts, so a simple comparison can be made (e.g. 0x000200000A280452).
__int64 GetHugeVersion(const _TCHAR * pszVersion)
{
	if (!pszVersion)
		return 0;

	_TCHAR * pszVersionCopy = my_strdup(pszVersion);
	__int64 nValue = 0;
	_TCHAR * pszContext;

	_TCHAR * pszNextSegment = _tcstok_s(pszVersionCopy, _T("."), &pszContext);
	int ctSegments = 1;
	while (pszNextSegment && ctSegments <= 4)
	{
		__int64 nSegment = _tstoi(pszNextSegment);
		nValue |= (nSegment << ((4 - ctSegments) * 16));
		ctSegments++;
		pszNextSegment = _tcstok_s(NULL, _T("."), &pszContext);
	}
	delete[] pszVersionCopy;
	pszVersionCopy = NULL;

	return nValue;
}

// Splits a 64-bit version number into the 4 constituent parts as a text string.
// Caller must delete[] the return value.
_TCHAR * GenVersionText(__int64 nHugeVersion)
{
	return new_sprintf(_T("%d.%d.%d.%d"), int(nHugeVersion >> 48), 
		int((nHugeVersion >> 32) & 0xFFFF), int((nHugeVersion >> 16) & 0xFFFF),
		int(nHugeVersion & 0xFFFF));
}


// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
// Uses inclusive test: candidate version is allowed to equal range extemities.
bool VersionInRange(__int64 nVersion, const _TCHAR * pszMinVersion, 
					const _TCHAR * pszMaxVersion)
{
	// Create 64-bit numbers from the min and max arguments:
	__int64 nMinVersion;
	__int64 nMaxVersion;

	if (pszMinVersion)
		nMinVersion = GetHugeVersion(pszMinVersion);
	else
		nMinVersion = 0;

	if (pszMaxVersion)
		nMaxVersion = GetHugeVersion(pszMaxVersion);
	else
		nMaxVersion = 0x7FFFFFFFFFFFFFFF;

	// See if the discovered version is within range:
	return (nVersion >= nMinVersion && nVersion <= nMaxVersion);
}

// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
// Uses inclusive test: candidate version is allowed to equal range extemities.
bool VersionInRange(const _TCHAR * pszVersion, const _TCHAR * pszMinVersion, 
					const _TCHAR * pszMaxVersion)
{
	if (!pszVersion)
		return false;

	// Create 64-bit number from the pszVersion argument:
	__int64 nVersion = GetHugeVersion(pszVersion);

	return VersionInRange(nVersion, pszMinVersion, pszMaxVersion);
}

// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
// Uses exclusive test: candidate version is not allowed to equal range extemities.
bool VersionInRangeEx(__int64 nVersion, const _TCHAR * pszMinVersion, 
					const _TCHAR * pszMaxVersion)
{
	// Create 64-bit numbers from the min and max arguments:
	__int64 nMinVersion;
	__int64 nMaxVersion;

	if (pszMinVersion)
		nMinVersion = GetHugeVersion(pszMinVersion);
	else
		nMinVersion = 0;

	if (pszMaxVersion)
		nMaxVersion = GetHugeVersion(pszMaxVersion);
	else
		nMaxVersion = 0x7FFFFFFFFFFFFFFF;

	// See if the discovered version is within range:
	return (nVersion > nMinVersion && nVersion < nMaxVersion);
}

// Returns true if the given version number is within the given range.
// If pszMinVersion is NULL, any range up to (and including) pszMaxVersion will do.
// If pszMaxVersion is NULL, any range above (and including) pszMinVersion will do.
// If pszMinVersion is NULL and pszMaxVersion is NULL, any version will do.
// Uses exclusive test: candidate version is not allowed to equal range extemities.
bool VersionInRangeEx(const _TCHAR * pszVersion, const _TCHAR * pszMinVersion, 
					const _TCHAR * pszMaxVersion)
{
	if (!pszVersion)
		return false;

	// Create 64-bit number from the pszVersion argument:
	__int64 nVersion = GetHugeVersion(pszVersion);

	return VersionInRangeEx(nVersion, pszMinVersion, pszMaxVersion);
}

// Gets the version of the given file.
// If file does not exist or some other error occurs, returns false; otherwise true.
bool GetFileVersion(const _TCHAR * pszFilePath, __int64 & nVersion)
{
	bool fResult = false;

	// Find out how much space we need to store the version information block.
	DWORD dwVersionInfoSize;
	DWORD dwZero; // Temp variable.
	dwVersionInfoSize = GetFileVersionInfoSize(pszFilePath, &dwZero);
	if (dwVersionInfoSize)
	{
		//Allocate space to store the version info.
		void * pVersionInfo = malloc(dwVersionInfoSize);

		// Use GetFileVersionInfo to copy the version info block into pVersion info.
		if (GetFileVersionInfo(pszFilePath, 0, dwVersionInfoSize, pVersionInfo))
		{
			// Use VerQueryValue to parse the version information data block and get a pointer
			// to the VS_FIXEDFILEINFO structure.
			VS_FIXEDFILEINFO * pFixedFileInfo;
			UINT nBytesReturned;
			if (VerQueryValue(pVersionInfo, _T("\\"), (void **)&pFixedFileInfo, &nBytesReturned))
			{
				nVersion = ((__int64(pFixedFileInfo->dwFileVersionMS)) << 32) |
					pFixedFileInfo->dwFileVersionLS;
				fResult = true;
			}
			free(pVersionInfo);
			pVersionInfo = NULL;
		}
	}
	return fResult;
}
