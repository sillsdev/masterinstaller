#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "ErrorHandler.h"
#include "Resource.h"
#include "LogFile.h"
#include "UsefulStuff.h"
#include "Dialogs.h"
#include "Globals.h"
#include "ProductManager.h"

// Display given error message, and additional info as necessary. If fFatal is set to true, this
// function does not return.
void HandleError(vErrorType vType, bool fCheckDependentStatus, int nErrorTextId, ...)
{
	// We will be passing on the variable arguments to the vsprintf() function:
	va_list arglist;
	va_start(arglist, nErrorTextId);

	// Initial MessageBox button:
	UINT uMbType = MB_OK;

	// Check if the first variable argument is the text to be copied to the clipboard:
	const TCHAR * pszCopyText = NULL;
	if (vType & kWithCopy)
	{
		pszCopyText = va_arg(arglist, TCHAR *);
		uMbType = MB_YESNOCANCEL;
	}

	// Get main error message and format it with variable arguments:
	_TCHAR * pszFormattedMsg = new_vsprintf(FetchString(nErrorTextId), arglist);


	// If error is fatal, add text to say so:
	if (vType == kNonFatal)
		uMbType |= MB_ICONINFORMATION;
	else
	{
		uMbType |= MB_ICONSTOP;
		if (vType == kFatal)
		{
			HideStatusDialog();
			new_sprintf_concat(pszFormattedMsg, 2, FetchString(IDC_ERROR_IS_FATAL));
		}
	}
	if (vType == kUserAbort)
		HideStatusDialog();

	// Display the message.
	_TCHAR * pszTitle = new_sprintf(_T("%s - %s"), g_pszTitle, FetchString(IDC_ERROR));
	int nResponse = MessageBox(NULL, pszFormattedMsg, pszTitle, uMbType);
	delete[] pszTitle;
	pszTitle = NULL;

	g_Log.Write(_T("Error message displayed:"));
	g_Log.Write(pszFormattedMsg);

	delete[] pszFormattedMsg;
	pszFormattedMsg = NULL;

	// See if we are to write data to Clipboard:
	if (nResponse == IDYES)
		WriteClipboardText(pszCopyText);

	try
	{
		if (fCheckDependentStatus)
		{
			// See if any dependencies remain uninstalled:
			IProductManager * ppmProductManager = CreateProductManager();
			if (ppmProductManager)
			{
				ppmProductManager->ShowFinalReport();
				DestroyProductManager(ppmProductManager);
			}
		}
	}
	catch (...)
	{
		// If another error occurred during the attempt to show a disaster report, bail out!
		throw 1;
	}
	if (vType == kFatal)
	{
		throw 1;
	}
	if (vType == kUserAbort)
	{
		throw UserQuitException;
	}
	return;
}
