#include <stdio.h>
#include <tchar.h>

#include "DiskManager.h"
#include "Resource.h"
#include "LogFile.h"
#include "UsefulStuff.h"
#include "Globals.h"

struct DiskDetail
{
	const _TCHAR * m_kpszNiceName;
	const _TCHAR * m_pszVolumeLabel;
};


#include "ConfigDisks.cpp"

static const int kctDiskDetails = sizeof(DiskDetails) / sizeof(DiskDetails[0]);


DiskManager_t::DiskManager_t()
{
	_TCHAR szCurrentPath[MAX_PATH];

	// Get current drive root:
	GetModuleFileName(NULL, szCurrentPath, MAX_PATH);
	for (int i = 0; i < 3; i++)
	 m_szCurrentDriveRoot[i] = szCurrentPath[i];
	m_szCurrentDriveRoot[3] = 0;
	g_Log.Write(_T("Current drive root: %s"), m_szCurrentDriveRoot);

	// Get current path:
	_tcscpy_s(m_szCurrentPath, MAX_PATH, szCurrentPath);
	_TCHAR * ch = _tcsrchr(m_szCurrentPath, _TCHAR('\\'));
	if (ch)
		*(ch + 1) = 0;
	g_Log.Write(_T("Current path: %s"), m_szCurrentPath);
}

DiskManager_t::~DiskManager_t()
{
}

int DiskManager_t::CurrentCd()
{
	const int kctchVolume = 128;
	_TCHAR szVolume[kctchVolume];
	GetVolumeInformation(m_szCurrentDriveRoot, szVolume, kctchVolume, NULL, NULL, NULL, NULL, 0);

	g_Log.Write(_T("Volume label for '%s' is '%s'"), m_szCurrentDriveRoot, szVolume);

	for (int i = 0; i < kctDiskDetails; i++)
		if (_tcsicmp(szVolume, DiskDetails[i].m_pszVolumeLabel) == 0)
			return i;

	return -1;
}

// Make sure the specified CD is inserted in the drive.
// If fAllowNoCd is true, then as long as none of the others in the set is inserted, that will
// do.
// If pszSkipMsgProduct is not NULL, then an incorrect disk message will include an option to
// skip the specified product.
int DiskManager_t::CheckCdPresent(int iCd, bool fAllowNoCd, const _TCHAR * pszSkipMsgProduct)
{
	int iCurrentCd = CurrentCd();
	if (iCurrentCd == iCd)
		knCorrectCdAlready;

	if (fAllowNoCd && iCurrentCd == -1)
		return knNoCd;

	while (iCurrentCd != iCd)
	{
		// We don't have the correct CD:
		g_Log.Write(_T("Disk %d is inserted - asking for disk %d.\n"), iCurrentCd, iCd);

		_TCHAR * pszMsg = new_sprintf(FetchString(IDC_MESSAGE_INSERT), DiskDetails[iCd].m_kpszNiceName);
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
				g_Log.Write(_T("User opted to quit."));
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
				g_Log.Write(_T("User opted to skip product."));
				return knUserSkip;
			}
			break;
		}
		iCurrentCd = CurrentCd();
	} // Try again

	return knCorrectCdFinally;
}

int DiskManager_t::EnsureCdForFile(const _TCHAR * pszFile, int iCd, const _TCHAR * pszSkipMsgProduct)
{
	int iCurrentCd = CurrentCd();

	// See if file can be found from current directory:
	const int knLen = MAX_PATH;
	_TCHAR szPath[knLen];
	_tcscpy_s(szPath, MAX_PATH, m_szCurrentPath);
	_tcscat_s(szPath, MAX_PATH, pszFile);
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
_TCHAR * DiskManager_t::NewFullPath(const _TCHAR * pszRelativePath)
{
	return new_sprintf(_T("%s%s"), m_szCurrentPath, pszRelativePath);
}
