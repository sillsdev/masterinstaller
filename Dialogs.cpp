/*
	Copyright (c) 2003 SIL International.

	Master installer dialogs file.
*/

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <CommCtrl.h>
#include <tchar.h>

#include "Dialogs.h"
#include "resource.h"
#include "UsefulStuff.h"
#include "Globals.h"
#include "HelpLauncher.h"
#include "LogFile.h"
#include "DiskManager.h"
#ifdef EASTER_EGGS
#include "midi.h"
#endif

void CentralizeWindow(HWND hwnd)
// Move specified window to center of desktop.
{
	HWND desktop = GetDesktopWindow();
	RECT screenRect, dialogRect;
	GetClientRect(desktop, &screenRect);
	GetClientRect(hwnd, &dialogRect);
	int nScreenW = screenRect.right - screenRect.left;
	int nScreenH = screenRect.bottom - screenRect.top;
	int nDialogW = dialogRect.right - dialogRect.left;
	int nDialogH = dialogRect.bottom - dialogRect.top;
	SetWindowPos(hwnd, HWND_TOP, (nScreenW - nDialogW)/2, (nScreenH - nDialogH)/2,
		nDialogW, nDialogH, SWP_NOSIZE);
}

void SetSilIcon(HWND hwnd)
// Set window's icon to the SIL logo.
{
	// Set Icon:
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDR_MAIN_ICON);
	if (hIcon)
	{
		SendMessage(hwnd, WM_SETICON, 1, (LPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, 0, (LPARAM)hIcon);
		DestroyIcon(hIcon);
		hIcon = NULL;
	}
}

int GetTextHeight(const _TCHAR * pszText, int nWidth)
// Calculates the height needed for a static control to display the given text
// on a dialog box. nWidth is in dialog units.
{
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = nWidth;
	rect.bottom = 0;
	DrawText(hdc, pszText, -1, &rect, DT_CALCRECT);
	if (rect.right > nWidth)
	{
		rect.right = nWidth;
		rect.bottom = 0;
		DrawText(hdc, pszText, -1, &rect, DT_CALCRECT | DT_WORDBREAK);
	}
	DeleteDC(hdc);
	return rect.bottom;
}

INT_PTR CALLBACK DlgProcDisplayReport(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// The dialog procedure for displaying dependency reports.
{
	static ReportPackage_t * ReportPackage = NULL;
	static bool fMore = false;

	switch(msg)
	{
	case WM_INITDIALOG: // Dialog is being shown.
		{
			CentralizeWindow(hwnd);

			ReportPackage = reinterpret_cast<ReportPackage_t *>(lParam);

			if (ReportPackage)
			{
				// Set Title:
				SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)ReportPackage->pszTitle);

				// Set Introductory text:
				SendDlgItemMessage(hwnd, IDC_STATIC_REPORT_INTRO, WM_SETTEXT, 0,
					(LPARAM)ReportPackage->pszIntro);

				// Set OK button text:
				if (ReportPackage->pszOkButtonText)
				{
					SendDlgItemMessage(hwnd, IDOK, WM_SETTEXT, 0,
						(LPARAM)ReportPackage->pszOkButtonText);
				}

				g_Log.Write(_T("Creating report dialog:"));
				g_Log.Write(ReportPackage->pszTitle);
				g_Log.Write(ReportPackage->pszIntro);
				if (ReportPackage->pProductManager)
				{
					_TCHAR * pszReport = ReportPackage->pProductManager->GenReport(
						ReportPackage->nRptType, ReportPackage->prgiProducts);
					if (pszReport)
					{
						SetDlgItemText(hwnd, IDC_EDIT_REPORT, pszReport);
						g_Log.Write(_T("Report contents:"));
						g_Log.Indent();
						g_Log.Write(_T("%s"), pszReport);
						g_Log.Unindent();
					}
					delete[] pszReport;
					pszReport = NULL;
				}

				// If we can't toggle between short and full reports, hide the More Details
				// button:
				if (!ReportPackage->fCanToggleShortAndFull)
					ShowWindow(GetDlgItem(hwnd, IDC_BUTTON_MORE_DETAILS), SW_HIDE);
				fMore = false;
			}

			// Set Icon:
			SetSilIcon(hwnd);
		}
		break;

	case WM_COMMAND: // We got a message from a control/menu - in this case, a button.
		switch(LOWORD(wParam))
		{
		case IDOK: // The Proceed button
			EndDialog(hwnd, 1);
			break;

		case IDCANCEL:
			// If there is no cancel button, user may have pressed the X on the top right, or
			// used ALT+F4:
			if (ReportPackage)
			{
				if (ReportPackage->fConfirmCancel)
				{
					if (MessageBox(hwnd, FetchString(IDC_MESSAGE_CONFIRM_QUIT), g_pszTitle,
						MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) != IDYES)
					{
						break;
					}
				}
			}
			g_Log.Write(_T("User quit in Report Dialog."));
			EndDialog(hwnd, 0);
			break;

		case IDC_BUTTON_MORE_DETAILS: // User pressed the More Details button
			if (ReportPackage)
			{
				if (ReportPackage->pProductManager)
				{
					fMore = !fMore;
					switch (ReportPackage->nRptType)
					{
					case IProductManager::rptPrerequisitesShort:
						ReportPackage->nRptType = IProductManager::rptPrerequisitesFull;
						break;
					case IProductManager::rptPrerequisitesFull:
						ReportPackage->nRptType = IProductManager::rptPrerequisitesShort;
						break;
					case IProductManager::rptRequirementsShort:
						ReportPackage->nRptType = IProductManager::rptRequirementsFull;
						break;
					case IProductManager::rptRequirementsFull:
						ReportPackage->nRptType = IProductManager::rptRequirementsShort;
						break;
					}
					_TCHAR * pszReport = ReportPackage->pProductManager->GenReport(
						ReportPackage->nRptType, ReportPackage->prgiProducts);
					if (pszReport)
					{
						SetDlgItemText(hwnd, IDC_EDIT_REPORT, pszReport);
						g_Log.Write(_T("Report contents:"));
						g_Log.Indent();
						g_Log.Write(_T("%s"), pszReport);
						g_Log.Unindent();
					}
					delete[] pszReport;
					pszReport = NULL;
					SendDlgItemMessage(hwnd, IDC_BUTTON_MORE_DETAILS, WM_SETTEXT, 0, (LPARAM)
						(FetchString(fMore? IDC_MESSAGE_BASIC_LIST :
						IDC_MESSAGE_MORE_DETAILS)));
				}
			}
			break;
		}
		break;

	case WM_DESTROY: // Dialog is off the screen by now.
		ReportPackage = NULL;
		break;

	default: // All the messages we don't handle are handled by Windows.
		return 0;
	}
	return 1; // This means we have processed the message.
}



