#pragma once

#include <tchar.h>
#include <Shlobj.h>

#include "Uninstall.cpp"
#include "SearchOtherUsersInstallations.cpp"

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
		// Form full path to uninstall batch file:
		const TCHAR * pszBatchFile = _T("UninstallSC.bat");
		TCHAR * pszApplication = new_sprintf(_T("%s\\%s"), pszUninstallFolder, pszBatchFile);

		// If the batch file is not there, we will assume the EC installation is version 2.5 or later,
		// so we will only proceed if it is there:
		FILE * f;
		if (_tfopen_s(&f, pszApplication, _T("r")) == 0)
		{
			fclose(f);

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
		}
		delete[] pszApplication;
		pszApplication = NULL;
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
		_T("{99DA3CBB-CE9C-4861-BBB6-7FD36077EA70}"), // Clipboard EC 2.2
		_T("{9D80029B-5B2C-45A1-9C8A-D599598CCAFF}"), // Bulk SFM converter 1.1.1
		_T("{EB74F31A-3768-4884-B23C-332A6E0E3B07}"), // SpellFixer add-in 2.2.1
		_T("{C4BEF638-52A5-4480-96BB-B54BDDF566D8}"), // EC Version 1.0.2 (with FW 3.0)
		_T("{1754401C-BEBE-415E-B0DF-9B6E0420E2F8}"), // Clipboard EC shipped with FW 3.0
		_T("{25AEB008-3E0B-4057-A66B-2F3C50228E0E}"), // EC Version 2.2.1 (with FW 3.1)
		_T("{26001E4D-320E-4162-8843-A798C973E3E2}"), // Clipboard EC shipped with FW 3.1
		_T("{21494256-771F-451F-94F9-8A52AC52BFB2}"), // EC Version 2.3
		_T("{6242D132-64F4-46E3-9F39-26B02215B896}"), // EC Version 2.5 shipped with FW 4.0
		_T("{AA48C03E-F339-430B-B8E9-FA666F607C17}"), // EC Version 2.6 produced with FW 4.2
		_T("{DCDE04DF-4429-44E3-939F-E618A8106DC7}"), // EC Version 2.6.1 produced with FW 5.0
//		_T("{287F6ADF-DA8A-46fc-8BE0-351AA6412B79}"), // EC Version 3.0 produced with FW 5.4
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

	// The previous uninstaller did not remove a couple of items from the Start menu in folder
	// C:\Documents and Settings\<user name>\Start Menu\Programs\SIL Converters, so we must remove
	// them ourselves.
	// Unfortunately, there is no CSIDL for C:\Documents and Settings\<user name>\Start Menu\Programs,
	// so we must go one deeper, then chop off back to the backslash:
	g_Log.Write(_T("Looking for unremoved shortcuts in Start Menu..."));
	g_Log.Indent();
	TCHAR * pszStartMenuProgramsStartupPath = GetFolderPathNew(CSIDL_STARTUP);

	if (pszStartMenuProgramsStartupPath)
	{
		g_Log.Write(_T("Initial folder is '%s'"), pszStartMenuProgramsStartupPath);
	
		// Chop off the last folder component:
		TCHAR * ch = _tcsrchr(pszStartMenuProgramsStartupPath, '\\');
		if (ch)
			*ch = 0;

		// Add the rest of the path:
		new_sprintf_concat(pszStartMenuProgramsStartupPath, 0, _T("\\SIL Converters"));
		g_Log.Write(_T("Deleting *.* in '%s'"), pszStartMenuProgramsStartupPath);

		// Prepare the search string, with an extra zero on the end, formed first with an '@'
		// then changed:
		new_sprintf_concat(pszStartMenuProgramsStartupPath, 0, _T("\\*.*@"));
		ch = _tcsrchr(pszStartMenuProgramsStartupPath, '@');
		if (ch)
			*ch = 0;

		SHFILEOPSTRUCT sfos;
		sfos.hwnd = NULL;
		sfos.wFunc = FO_DELETE;
		sfos.pFrom = pszStartMenuProgramsStartupPath;
		sfos.pTo = NULL;
		sfos.fFlags = FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		SHFileOperation(&sfos);
	}
	else
		g_Log.Write(_T("Could not retrieve Startup folder path"));

	delete[] pszStartMenuProgramsStartupPath;
	pszStartMenuProgramsStartupPath = NULL;

	g_Log.Unindent();
	g_Log.Write(_T("...Done"));
	
	// Finally, see if any of the above products were installed under other user's accounts:
	g_Log.Write(_T("Searching for installations in other user accounts..."));
	g_Log.Indent();
	int cIssues = 0; // Number of issues found.
	TCHAR * pszReport = SearchOtherUsersInstallations(pszProductCode,
		sizeof(pszProductCode) / sizeof(pszProductCode[0]),
		_T("Application connected with SIL Encoding Converters"), cIssues);
	g_Log.Unindent();
	g_Log.Write(_T("...done."));
	if (pszReport)
	{
		TCHAR * pszMessage = NULL;
		if (cIssues == 1)
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there is a program installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under another user's account which is incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user, then uninstall the program "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		else
		{
			new_sprintf_concat(pszMessage, 0, _T("Unfortunately, there are programs installed on this computer "));
			new_sprintf_concat(pszMessage, 0, _T("under other users' accounts which are incompatible with this "));
			new_sprintf_concat(pszMessage, 0, _T("installation. The details are listed below. You must log on as "));
			new_sprintf_concat(pszMessage, 0, _T("the specified user in each case, then uninstall the programs "));
			new_sprintf_concat(pszMessage, 0, _T("using the Add or Remove Programs facility in Control Panel."));
		}
		new_sprintf_concat(pszMessage, 0, _T(" If this is not possible, check out alternatives in the "));
		new_sprintf_concat(pszMessage, 0, _T("Installation Issues section of the SetupFW.rtf "));
		new_sprintf_concat(pszMessage, 0, _T("file in the FieldWorks folder of this CD (or web download)."));

		new_sprintf_concat(pszMessage, 1, _T("%s"), pszReport);
		delete[] pszReport;
		new_sprintf_concat(pszMessage, 2, _T("This installation will now terminate."));
		HideStatusDialog();
		MessageBox(NULL, pszMessage, g_pszTitle, MB_ICONSTOP | MB_OK);
		g_Log.Write(_T("Found installations in other user accounts - reported: %s"), pszMessage);
		nResult = -2;
	}

	return nResult;
}
