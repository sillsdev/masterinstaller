#pragma once

#include <tchar.h>

// Because of the lack of consistency in registry settings between .NET versions,
// the following data is used to specify what registry settings should be checked
// for what releases. Note that we're ignoring versions earlier than 1.1, as
// those made it impossible to reduce data down to this structure:

struct RegData
{
	const _TCHAR * MasterInstallerTagRoot;
	const _TCHAR * RegKey;
	const _TCHAR * InstallValueName;
	const DWORD InstallValueData;
	const _TCHAR * ServicePackValueName;
};

const struct RegData DotNetRegData[] =
{
	{ _T("1.1"), _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v1.1.4322"), _T("Install"), 1, _T("SP") },
	{ _T("2.0"), _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"), _T("Install"), 1, _T("SP") },
	{ _T("3.0"), _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v3.0"), _T("Install"), 1, _T("SP") },
	{ _T("3.5"), _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v3.5"), _T("Install"), 1, _T("SP") },
	{ _T("4 Client"), _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client"), _T("Install"), 1, NULL },
	{ _T("4 Full"), _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"), _T("Install"), 1, NULL },
};

// Tests for presence of .NET runtime
bool TestDotNetPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion,
						SoftwareProduct * /*Product*/)
{
	// Prerequisite: pszMinVersion is one of the tag root values (first column) listed in the
	// DotNetRegData structure above, with an optional " SPn" string, where n is a service
	// pack number. This represents the minimum .NET version that has to be present to return
	// true.
	// pszMaxVersion must be empty or NULL; its value is not used.

	// Test pszMinVersion has a value:
	if (pszMinVersion == NULL || pszMinVersion[0] == 0)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("TestDotNetPresence was given no value for pszMinVersion"));
#endif
		return false;
	}

	// Test pszMaxVersion has no value:
	if (pszMaxVersion != NULL && pszMaxVersion[0] != 0)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("TestDotNetPresence %s was given a redundant value for pszMaxVersion: %s"), pszMinVersion, pszMaxVersion);
#endif
	}

	_TCHAR * versionTag = my_strdup(pszMinVersion);
	int servicePack = -1;

	// Detect if version tag contains " SPn" at end:
	int tagLen = _tcslen(versionTag);
	if (tagLen > 4)
		if (versionTag[tagLen - 4] == _TCHAR(' '))
			if (versionTag[tagLen - 3] == _TCHAR('S'))
				if (versionTag[tagLen - 2] == _TCHAR('P'))
					if (isdigit(versionTag[tagLen - 1]))
					{
						// It does, so record service pack integer and truncate tag:
						servicePack = _tstoi(&versionTag[tagLen - 1]);
						versionTag[tagLen - 4] = 0;
					}

	// Find position of versionTag in DotNetRegData structure:
	int numIndexes = sizeof(DotNetRegData) / sizeof(DotNetRegData[0]);
	int tagIndex;
	for (tagIndex = numIndexes - 1; tagIndex >= 0; tagIndex--)
		if (_tcscmp(DotNetRegData[tagIndex].MasterInstallerTagRoot, versionTag) == 0)
			break;

	if (tagIndex < 0)
	{
#if !defined NOLOGGING
		g_Log.Write(_T("TestDotNetPresence was given unrecognized .NET version tag '%s'"), pszMinVersion);
#endif
		return false;
	}

	// Detect if the given version of .NET, or a higher one, is installed:
	int index;
	for (index = tagIndex; index < numIndexes; index++)
	{
		const _TCHAR * RegKey = DotNetRegData[index].RegKey;
		const _TCHAR * InstallValueName = DotNetRegData[index].InstallValueName;
		DWORD InstallValueData = DotNetRegData[index].InstallValueData;
		const _TCHAR * ServicePackValueName = DotNetRegData[index].ServicePackValueName;

		_TCHAR * InstallValueString = NewRegString(HKEY_LOCAL_MACHINE, RegKey, InstallValueName);
		if (InstallValueString == NULL)
			continue;
		if (*((DWORD *)InstallValueString) != InstallValueData)
		{
			delete[] InstallValueString;
			continue;
		}
		delete[] InstallValueString;

		if (servicePack > -1)
		{
			// If anything fails in this section, we will look for the next
			// version up of .NET, so no need to worry about service packs after this point.

			if (ServicePackValueName == NULL)
			{
				servicePack = -1;
				continue;
			}
			_TCHAR * ServicePackString = NewRegString(HKEY_LOCAL_MACHINE, RegKey, ServicePackValueName);
			if (ServicePackString == NULL)
			{
				servicePack = -1;
				continue;
			}
			if (*((DWORD *)ServicePackString) < (DWORD)servicePack)
			{
				delete[] ServicePackString;
				servicePack = -1;
				continue;
			}
			delete[] ServicePackString;
		}

		return true;
	}

	return false;
}