INT_PTR CALLBACK DlgProcRebootCountdown(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// The dialog procedure for displaying a countdown to a reboot.
{
	static __int64 nStartTime = 0;
	// Factor for converting seconds into 100s of nanoseconds, for use with time functions:
	static const __int64 kn100NanoSeconds = 10 * 1000 * 1000;
	static int nDelay = 180; // Allow 180 seconds before quitting automatically.

	switch(msg)
	{
	case WM_INITDIALOG: // Dialog is being shown.
		{
			// Set Title:
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)g_pszTitle);

			CentralizeWindow(hwnd);

			// Set up a timer, so we can adjust the countdown value every second:
			::SetTimer(hwnd, 1, 1000, NULL);
			// Get the current time:
			SYSTEMTIME systTime;
			FILETIME filtTime;
			GetLocalTime(&systTime);
			SystemTimeToFileTime(&systTime, &filtTime);
			nStartTime = *((__int64 *)(&filtTime));
		}
		break;

	case WM_COMMAND: // We got a message from a control/menu - in this case, a button.
		switch(LOWORD(wParam))
		{
		case IDOK:
			::KillTimer(hwnd, 1);
			EndDialog(hwnd, 1);
			break;
		case IDCANCEL:
			if (MessageBox(hwnd, FetchString(IDC_MESSAGE_CONFIRM_QUIT_REBOOT), g_pszTitle,
				MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) == IDYES)
			{
				g_Log.Write(_T("User quit in Reboot Countdown Dialog."));
				EndDialog(hwnd, 0);
			}
			break;
		case IDC_BUTTON_PAUSE_RESUME: // User pressed Pause/Resume button.
			{
				static bool fPaused = false;
				// Get the current time:
				SYSTEMTIME systTime;
				FILETIME filtTime;
				GetLocalTime(&systTime);
				SystemTimeToFileTime(&systTime, &filtTime);
				__int64 nCurrentTime = *((__int64 *)(&filtTime));
				if (fPaused)
				{
					// Restart timer:
					nStartTime = *((__int64 *)(&filtTime));
					::SetTimer(hwnd, 1, 1000, NULL);
					SendDlgItemMessage(hwnd, IDC_BUTTON_PAUSE_RESUME, WM_SETTEXT, 0,
						(LPARAM)FetchString(IDC_MESSAGE_PAUSE));
				}
				else // Not currently paused
				{
					int nDuration = (int)((nCurrentTime - nStartTime) / kn100NanoSeconds);
					// Update the remaining delay:
					nDelay -= nDuration;
					// Cancel current timer:
					::KillTimer(hwnd, 1);
					SendDlgItemMessage(hwnd, IDC_BUTTON_PAUSE_RESUME, WM_SETTEXT, 0,
						(LPARAM)FetchString(IDC_MESSAGE_RESUME));
				}
				fPaused = !fPaused;
			}
		}
		break;

	case WM_TIMER:
		{
			// Get the current time:
			SYSTEMTIME systTime;
			FILETIME filtTime;
			GetLocalTime(&systTime);
			SystemTimeToFileTime(&systTime, &filtTime);
			__int64 nCurrentTime = *((__int64 *)(&filtTime));
			int nDuration = (int)((nCurrentTime - nStartTime) / kn100NanoSeconds);
			_TCHAR * pszTime = new_sprintf(_T("%d"), nDelay - nDuration);
			SendDlgItemMessage(hwnd, IDC_STATIC_REBOOT_COUNTDOWN, WM_SETTEXT, 0,
				(LPARAM)pszTime);
			delete[] pszTime;
			pszTime = NULL;
			if (nDuration >= nDelay)
				SendMessage(hwnd, WM_COMMAND, IDOK, 0);
		}
		break;

	case WM_DESTROY: // Dialog is off the screen by now.
		break;

	default: // All the messages we don't handle are handled by Windows.
		return 0;
	}
	return 1; // This means we have processed the message.
}

// Blends the given source bitmap into the given background color, according to 
// the given percentage weightings. Resultant image is drawn in hbmpDest. Works for all bitmap 
// types, but is slower than BlendBitmap32.
void BlendBitmapNon32(HBITMAP hbmpSource, int nSrcWidth, int nSrcHeight, int nSrcDx, int nSrcDy,
					  COLORREF clrBackground,
					  HBITMAP hbmpDest, HDC hdcDest, int nDestWidth, int nDestHeight,
					  int nWeightLeft, int nWeightRight, int nWeightTop, int nWeightBottom)
{
	HDC memDestDC = CreateCompatibleDC(hdcDest);
	SelectObject(memDestDC, hbmpDest);
	HDC memBmpDC = CreateCompatibleDC(hdcDest);
	SelectObject(memBmpDC, hbmpSource);

	// Sort out individual Red, Green and Blue for default background. This is arranged
	// diffenrently in a bitmap from in a COLORREF:
	int bgclrRed, bgclrGreen, bgclrBlue;
	bgclrRed = GetRValue(clrBackground);
	bgclrGreen = GetGValue(clrBackground);
	bgclrBlue = GetBValue(clrBackground);

	for (int DestY = 0; DestY < nDestHeight; DestY++)
	{
		for (int DestX = 0; DestX < nDestWidth; DestX++)
		{
			int SrcX = DestX - nSrcDx;
			int SrcY = DestY - nSrcDy;

			if (SrcX >= 0 && SrcX < nSrcWidth && SrcY >= 0 && SrcY < nSrcHeight)
			{
				COLORREF colSource = GetPixel(memBmpDC, SrcX, SrcY);
				int redSource = GetRValue(colSource);
				int greenSource = GetGValue(colSource);
				int blueSource = GetBValue(colSource);

				double HBlend = nWeightLeft + ((nWeightRight - nWeightLeft) * DestX) / nDestWidth;
				double VBlend = nWeightTop + ((nWeightBottom - nWeightTop) * DestY) / nDestHeight;
				double Blend = (HBlend + VBlend) / 200.0;

				DWORD RedFinal = (DWORD)(bgclrRed + (Blend * (redSource - bgclrRed)));
				DWORD GreenFinal = (DWORD)(bgclrGreen + (Blend * (greenSource - bgclrGreen)));
				DWORD BlueFinal = (DWORD)(bgclrBlue + (Blend * (blueSource - bgclrBlue)));

				SetPixel(memDestDC, DestX, DestY, RGB(RedFinal, GreenFinal, BlueFinal));
			}
			else
				SetPixel(memDestDC, DestX, DestY, clrBackground);
		}
	}
	DeleteDC(memDestDC);
	DeleteDC(memBmpDC);
}

