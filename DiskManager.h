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
	};
	int CheckCdPresent(int iCd, bool fAllowNoCd = false, const char * pszSkipMsgProduct = NULL);
	int EnsureCdForFile(const char * pszFile, int iCd, const char * pszSkipMsgProduct);
	char * NewFullPath(const char * pszRelativePath);

protected:
	char m_szCurrentDriveRoot[4];
	char m_szCurrentPath[MAX_PATH];

	int CurrentCd();
};

// Global declaration:
extern DiskManager_t g_DiskManager;