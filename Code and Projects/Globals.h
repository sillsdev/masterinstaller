#pragma once

// Comprehensive functionality dealing with operating system versions:
class OSVersion_t
{
public:
	OSVersion_t();
	~OSVersion_t();

	_TCHAR * MakeGeneralDescription(const _TCHAR * pszOsNum);
	_TCHAR * MakeDescription();
	_TCHAR * Numeric();
	bool WithinRange(const _TCHAR * pszMin, const _TCHAR * pszMax);
	bool operator == (const _TCHAR * pszVer);
	bool operator != (const _TCHAR * pszVer);
	bool operator <= (const _TCHAR * pszVer);
	bool operator >= (const _TCHAR * pszVer);
	bool operator < (const _TCHAR * pszVer);
	bool operator > (const _TCHAR * pszVer);

	static _TCHAR * Win2k;
	static _TCHAR * XP;
	static _TCHAR * Vista;
	static _TCHAR * W7;
	static _TCHAR * W8;
	static _TCHAR * kpszOperatorError;

protected:
	OSVERSIONINFOEX m_OSversion;
	_TCHAR * m_pszVersion;
};
// Useful class for handling selections or other groups of products:
class IndexList_t
{
	friend class PersistantProgress;

public:
	IndexList_t();
	IndexList_t(IndexList_t &Copy);
	~IndexList_t();

	int operator [] (int i) const;
	IndexList_t & operator = (const IndexList_t & Copy);
	void CopyObject(const IndexList_t & Copy);
	int GetCount() const;
	bool Contains(int index) const;
	void Add(int n, bool fIgnoreDuplicates = true);
	void Add(const IndexList_t & List, bool fIgnoreDuplicates = true);
	int RemoveNthItem(int n);
	void Flush();
	void ReplaceItem(int i, int nNew);

protected:
	int * m_pi;
	int m_ct;
};
// Structure for returning from the main product selection dialog. Caller must delete it.
struct MainSelectionReturn_t
{
	IndexList_t m_rgiChosen;
	bool m_fInstallRequiredSoftware;
	bool m_fReenterKey;
};
extern OSVersion_t g_OSVersion;
extern bool g_fStopRequested;
extern DWORD g_langidWindowsLanguage;
extern bool g_fAdministrator;
extern bool g_fRebootPending;
extern bool g_fManualInstall;
extern bool g_fSilent;
extern struct MainSelectionReturn_t * g_pCmdLineProductSelection;
class UserQuitException_t
{
};
extern UserQuitException_t UserQuitException;

extern void * hStopRequestInProgress; // Event to pause main thread while user confirms stop.
extern void CheckIfStopRequested();

enum
{
	eTextAlignLeftEdge,
	eTextAlignButtons,
	eTextAlignCheckBoxes,
};
extern const _TCHAR * g_pszTitle;
extern const _TCHAR * g_pszListSubtitle;
extern const int g_nListBackgroundR;
extern const int g_nListBackgroundG;
extern const int g_nListBackgroundB;
extern const int g_nListBackgroundBmpOffsetX;
extern const int g_nListBackgroundBmpOffsetY;
extern const int g_nListBackgroundBmpWeightTop;
extern const int g_nListBackgroundBmpWeightBottom;
extern const int g_nListBackgroundBmpWeightLeft;
extern const int g_nListBackgroundBmpWeightRight;
extern const bool g_fListEvenOneProduct;
extern const int g_nListSpacingAdjust;
extern const int g_nInfoButtonAdjust;
extern const bool g_fStartFromAnyCd;
extern const bool g_fKeyPromptNeedsShiftCtrl;
extern const _TCHAR * g_pszGetKeyTitle;
extern const _TCHAR * g_pszInitialText;
extern const int g_nInitialTextAlign;
extern const int g_nInitialTextLeftEdge;
extern const int g_nInitialTextRightEdge;
extern const _TCHAR * g_pszExternalHelpFile;
extern const _TCHAR * g_pszHelpButtonText;
extern const _TCHAR * g_pszTermsOfUseFile;
extern const _TCHAR * g_pszTermsButtonText;
extern const bool g_fShowInstallCompleteMessage;

// Used to signify products which are visible but not installable:
extern const int knVisibleOnlyOffset;

// Add the auto-generated globals:
#include "AutoGlobals.h"