// Blends the given source bitmap into the given background color, according to 
// the given percentage weightings. Resultant image is drawn in hbmpDest.
// Only works bitmaps that have 32 bits per pixel.
void BlendBitmap32(HBITMAP hbmpSource, int nSrcWidth, int nSrcHeight, int nSrcDx, int nSrcDy,
				   COLORREF clrBackground,
				   HBITMAP hbmpDest, HDC hdcDest, int nDestWidth, int nDestHeight,
				   int nWeightLeft, int nWeightRight, int nWeightTop, int nWeightBottom)
{
	// Get pixels for our bitmap:
	long cBytesSource = 4 * nSrcWidth * nSrcHeight;
	BYTE * pBytesSource = new BYTE [cBytesSource];
	cBytesSource = GetBitmapBits(hbmpSource, cBytesSource, pBytesSource);
	DWORD * pPixelsSource = (DWORD *)pBytesSource;

	// Get pixels for new Dialog bitmap:
	long cBytesDest = 4 * nDestWidth * nDestHeight;
	BYTE * pBytesDest = new BYTE [cBytesDest];
	cBytesDest = GetBitmapBits(hbmpDest, cBytesDest, pBytesDest);
	DWORD * pPixelsDest = (DWORD *)pBytesDest;

	// Sort out individual Red, Green and Blue for default background. This is arranged
	// diffenrently in a bitmap from in a COLORREF:
	int bgclrRed, bgclrGreen, bgclrBlue;
	bgclrRed = GetRValue(clrBackground);
	bgclrGreen = GetGValue(clrBackground);
	bgclrBlue = GetBValue(clrBackground);

	for (int DestY = 0; DestY < nDestHeight; DestY++)
	{
		for (int DestX = 0; DestX < nDestWidth; DestX++)
		{
			int SrcX = DestX - nSrcDx;
			int SrcY = DestY - nSrcDy;

			DWORD RedFinal = bgclrRed;
			DWORD GreenFinal = bgclrGreen;
			DWORD BlueFinal = bgclrBlue;

			if (SrcX >= 0 && SrcX < nSrcWidth && SrcY >= 0 && SrcY < nSrcHeight)
			{
				int iPixelSource = nSrcWidth * SrcY + SrcX;
				DWORD colSource = pPixelsSource[iPixelSource];

				int redSource = (BYTE)(colSource >> 16);
				int greenSource = (BYTE)(colSource >> 8);
				int blueSource = (BYTE)(colSource >> 0);

				double HBlend = nWeightLeft + ((nWeightRight - nWeightLeft) * DestX) / nDestWidth;
				double VBlend = nWeightTop + ((nWeightBottom - nWeightTop) * DestY) / nDestHeight;
				double Blend = (HBlend + VBlend) / 200.0;

				RedFinal = (DWORD)(bgclrRed + (Blend * (redSource - bgclrRed)));
				GreenFinal = (DWORD)(bgclrGreen + (Blend * (greenSource - bgclrGreen)));
				BlueFinal = (DWORD)(bgclrBlue + (Blend * (blueSource - bgclrBlue)));
			}
			int iPixelDest = nDestWidth * DestY + DestX;
			pPixelsDest[iPixelDest] = (RedFinal << 16) | (GreenFinal << 8) | BlueFinal;
		}
	}
	SetBitmapBits(hbmpDest, cBytesDest, pBytesDest);
	delete[] pBytesSource;
	delete[] pBytesDest;
}

// Merges a bitmap (if present in our resources) with the global background color,
// and returns a handle to a brush that will paint the processed image.
HBRUSH CreateBackgroundBrush(HWND hwndDialog, HDC hdcDialog)
{
	COLORREF clrBackground = RGB(g_nListBackgroundR, g_nListBackgroundG, g_nListBackgroundB);
	if (!g_nListBackgroundR && !g_nListBackgroundG && !g_nListBackgroundB)
		clrBackground = GetSysColor(COLOR_3DFACE);

	// Get data on our bitmap:
	BITMAP bmpOurs;
	HBITMAP hBitmapOurs = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BACKGROUND_BMP));
	if (hBitmapOurs)
		GetObject(hBitmapOurs, sizeof(BITMAP), &bmpOurs);
	else
	{
		// Give up on bitmaps, and return a solid color brush:
		return CreateSolidBrush(clrBackground);
	}

	// Create new bitmap for dialog background:
	RECT dialogRect;
	GetClientRect(hwndDialog, &dialogRect);
	int nWidth = dialogRect.right - dialogRect.left;
	int nHeight = dialogRect.bottom - dialogRect.top;
	if (nWidth == 0 || nHeight == 0)
	{
		// Give up on bitmaps, and return a solid color brush:
		DeleteObject(hBitmapOurs);
		return CreateSolidBrush(clrBackground);
	}

	BITMAP bmpDialog;
	HBITMAP hDialogBitmap = CreateCompatibleBitmap(hdcDialog, nWidth, nHeight);
	GetObject(hDialogBitmap, sizeof(BITMAP), &bmpDialog);

	// See if we're using full color bitmaps:
	if (bmpOurs.bmBitsPixel == 32 && bmpDialog.bmBitsPixel == 32)
	{
		BlendBitmap32(hBitmapOurs, bmpOurs.bmWidth, bmpOurs.bmHeight,
			g_nListBackgroundBmpOffsetX, g_nListBackgroundBmpOffsetY,
			clrBackground,
			hDialogBitmap, hdcDialog, nWidth, nHeight, 
			g_nListBackgroundBmpWeightLeft, g_nListBackgroundBmpWeightRight,
			g_nListBackgroundBmpWeightTop, g_nListBackgroundBmpWeightBottom);
	}
	else
	{
		BlendBitmapNon32(hBitmapOurs, bmpOurs.bmWidth, bmpOurs.bmHeight,
			g_nListBackgroundBmpOffsetX, g_nListBackgroundBmpOffsetY,
			clrBackground,
			hDialogBitmap, hdcDialog, nWidth, nHeight, 
			g_nListBackgroundBmpWeightLeft, g_nListBackgroundBmpWeightRight,
			g_nListBackgroundBmpWeightTop, g_nListBackgroundBmpWeightBottom);
	}
	DeleteObject(hBitmapOurs);

	HBRUSH hBrush;
	hBrush = CreatePatternBrush(hDialogBitmap);
	return hBrush;
}

