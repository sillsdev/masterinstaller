#pragma once

#include <windows.h>

class DiskManager_t
{
public:
	DiskManager_t();
	~DiskManager_t();
	
	enum
	{
		knUserQuit,
		knUserSkip,
		knNoCd,
		knCorrectCdAlready,
		knCorrectCdFinally,
		knFileNotFoundCorrectCd,
		knFileFoundWrongCd,
		knFileOmitted,
	};
	int CheckCdPresent(int iCd, bool fAllowNoCd = false, const _TCHAR * pszSkipMsgProduct = NULL);
	int EnsureCdForFile(const _TCHAR * pszFile, int iCd, const _TCHAR * pszSkipMsgProduct);
	_TCHAR * NewFullPath(const _TCHAR * pszRelativePath);

protected:
	_TCHAR m_szCurrentDriveRoot[4];
	_TCHAR m_szCurrentPath[MAX_PATH];

	int CurrentCd();
};

// Global declaration:
extern DiskManager_t g_DiskManager;
