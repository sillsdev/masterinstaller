#pragma once

#include <tchar.h>
#include <Shlobj.h>

#include "Uninstall.cpp"

// Remove any existing version of Encoding Converters
int RemovePreviousECs(const TCHAR * /*pszCriticalFile*/)
{
	int nResult = 0;

	// Search for Bob Eaton's uninstall batch file:
	g_Log.Write(_T("Looking in registry for Bob's uninstall batch file path..."));
	g_Log.Indent();
	TCHAR * pszUninstallFolder = NULL;
	HKEY hKey;
	const TCHAR * pszKeyPath = _T("SOFTWARE\\SIL\\SilEncConverters22\\Installer");
	const TCHAR * pszKeyValue = _T("InstallerPath");
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
		pszKeyPath, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		g_Log.Write(_T("Opened reg key."));
		DWORD cbData = 0;		
		lResult = RegQueryValueEx(hKey, pszKeyValue, NULL, NULL, NULL, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			g_Log.Write(_T("Read required buffer size."));
			cbData++;
			pszUninstallFolder = new TCHAR [cbData];
			lResult = RegQueryValueEx(hKey, pszKeyValue, NULL, NULL, (LPBYTE)pszUninstallFolder,
				&cbData);
			if (ERROR_SUCCESS == lResult)
				g_Log.Write(_T("Got path: '%s'."), pszUninstallFolder);
			else
			{
				g_Log.Write(_T("Could not read path."));
				delete[] pszUninstallFolder;
				pszUninstallFolder = NULL;
			}
		}
		RegCloseKey(hKey);
	}

	// If Bob Eaton's uninstall link exists, run it:
	if (pszUninstallFolder)
	{
		const TCHAR * pszBatchFile = _T("UninstallSC.bat");

		HideStatusDialog();
		g_Log.Write(_T("Informing user that Bob's older Encoding Converters exist."));
		MessageBox(NULL,
			_T("The existing version of SILConverters must be upgraded for this installation. Setup will now uninstall the old version before installing the new version. Shortly into that process, you will see a dialog box that asks whether you want to remove all of your converters. You will want to answer \"No\" to that question."),
			g_pszTitle,
			MB_ICONINFORMATION | MB_OK);
		ShowStatusDialog();

		DisplayStatusText(0, _T("Removing obsolete Encoding Converters programs."));
		DisplayStatusText(1, _T("To preserve your existing converters, answer \"no\""));
		DisplayStatusText(2, _T("when asked if you want to remove them."));

		TCHAR * pszApplication = new_sprintf(_T("%s\\%s"), pszUninstallFolder, pszBatchFile);
		g_Log.Write(_T("Application is '%s'"), pszApplication);
		g_Log.Write(_T("Working directory is '%s'"), pszUninstallFolder);

		// Set up data for creating new process:
		BOOL bReturnVal = false;
		STARTUPINFO si;
		DWORD dwExitCode =  0;
		PROCESS_INFORMATION process_info;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_MINIMIZE;

		// Launch new process. The CREATE_SEPARATE_WOW_VDM should be ignored for 32-bit programs,
		// and also when running on Windows 98, but it is essential for 16-bit programs running on
		// Windows 2000 or later, else we cannot easily monitor when termination occurs:
		TCHAR * pszQuotedCommand = new_sprintf(_T("\"%s\""), pszApplication);
		g_Log.Write(_T("Quoted command is %s"), pszQuotedCommand);
		bReturnVal = CreateProcess(NULL, (LPTSTR)pszQuotedCommand, NULL, NULL, false,
			CREATE_SEPARATE_WOW_VDM, NULL, pszUninstallFolder, &si, &process_info);
		delete[] pszQuotedCommand;
		pszQuotedCommand = NULL;

		if (bReturnVal)
		{
			CloseHandle(process_info.hThread);

			g_Log.Write(_T("Waiting for '%s' to finish."), pszApplication);

			// New code based on Microsoft support article 824042:
			// http://support.microsoft.com/kb/824042
			WaitForInputIdle(process_info.hProcess, INFINITE);
			WaitForSingleObject(process_info.hProcess, INFINITE);

			// Get the exit code:
			GetExitCodeProcess(process_info.hProcess, &dwExitCode);

			g_Log.Write(_T("'%s' finished with exit code %d."), pszApplication, dwExitCode);

			// If the result of executing the process was that a reboot was triggered, we'll
			// inform the user and just wait:
			if (ERROR_SUCCESS_REBOOT_INITIATED == dwExitCode)
				PauseForReboot();

			CloseHandle(process_info.hProcess);
		}
		else
		{
			g_Log.Write(_T("Could not create process for '%s'."), pszApplication);
			delete[] pszApplication;
			return -2;
		}
		delete[] pszUninstallFolder;
		pszUninstallFolder = NULL;
	}
	else
		g_Log.Write(_T("Did not read uninstall batch file folder."));

	g_Log.Unindent();
	g_Log.Write(_T("...Done looking for Bob's uninstall batch file path."));

	// Now clear up any other installations that we know about.
	// List of all previous EC product codes:
	const TCHAR * pszProductCode[] = 
	{
		_T("{25AEB008-3E0B-4057-A66B-2F3C50228E0E}"), // EC Version 2.2.1 (with FW 3.1)
		_T("{99DA3CBB-CE9C-4861-BBB6-7FD36077EA70}"), // Clipboard EC 2.2
		_T("{26001E4D-320E-4162-8843-A798C973E3E2}"), // Clipboard EC shipped with FW 3.1
		_T("{9D80029B-5B2C-45A1-9C8A-D599598CCAFF}"), // Bulk SFM converter 1.1.1
		_T("{EB74F31A-3768-4884-B23C-332A6E0E3B07}"), // SpellFixer add-in 2.2.1
		_T("{C4BEF638-52A5-4480-96BB-B54BDDF566D8}"), // EC Version 1.0.2 (with FW 3.0)
		_T("{1754401C-BEBE-415E-B0DF-9B6E0420E2F8}"), // Clipboard EC shipped with FW 3.0
	};

	for (int i = 0; i < (sizeof(pszProductCode) / sizeof(pszProductCode[0])); i++)
	{
		g_Log.Write(_T("Uninstalling earlier EC (Product code %s)."), pszProductCode[i]);
		DWORD dwResult = Uninstall(pszProductCode[i], _T("Removing obsolete Encoding Converters programs"));
		if (dwResult == 0)
			g_Log.Write(_T("Uninstall succeeded."));
		else
		{
			g_Log.Write(_T("Uninstall failed with error code %d."), dwResult);
			nResult = (int)dwResult;
			return nResult;
		}
	}
	return nResult;
}