INT_PTR CALLBACK DlgProcMainProductSelect(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// The dialog procedure for displaying the main product choices.
{
	const int knCheckboxInitialId = 10; // Make sure this is higher than other control Ids

	static IProductManager * pProductManager = NULL;
	static IHelpLauncher * pHelpLauncher = NULL;
	static IndexList_t rgiAvailableProducts;
	static bool fReenterKeyAllowed = false;
	static HBRUSH hBrushBackground = NULL;

	switch(msg)
	{
	case WM_INITDIALOG: // Dialog is being shown.
		{
			// Set Title:
			if (g_pszTitle && g_pszListSubtitle)
			{
				_TCHAR * pszTitle = new_sprintf(_T("%s %s"), g_pszTitle, g_pszListSubtitle);
				SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)pszTitle);
				delete[] pszTitle;
			}
			else
				SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)g_pszTitle);

			// Set Icon:
			SetSilIcon(hwnd);

			// Add a check box for every permitted installation, plus a help button if a help
			// command has been specified:
			DlgMainProductParams_t * DlgMainProductParams =
				reinterpret_cast<DlgMainProductParams_t *>(lParam);
			if (DlgMainProductParams)
			{
				pProductManager = DlgMainProductParams->pProductManager;
				pHelpLauncher = DlgMainProductParams->pHelpLauncher;
				fReenterKeyAllowed = DlgMainProductParams->m_fReenterKeyAllowed;
			}
			int nYcoord = 90; // Starting place for introductory text.
			int nXcoord = 220; // Left hand edge of added controls

			// Write introductory text:
			CreateWindow(_T("STATIC"), FetchString(IDC_MESSAGE_INSTRUCTIONS),
				WS_CHILD | WS_VISIBLE, nXcoord, nYcoord, 500, 24, hwnd,
				reinterpret_cast<HMENU>(__int64(-1)), NULL, NULL);

			nYcoord += 40;

			const int kdy = 30 + g_nListSpacingAdjust; // Allows for checkbox height etc.
			const int knCheckBoxOffest = 30; // space between help button and checkbox
			const int knDialogMaxRight = 750; // theoretical max X coord for visible controls

			if (g_pszInitialText)
			{
				// Add initial text:
				int xText = 0;
				switch (g_nInitialTextAlign)
				{
				case eTextAlignButtons:
					xText = nXcoord - g_nInfoButtonAdjust;
					break;
				case eTextAlignCheckBoxes:
					xText = nXcoord + knCheckBoxOffest;
				}
				xText += g_nInitialTextLeftEdge;
				int xWidth = knDialogMaxRight + g_nInitialTextRightEdge - xText;
				int yHeight = GetTextHeight(g_pszInitialText, xWidth);
				CreateWindow(_T("STATIC"), g_pszInitialText, WS_CHILD | WS_VISIBLE, xText, nYcoord, xWidth, yHeight, hwnd, reinterpret_cast<HMENU>(__int64(-1)), NULL, NULL);
				nYcoord += 8 + yHeight;
			}
			pProductManager->GenAvailableMainProductList(rgiAvailableProducts, true);
			bool fFoundInstalledProduct = false;
			g_Log.Indent();
			for (int i = 0; i < rgiAvailableProducts.GetCount(); i++)
			{
				int iProduct = rgiAvailableProducts[i];
				// Check if product is only visible, not installable:
				bool fOnlyVisible = false;
				if (iProduct >= knVisibleOnlyOffset)
				{
					iProduct -= knVisibleOnlyOffset;
					rgiAvailableProducts.ReplaceItem(i, iProduct);
					fOnlyVisible = true;
				}

				_TCHAR * pszCheckBox = my_strdup(pProductManager->GetName(iProduct));

				if (pProductManager->IsInstallable(iProduct) || pProductManager->IsContainer(iProduct))
				{
					// See if product has been installed before:
					if (pProductManager->IsMsiUpgradePermitted(iProduct))
					{
						_TCHAR * pszInstalledVersion = NULL;
						switch (pProductManager->CompareMsiVersionWithInstalled(iProduct, &pszInstalledVersion))
						{
						case -1:
							new_sprintf_concat(pszCheckBox, 0, FetchString(IDC_MESSAGE_DOWNGRADE_DANGER), pszInstalledVersion);
							fOnlyVisible = true;
							break;
						case 0:
							new_sprintf_concat(pszCheckBox, 0, FetchString(IDC_MESSAGE_ALREADY_INSTALLED));
							fFoundInstalledProduct = true;
							break;
						case 1:
							new_sprintf_concat(pszCheckBox, 0, FetchString(IDC_MESSAGE_UPGRADE_READY), pszInstalledVersion);
							break;
						case 2:
							new_sprintf_concat(pszCheckBox, 0, FetchString(IDC_MESSAGE_UPGRADE_TOO_LATE), pszInstalledVersion);
							break;
						}
						delete[] pszInstalledVersion;
						pszInstalledVersion = NULL;
					}
					else if (pProductManager->PossibleToTestPresence(iProduct))
					{
						const _TCHAR * pszVersion = pProductManager->GetTestPresenceVersion(iProduct);
						bool fInstalled = pProductManager->TestPresence(iProduct, pszVersion,
							pszVersion);
						if (fInstalled && !fOnlyVisible)
						{
							new_sprintf_concat(pszCheckBox, 0, _T(" %s"),
								FetchString(IDC_MESSAGE_ALREADY_INSTALLED));
							fFoundInstalledProduct = true;
						}
					}
					// See if product should be disabled because user needs better OS:
					if (!pProductManager->IsOsHighEnough(iProduct))
					{
						_TCHAR * pszMinOS = g_OSVersion.MakeGeneralDescription(
							pProductManager->GetMinOsRequirement(iProduct));

						new_sprintf_concat(pszCheckBox, 0, 
							FetchString(IDC_MESSAGE_NEED_BETTER_OS), pszMinOS);

						delete[] pszMinOS;
						pszMinOS = NULL;

						g_Log.Write(_T("Product %s will be disabled because it needs a minimum of Windows %s"),
							pProductManager->GetName(iProduct), pProductManager->GetMinOsRequirement(iProduct));

						fOnlyVisible = true;
					}
					// See if product should be disabled because user has too good an OS:
					if (!pProductManager->IsOsLowEnough(iProduct))
					{
						_TCHAR * pszMaxOS = g_OSVersion.MakeGeneralDescription(
							pProductManager->GetMaxOsRequirement(iProduct));

						new_sprintf_concat(pszCheckBox, 0,
							FetchString(IDC_MESSAGE_NEED_WORSE_OS), pszMaxOS);

						delete[] pszMaxOS;
						pszMaxOS = NULL;

						g_Log.Write(_T("Product %s will be disabled because it only works up to Windows %s"),
							pProductManager->GetName(iProduct), pProductManager->GetMaxOsRequirement(iProduct));

						fOnlyVisible = true;
					}
					// See if product should be disabled because user needs Admin Privileges:
					else if (!g_fAdministrator && pProductManager->GetMustBeAdminFlag(iProduct))
					{
						new_sprintf_concat(pszCheckBox, 0, _T(" %s"),
							FetchString(IDC_MESSAGE_NEED_ADMIN_PRIV));
						fOnlyVisible = true;
					}

					DWORD wsStyle = BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE;
					// If product is only visible, disable checkbox:
					if (fOnlyVisible)
						wsStyle |= WS_DISABLED;

					CreateWindow(_T("BUTTON"), pszCheckBox, wsStyle, nXcoord + knCheckBoxOffest, nYcoord,
						knDialogMaxRight - nXcoord - knCheckBoxOffest, 20, hwnd,
						reinterpret_cast<HMENU>(__int64(knCheckboxInitialId + i)), NULL,
						NULL);
				}
				else // Product is not installable
				{
					CreateWindow(_T("STATIC"), pszCheckBox, WS_CHILD | WS_VISIBLE, nXcoord + knCheckBoxOffest,
						nYcoord + 2, knDialogMaxRight - nXcoord - knCheckBoxOffest, 20, hwnd,
						reinterpret_cast<HMENU>(__int64(-1)), NULL, NULL);
				}
				delete[] pszCheckBox;
				pszCheckBox = NULL;

				if (pProductManager->GetHelpTag(iProduct))
				{
					int nButtonWidth = 20 + g_nInfoButtonAdjust;
					int nButtonHeight = 24 + g_nInfoButtonAdjust;
					int nButtonX = nXcoord - g_nInfoButtonAdjust;
					int nButtonY = nYcoord + 10 - nButtonHeight / 2;
					// Create a help (more info) button:
					CreateWindow(_T("BUTTON"), _T("?"),
						BS_PUSHBUTTON | WS_TABSTOP | WS_CHILD |WS_VISIBLE, nButtonX, nButtonY,
						nButtonWidth, nButtonHeight, hwnd,
						reinterpret_cast<HMENU>(__int64(knCheckboxInitialId +
						rgiAvailableProducts.GetCount() + 1 + i)), NULL, NULL);
				}
				nYcoord += kdy;
			}
			g_Log.Unindent();

			// See if any installed main products have any uninstalled dependencies:
			if (fFoundInstalledProduct && pProductManager->RequirementsNeeded())
			{
				// Add option to allow user just to install required supporting software:
				HWND hwnd3rdParty = CreateWindow(_T("BUTTON"),
					FetchString(IDC_MESSAGE_REQUIRED_SOFTWARE),
					BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD |WS_VISIBLE, 250, nYcoord, 400,
					20, hwnd,
					reinterpret_cast<HMENU>(__int64(knCheckboxInitialId +
					rgiAvailableProducts.GetCount())), NULL, NULL);
				nYcoord += kdy;
				SendMessage(hwnd3rdParty, BM_SETCHECK, BST_CHECKED, 0);
			}
			// Now move bottom buttons beyond check boxes:
			nYcoord += 15; // Extra gap after last check box
			HWND hwndCancel = GetDlgItem(hwnd, IDCANCEL);
			HWND hwndOk = GetDlgItem(hwnd, IDOK);
			HWND hwndHelp = GetDlgItem(hwnd, IDC_BUTTON_MAIN_HELP);
			HWND hwndTerms = GetDlgItem(hwnd, IDC_BUTTON_TERMS);
			HWND hwndReenter = GetDlgItem(hwnd, IDC_BUTTON_REENTER_KEY);
			RECT rectCancel, rectOk, rectHelp, rectTerms, rectReenter, dialogRect;
			GetWindowRect(hwndCancel, &rectCancel);
			GetWindowRect(hwndOk, &rectOk);
			GetWindowRect(hwndHelp, &rectHelp);
			GetWindowRect(hwndTerms, &rectTerms);
			GetWindowRect(hwndReenter, &rectReenter);
			GetWindowRect(hwnd, &dialogRect);
			int nCancelRelL = rectCancel.left - dialogRect.left;
			int nCancelRelT = rectCancel.top - dialogRect.top;
			int nCancelW = rectCancel.right - rectCancel.left;
			int nCancelH = rectCancel.bottom - rectCancel.top;
			int nOkRelL = rectOk.left - dialogRect.left;
			int nOkRelT = rectOk.top - dialogRect.top;
			int nOkW = rectOk.right - rectOk.left;
			int nOkH = rectOk.bottom - rectOk.top;
			int nHelpRelL = rectHelp.left - dialogRect.left;
			int nHelpRelT = rectHelp.top - dialogRect.top;
			int nHelpW = rectHelp.right - rectHelp.left;
			int nHelpH = rectHelp.bottom - rectHelp.top;
			int nTermsRelL = rectTerms.left - dialogRect.left;
			int nTermsRelT = rectTerms.top - dialogRect.top;
			int nTermsW = rectTerms.right - rectTerms.left;
			int nTermsH = rectTerms.bottom - rectTerms.top;
			int nReenterRelL = rectReenter.left - dialogRect.left;
			int nReenterRelT = rectReenter.top - dialogRect.top;
			int nReenterW = rectReenter.right - rectReenter.left;
			int nReenterH = rectReenter.bottom - rectReenter.top;
			MoveWindow(hwndCancel, nCancelRelL, nYcoord, nCancelW, nCancelH, true);
			MoveWindow(hwndOk, nOkRelL, nYcoord, nOkW, nOkH, true);
			// Move the help button, unless there is no help file, in which case hide it:
			if (g_pszExternalHelpFile)
			{
				MoveWindow(hwndHelp, nHelpRelL, nYcoord, nHelpW, nHelpH, true);
				// Set the correct text, too:
				SendMessage(hwndHelp, WM_SETTEXT, 0, (LPARAM)g_pszHelpButtonText);
			}
			else
				ShowWindow(hwndHelp, SW_HIDE);
			// Move the terms of use button, unless there is no such file, in which case hide:
			if (g_pszTermsOfUseFile)
			{
				MoveWindow(hwndTerms, nTermsRelL, nYcoord, nTermsW, nTermsH, true);
				// Set the correct text, too:
				SendMessage(hwndTerms, WM_SETTEXT, 0, (LPARAM)g_pszTermsButtonText);
			}
			else
				ShowWindow(hwndTerms, SW_HIDE);

			// If there are no protected products, and the user is allowed to re-enter
			// a product key, remove the Re-enter Key button.
			// Otherwise, move it to its new location:
			if (pProductManager->GetNumProtectedMainProducts() > 0 && fReenterKeyAllowed)
				MoveWindow(hwndReenter, nReenterRelL, nYcoord, nReenterW, nReenterH, true);
			else
				ShowWindow(hwndReenter, SW_HIDE);

			// Now resize window and centralize it:
			int nFullDialogH = dialogRect.bottom - dialogRect.top;
			HWND desktop = GetDesktopWindow();
			RECT screenRect;
			GetClientRect(desktop, &screenRect);
			GetClientRect(hwnd, &dialogRect);
			int nScreenW = screenRect.right - screenRect.left;
			int nScreenH = screenRect.bottom - screenRect.top;
			int nDialogW = dialogRect.right - dialogRect.left;
			int nDialogH = dialogRect.bottom - dialogRect.top;
			nDialogH = nYcoord + nCancelH + 10 + nFullDialogH -
				(dialogRect.bottom - dialogRect.top);
			SetWindowPos(hwnd, HWND_TOP, (nScreenW - nDialogW)/2, (nScreenH - nDialogH)/2,
				nDialogW, nDialogH, 0);
		}
		break;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		{
			// Set background:
			if (!hBrushBackground)
				hBrushBackground = CreateBackgroundBrush(hwnd, (HDC)wParam);

			SetBkMode((HDC)wParam, TRANSPARENT);
			return (INT_PTR)hBrushBackground;
		}
		break;

	case WM_COMMAND: // We got a message from a control/menu - in this case, a button.
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				IndexList_t rgiChosen;
				g_Log.Write(_T("User chose following products:"));
				g_Log.Indent();
				for (int i = 0; i < rgiAvailableProducts.GetCount(); i++)
				{
					if (SendDlgItemMessage(hwnd, knCheckboxInitialId + i, BM_GETCHECK, 0, 0) ==
						BST_CHECKED)
					{
						// The current product checkbox was checked, so add product index to
						// return structure:
						rgiChosen.Add(rgiAvailableProducts[i]);
						g_Log.Write(_T("%d: %s"), rgiChosen.GetCount(), pProductManager ?
							pProductManager->GetName(rgiAvailableProducts[i]) : _T("Unknown"));
					}
				}
				g_Log.Unindent();
				bool fInstallRequiredSoftware = true;
				bool fInstallRequiredSoftwareButtonChecked = false;
				// See if we offered the user a choice of installing 3rd party software:
				if (GetDlgItem(hwnd, knCheckboxInitialId + rgiAvailableProducts.GetCount()))
				{
					// We did, so collect their choice:
					fInstallRequiredSoftwareButtonChecked = (SendDlgItemMessage(hwnd,
						knCheckboxInitialId + rgiAvailableProducts.GetCount(), BM_GETCHECK, 0,
						0) == BST_CHECKED);
					if (!fInstallRequiredSoftwareButtonChecked)
						fInstallRequiredSoftware = false;
					g_Log.Write(_T("User opted for additional software: %s."),
						fInstallRequiredSoftware ? _T("true") : _T("false"));
				}
				if (rgiChosen.GetCount() == 0 && !fInstallRequiredSoftwareButtonChecked)
				{
					g_Log.Write(_T("User selected no products, nor the additional software."));
					if (MessageBox(hwnd, FetchString(IDC_MESSAGE_CONFIRM_QUIT_MAIN), g_pszTitle,
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)
					{
						g_Log.Write(_T("User quit in Main Product Selection Dialog."));
						EndDialog(hwnd, 0);
					}
					else
					{
						g_Log.Write(_T("User decided to reconsider."));
						break;
					}
				}
				// The return value of this dialog will be a pointer to this structure:
				MainSelectionReturn_t * MainSelectionReturn = new MainSelectionReturn_t;
				// populate return structure:
				MainSelectionReturn->m_rgiChosen = rgiChosen;
				MainSelectionReturn->m_fInstallRequiredSoftware = fInstallRequiredSoftware;
				MainSelectionReturn->m_fReenterKey = false;

				EndDialog(hwnd, reinterpret_cast<INT_PTR>(MainSelectionReturn));
			}
			break;

		case IDCANCEL: // User pressed either Quit, the X on the top right, or used ALT+F4
			if (MessageBox(hwnd, FetchString(IDC_MESSAGE_CONFIRM_QUIT_MAIN), g_pszTitle,
				MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)
			{
				g_Log.Write(_T("User quit in Main Product Selection Dialog."));
				EndDialog(hwnd, 0);
			}
			break;
		// Deal with service bulletins and terms of use files:
		case IDC_BUTTON_MAIN_HELP: // Fall through
		case IDC_BUTTON_TERMS:
			{
				// Build path to file:
				_TCHAR * pszExeFolder = NewGetExeFolder();
				_TCHAR * pszHelpPath;
				if (LOWORD(wParam) == IDC_BUTTON_MAIN_HELP)
					pszHelpPath = MakePath(pszExeFolder, g_pszExternalHelpFile);
				else
					pszHelpPath = MakePath(pszExeFolder, g_pszTermsOfUseFile);

				delete[] pszExeFolder;
				pszExeFolder = NULL;

				ShellExecute(hwnd, _T("open"), pszHelpPath, NULL, _T("."), SW_SHOW);
				
				delete[] pszHelpPath;
				pszHelpPath = NULL;
				
				break;
			}
		case IDC_BUTTON_REENTER_KEY:
			{
				// The return value of this dialog will be a pointer to this structure:
				MainSelectionReturn_t * MainSelectionReturn = new MainSelectionReturn_t;
				MainSelectionReturn->m_fReenterKey = true;
				EndDialog(hwnd, reinterpret_cast<INT_PTR>(MainSelectionReturn));
			}
			break;
		default: // See if a help button was pressed:
			{
				int nHelpId = LOWORD(wParam) - knCheckboxInitialId -
					rgiAvailableProducts.GetCount() - 1;
				if (nHelpId >= 0 && nHelpId < rgiAvailableProducts.GetCount())
				{
					const _TCHAR * pszHelpTag =
						pProductManager->GetHelpTag(rgiAvailableProducts[nHelpId]);
					if (pszHelpTag)
					{
						// See if the help tag is an external file:
						if (!pProductManager->GetHelpTagInternalFlag(
							rgiAvailableProducts[nHelpId]))
						{
							// Form full path name to file:
							_TCHAR * pszFilePath = g_DiskManager.NewFullPath(pszHelpTag);
							// Open file:
							ShellExecute(NULL, NULL, pszFilePath, NULL, _T("."), SW_SHOW);
							delete[] pszFilePath;
							pszFilePath = NULL;
						}
						else // Use InstallerHelp2.dll
							pHelpLauncher->LaunchHelp(pszHelpTag);
					}
				}
			}
			break;
		}
		break;

	case WM_DESTROY: // Dialog is off the screen by now.
		rgiAvailableProducts.Flush();
		DeleteObject(hBrushBackground);
		hBrushBackground = NULL;
		break;

	default: // All the messages we don't handle are handled by Windows.
		return 0;
	}
	return 1; // This means we have processed the message.
}

