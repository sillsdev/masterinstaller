#pragma once

/*
	Copyright (c) 2003 SIL International.

	Master installer Dialogs header file.
*/

#include "ProductManager.h"

class IHelpLauncher;

// Structure for passing to the main product selection dialog:
struct DlgMainProductParams_t
{
	IHelpLauncher * pHelpLauncher;
	IProductManager * pProductManager;
	bool m_fReenterKeyAllowed;
};

// Structure for passing to the report dialog:
struct ReportPackage_t
{
	const char * pszTitle;
	const char * pszIntro;
	const char * pszOkButtonText;
	bool fConfirmCancel;
	const IProductManager * pProductManager;
	int nRptType;
	bool fCanToggleShortAndFull;
	IndexList_t * prgiProducts;

	ReportPackage_t() : pszTitle(NULL), pszIntro(NULL), pszOkButtonText(NULL),
		fConfirmCancel(true), pProductManager(NULL), nRptType(0),
		fCanToggleShortAndFull(false), prgiProducts(NULL) { }
};

// Structure for returning from the main product selection dialog. Caller must delete it.
struct MainSelectionReturn_t
{
	IndexList_t m_rgiChosen;
	bool m_fInstallRequiredSoftware;
	bool m_fReenterKey;
};

INT_PTR CALLBACK DlgProcDisplayReport(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcRebootCountdown(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcProductKey(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcMainProductSelect(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern void ShowStatusDialog();
extern const char * DisplayStatusText(int iPosition, const char * pszText, ...);
extern void PauseOnStatusDialog(int ridStatusText);
extern void HideStatusDialog();
extern void PauseForReboot();
