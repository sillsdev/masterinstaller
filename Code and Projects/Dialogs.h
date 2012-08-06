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
	const _TCHAR * pszTitle;
	const _TCHAR * pszIntro;
	const _TCHAR * pszOkButtonText;
	bool fConfirmCancel;
	const IProductManager * pProductManager;
	int nRptType;
	bool fCanToggleShortAndFull;
	IndexList_t * prgiProducts;

	ReportPackage_t() : pszTitle(NULL), pszIntro(NULL), pszOkButtonText(NULL),
		fConfirmCancel(true), pProductManager(NULL), nRptType(0),
		fCanToggleShortAndFull(false), prgiProducts(NULL) { }
};

class StatusTextSnapshot
{
protected:
	_TCHAR * m_pszText[3];
	_TCHAR * GetText(int i);

public:
	StatusTextSnapshot();
	~StatusTextSnapshot();
	void Repost();
};

INT_PTR CALLBACK DlgProcDisplayReport(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcRebootCountdown(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcMainProductSelect(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern void CentralizeWindow(HWND hwnd);
extern void SetSilIcon(HWND hwnd);
extern void ShowStatusDialog();
extern BOOL my_OpenClipboard(bool * fCloseStatusDialog);
extern const _TCHAR * DisplayStatusText(int iPosition, const _TCHAR * pszText, ...);
extern void PauseOnStatusDialog(int ridStatusText);
extern void HideStatusDialog();
extern void PauseForReboot();