static HWND hwndStatusDialog = NULL; // Handle of Status dialog window
static bool fContinueWithoutStatusDialog = false; // Flag for when StatusDialog fails
static HANDLE hCreateStatusDlg = NULL; // Event used to synchronize threads on dialog creation.
static HANDLE hDestroyStatusDlg = NULL; // Event used to sync threads on dialog destruction.
static const _TCHAR * kszStatusDlgMutexName = _T("SIL Master Installer Status Dialog");
static const int s_kctStatusText = 3;
static _TCHAR * pszStatusTextCopy[s_kctStatusText] = { NULL, NULL, NULL };
static int ridStatusText[s_kctStatusText] =
	{ IDC_STATIC_STATUS_MAIN, IDC_STATIC_STATUS_SUB, IDC_STATIC_STATUS_3RD };
static bool fPauseStatusDialog = false;
static int ridPauseStatusText = IDC_MESSAGE_MUST_WAIT;

INT_PTR CALLBACK DlgProcStatus(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// The dialog procedure for displaying current status.
{
	switch(msg)
	{
	case WM_INITDIALOG: // Dialog is being shown.
		{
			hwndStatusDialog = hwnd;

			// See if we need the Continue button:
			if (fPauseStatusDialog)
			{
				// We do, so add the extra status text:
				if (ridPauseStatusText)
					DisplayStatusText(2, FetchString(ridPauseStatusText));
				// Restore other status texts:
				for (int i = 0; i < 2; i++)
					DisplayStatusText(i, pszStatusTextCopy[i]);
			}
			else // we do not need it, so hide it:
			{
				HWND hwndContinue = GetDlgItem(hwnd, IDC_BUTTON_CONTINUE);
				ShowWindow(hwndContinue, SW_HIDE);
			}

			// Set Title:
			SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)g_pszTitle);
			// Set Icon:
			SetSilIcon(hwnd);

			bool fUseAnimateControl = (lParam == 1);

			// Display dialog near bottom left of screen:
			HWND desktop = GetDesktopWindow();
			RECT screenRect, dialogRect;
			GetClientRect(desktop, &screenRect);
			GetWindowRect(hwnd, &dialogRect);
			int nScreenW = screenRect.right - screenRect.left;
			int nScreenH = screenRect.bottom - screenRect.top;
			int nDialogW = dialogRect.right - dialogRect.left;
			int nDialogH = dialogRect.bottom - dialogRect.top;
			SetWindowPos(hwnd, HWND_TOP, 50, nScreenH - nDialogH - 50, 0, 0, SWP_NOSIZE);

			if (fUseAnimateControl)
			{
				// Start the animation:
				HWND hwndAvi = GetDlgItem(hwnd, IDC_ANIMATE_STATUS);
				Animate_Open(hwndAvi, _T("ANIMATIONFILE"));
			}

			// Signal that our window is created:
			if (hCreateStatusDlg)
				::SetEvent(hCreateStatusDlg);
		}
		break;

	case WM_COMMAND: // We got a message from a control/menu - in this case, a button.
		switch(LOWORD(wParam))
		{
		case IDCANCEL: // User pressed either Quit, the X on the top right, or used ALT+F4

			// Make sure main thread does not advance far until user has decided whether
			// or not to confirm the stop request:
			hStopRequestInProgress = CreateEvent(NULL, true, false, NULL);

			if (MessageBox(hwnd, FetchString(IDC_MESSAGE_CONFIRM_QUIT_WHOLE), g_pszTitle,
				MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) == IDYES)
			{
				g_fStopRequested = true;
				_TCHAR * pszMsg;
				if (g_rgchActiveProcessDescription[0])
				{
					pszMsg = new_sprintf(FetchString(IDC_MESSAGE_QUITTING_P),
						g_rgchActiveProcessDescription);
				}
				else
					pszMsg = new_sprintf(FetchString(IDC_MESSAGE_QUITTING));
				SendDlgItemMessage(hwnd, IDC_STATIC_STATUS_MAIN, WM_SETTEXT, 0,
					(LPARAM)pszMsg);
				delete[] pszMsg;
				pszMsg = NULL;
				SendDlgItemMessage(hwnd, IDC_STATIC_STATUS_SUB, WM_SETTEXT, 0,
					(LPARAM)_T(""));
				SendDlgItemMessage(hwnd, IDC_STATIC_STATUS_3RD, WM_SETTEXT, 0,
					(LPARAM)_T(""));
				EnableWindow(GetDlgItem(hwnd, IDCANCEL), false);
				EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_CONTINUE), false);
			}
			// We're done with the user's confirmation, so release the event handle:
			if (hStopRequestInProgress)
			{
				SetEvent(hStopRequestInProgress);
				CloseHandle(hStopRequestInProgress);
			}
			hStopRequestInProgress = NULL;

			// If we're running in the main thread, this dialog has to commit suicide:
			if (fPauseStatusDialog)
			{
				DisplayStatusText(2, _T(""));
				EndDialog(hwnd, 0);
			}

			break;

		case IDC_BUTTON_CONTINUE: // Main thread, user pressed Continue.
			DisplayStatusText(2, _T(""));
			EndDialog(hwnd, 0);
			break;

