#include <stdio.h>

#include "DiskManager.h"
#include "Resource.h"
#include "LogFile.h"
#include "UsefulStuff.h"
#include "Globals.h"

struct DiskDetail
{
	const char * m_kpszNiceName;
	const char * m_pszVolumeLabel;
};

#include "ConfigDisks.cpp"

static const int kctDiskDetails = sizeof(DiskDetails) / sizeof(DiskDetails[0]);

DiskManager_t::DiskManager_t()
{
	char szCurrentPath[MAX_PATH];

	// Get current drive root:
	GetModuleFileName(NULL, szCurrentPath, MAX_PATH);
	strncpy(m_szCurrentDriveRoot, szCurrentPath, 3);
	m_szCurrentDriveRoot[3] = 0;
	g_Log.Write("Current drive root: %s", m_szCurrentDriveRoot);

	// Get current path:
	strcpy(m_szCurrentPath, szCurrentPath);
	char * ch = strrchr(m_szCurrentPath, '\\');
	if (ch)
		*(ch + 1) = 0;
	g_Log.Write("Current path: %s", m_szCurrentPath);
}

DiskManager_t::~DiskManager_t()
{
}

int DiskManager_t::CurrentCd()
{
	const int kctchVolume = 128;
	char szVolume[kctchVolume];
	GetVolumeInformation(m_szCurrentDriveRoot, szVolume, kctchVolume, NULL, NULL, NULL, NULL, 0);

	g_Log.Write("Volume label for '%s' is '%s'", m_szCurrentDriveRoot, szVolume);

	for (int i = 0; i < kctDiskDetails; i++)
		if (stricmp(szVolume, DiskDetails[i].m_pszVolumeLabel) == 0)
			return i;

	return -1;
}

// Make sure the specified CD is inserted in the drive.
// If fAllowNoCd is true, then as long as none of the others in the set is inserted, that will
// do.
// If pszSkipMsgProduct is not NULL, then an incorrect disk message will include an option to
// skip the specified product.
int DiskManager_t::CheckCdPresent(int iCd, bool fAllowNoCd, const char * pszSkipMsgProduct)
{
	int iCurrentCd = CurrentCd();
	if (iCurrentCd == iCd)
		knCorrectCdAlready;

	if (fAllowNoCd && iCurrentCd == -1)
		return knNoCd;

	while (iCurrentCd != iCd)
	{
		// We don't have the correct CD:
		g_Log.Write("Disk %d is inserted - asking for disk %d.\n", iCurrentCd, iCd);

		char * pszMsg = new_sprintf(FetchString(IDC_MESSAGE_INSERT), DiskDetails[iCd].m_kpszNiceName);
		if (pszSkipMsgProduct)
			new_sprintf_concat(pszMsg, 2, FetchString(IDC_MESSAGE_SKIPORCANCEL), pszSkipMsgProduct);

		int nResult = MessageBox(NULL, pszMsg, g_pszTitle,
			pszSkipMsgProduct ? MB_ABORTRETRYIGNORE : MB_OKCANCEL);
		delete[] pszMsg;
		pszMsg = NULL;

		switch (nResult)
		{
		case IDABORT:
		case IDCANCEL:
			// Confirm quit:
			if (MessageBox(NULL, FetchString(IDC_MESSAGE_CONFIRM_QUIT_GENERAL), g_pszTitle,
				MB_YESNO) == IDYES)
			{
				g_Log.Write("User opted to quit.");
				return knUserQuit;
			}
			break;
		case IDRETRY:
		case IDOK:
			// Go around while-loop again...
			break;
		case IDIGNORE:
			// Confirm skip:
			if (MessageBox(NULL, FetchString(IDC_MESSAGE_CONFIRM_SKIP), g_pszTitle,
				MB_YESNO) == IDYES)
			{
				g_Log.Write("User opted to skip product.");
				return knUserSkip;
			}
			break;
		}
		iCurrentCd = CurrentCd();
	} // Try again

	return knCorrectCdFinally;
}

int DiskManager_t::EnsureCdForFile(const char * pszFile, int iCd, const char * pszSkipMsgProduct)
{
	int iCurrentCd = CurrentCd();

	// See if file can be found from current directory:
	const int knLen = MAX_PATH;
	char szPath[knLen];
	strcpy(szPath, m_szCurrentPath);
	strcat(szPath, pszFile);
	if (GetFileAttributes(szPath) != INVALID_FILE_ATTRIBUTES)
	{
		if (iCurrentCd == iCd)
			return knCorrectCdAlready;
		else if (iCurrentCd == -1)
			return knFileFoundWrongCd;
	}
	int nResult = CheckCdPresent(iCd, false, pszSkipMsgProduct);
	if (nResult == knCorrectCdFinally || nResult == knCorrectCdAlready)
		if (GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES)
			return knFileNotFoundCorrectCd;
	return nResult;
}

// Make a new string containing a concatenation of the current path and the given path.
char * DiskManager_t::NewFullPath(const char * pszRelativePath)
{
	return new_sprintf("%s%s", m_szCurrentPath, pszRelativePath);
}
