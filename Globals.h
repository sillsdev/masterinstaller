#pragma once

extern bool g_fStopRequested;
extern bool g_fLessThanWin2k;
extern DWORD g_langidWindowsLanguage;
extern bool g_fAdministrator;
extern bool g_fRebootPending;
extern bool g_fManualInstall;
class UserQuitException_t
{
};
extern UserQuitException_t UserQuitException;

extern void * hStopRequestInProgress; // Event to pause main thread while user confirms stop.
extern void CheckIfStopRequested();

extern const char * g_pszTitle;
extern const char * g_pszListSubtitle;
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
extern const char * g_pszGetKeyTitle;
extern const char * g_pszExternalHelpFile;
extern const char * g_pszHelpButtonText;
extern const char * g_pszTermsOfUseFile;
extern const char * g_pszTermsButtonText;

// Used to signify products which are visible but not installable:
extern const int knVisibleOnlyOffset;

// Add the auto-generated globals:
#include "AutoGlobals.h"