#ifdef EASTER_EGGS
		case IDC_BUTTON_HIDDEN1:
			JukeBox.PlayNextSong();
			break;
		case IDC_BUTTON_HIDDEN2:
			JukeBox.TogglePause();
			break;
#endif
		}
		break;
	case WM_USER: // Sent from another thread to force this dialog to quit.
		EndDialog(hwnd, 0);
		break;

	case WM_DESTROY: // Dialog is off the screen by now.
		// Signal that our window is destroyed:
		::SetEvent(hDestroyStatusDlg);
		break;

	default: // All the messages we don't handle are handled by Windows.
		return 0;
	}
	return 1; // This means we have processed the message.
}

// Deals with the failure to create a status dialog.
void StatusDialogCreateFailed()
{
	if (MessageBox(NULL, FetchString(IDC_ERROR_CREATE_STATUS_DLG), g_pszTitle,
		MB_OKCANCEL | MB_ICONSTOP) == IDCANCEL)
	{
		g_fStopRequested = true;
	}
	else
	{
		// User wants to continue without the status dialog.
		fContinueWithoutStatusDialog = true;
	}
	// Signal (falsely) that our window is created, so we can continue:
	if (hCreateStatusDlg)
		::SetEvent(hCreateStatusDlg);
}

// Creates a modal dialog in a new thread.
DWORD WINAPI StatusDlgThreadEntry(LPVOID)
{
	// Try to create dialog with animation control:
	if (DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_STATUS), NULL,
		DlgProcStatus, 1) == -1)
	{
		// The dialog with the animation control could not be created. We will assume that the
		// reason is that animation controls are not supported, and try to create a dialog
		// without one:
		if (DialogBoxParam(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_DIALOG_NO_ANIMATION_STATUS), NULL, DlgProcStatus, 0) == -1)
		{
			// Now we're in trouble!
			StatusDialogCreateFailed();
		}
	}
	return 0;
}

// Create and show the Status dialog. Returns the HWND of the static text control.
void ShowStatusDialog()
{
	if (hwndStatusDialog || fContinueWithoutStatusDialog)
		return; // Dialog already present, or we can't create one.

	// It is possible for the Status Dialog to be shown and hidden via another thread.
	// We don't want other threads interfering with this creation, so we'll use a mutex:
	HANDLE hMutex = ::CreateMutex(NULL, false, kszStatusDlgMutexName);

	// Check that the mutex handle didn't already exist:
	if (ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		// Mutex does already exist, so we'll stop our creation:
		::CloseHandle(hMutex);
		hMutex = NULL;
		g_Log.Write(_T("ShowStatusDialog: creation/destruction mutex already exists."));
		return;
	}

	// Create an Event, initially reset so we wait until the window handle is created:
	hCreateStatusDlg = CreateEvent(NULL, true, false, NULL);

	// Create new thread to process dialog message queue independently:
	DWORD nThreadId; // MSDN says you can pass NULL instead of this, but you can't on Win98.
	HANDLE hThread = CreateThread(NULL, 0, StatusDlgThreadEntry, NULL, 0, &nThreadId);

	// Wait upto 15 seconds or until window has a handle:
	if (WaitForSingleObject(hCreateStatusDlg, 15000/*INFINITE*/) == WAIT_TIMEOUT)
	{
		StatusDialogCreateFailed();
	}
	// Restore status texts:
	for (int i = 0; i < s_kctStatusText; i++)
		DisplayStatusText(i, pszStatusTextCopy[i]);

	CloseHandle(hCreateStatusDlg);
	hCreateStatusDlg = NULL;

	// Release mutex:
	::CloseHandle(hMutex);
	hMutex = NULL;
}

BOOL my_OpenClipboard(bool * fCloseStatusDialog)
{
	if (!fCloseStatusDialog)
		return FALSE;

	*fCloseStatusDialog = false;

	if (!hwndStatusDialog)
	{
		ShowStatusDialog();
		*fCloseStatusDialog = true;
	}
	return OpenClipboard(hwndStatusDialog);
}

_TCHAR * StatusTextSnapshot::GetText(int i)
{
	_TCHAR pszBuf[1024];
	SendDlgItemMessage(hwndStatusDialog, ridStatusText[i], WM_GETTEXT, 1024, (LPARAM)pszBuf);
	return my_strdup(pszBuf);
}

StatusTextSnapshot::StatusTextSnapshot()
{
	for (int i = 0; i < 3; i++)
		m_pszText[i] = GetText(i);
}
StatusTextSnapshot::~StatusTextSnapshot()
{
	for (int i = 0; i < 3; i++)
		delete[] m_pszText[i];
}
void StatusTextSnapshot::Repost()
{
	for (int i = 0; i < 3; i++)
		DisplayStatusText(i, m_pszText[i]);
}

// Put formatted message into the status dialog at the given position.
// Caller must NOT delete[] return value, which is the formatted message.
const _TCHAR * DisplayStatusText(int iPosition, const _TCHAR * pszText, ...)
{
	if (iPosition < 0 || iPosition >= s_kctStatusText)
		return _T("");
	if (!pszText)
	{
		SendDlgItemMessage(hwndStatusDialog, ridStatusText[iPosition], WM_SETTEXT, 0,
			(LPARAM)_T(""));
		return _T("");
	}

	// Collect variable arguments and format the message:
	va_list arglist;
	va_start(arglist, pszText);
	_TCHAR * pszStatusText = new_vsprintf(pszText, arglist);

	g_Log.Write(_T("Status text %d: %s"), iPosition, pszStatusText);

	// Put the message in the dialog:
	if (hwndStatusDialog)
	{
		SendDlgItemMessage(hwndStatusDialog, ridStatusText[iPosition], WM_SETTEXT, 0,
			(LPARAM)pszStatusText);
	}
	else
		g_Log.Write(_T("Status Dialog not visible!"));

	// Save the message in case we need to re-write it:
	_TCHAR * pszOld = pszStatusTextCopy[iPosition];
	pszStatusTextCopy[iPosition] = pszStatusText;
	delete[] pszOld;

	// Caller must not delete[] return value:
	return pszStatusText;
}

// In some circumsstances, we need to put up the status dialog and wait until the user
// presses a Continue button. This requires the dialog to run in the main thread.
void PauseOnStatusDialog(int ridStatusText)
{
	// Make sure the dialog isn't running on its own thread:
	HideStatusDialog();

	// Signal that we need a Continue button:
	fPauseStatusDialog = true;
	ridPauseStatusText = ridStatusText;

	// Bypass the creation of a new thread, and launch the dialog directly:
	StatusDlgThreadEntry(NULL); // Does not return till dialog commits suicide.
	hwndStatusDialog = NULL;

	fPauseStatusDialog = false;
}

void HideStatusDialog()
{
	if (!hwndStatusDialog)
		return; // Dialog already absent.

#ifdef EASTER_EGGS
	if (JukeBox.IsActive())
		return;
#endif

	// It is possible for the Status Dialog to be shown and hidden via another thread.
	// We don't want other threads interfering with this destruction, so we'll use a mutex:
	HANDLE hMutex = ::CreateMutex(NULL, false, kszStatusDlgMutexName);

	// Check that the mutex handle didn't already exist:
	if (ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		// Mutex does already exist, so we'll stop our destruction:
		::CloseHandle(hMutex);
		hMutex = NULL;
		g_Log.Write(_T("HideStatusDialog: creation/destruction mutex already exists."));
		return;
	}

	// Create an Event, initially reset so we wait until the window handle is destroyed:
	hDestroyStatusDlg = CreateEvent(NULL, true, false, NULL);

	SendMessage(hwndStatusDialog, WM_USER, 0, 0);

	// Wait until window is destroyed:
	WaitForSingleObject(hDestroyStatusDlg, INFINITE);

	CloseHandle(hDestroyStatusDlg);
	hDestroyStatusDlg = NULL;

	hwndStatusDialog = NULL;

	// Release mutex:
	::CloseHandle(hMutex);
	hMutex = NULL;
}

// Show dialog saying that we're waiting for a reboot, and then freeze.
void PauseForReboot()
{
	HideStatusDialog();
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_PAUSE_FOR_REBOOT), NULL, NULL);
}
