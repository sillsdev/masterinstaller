#pragma once

#include <tchar.h>
#include <Shlobj.h>

#include "Uninstall.cpp"

// Remove any existing version of Encoding Converters
int RemovePreviousECs(const TCHAR * /*pszCriticalFile*/)
{
	int nResult = 0;

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
	// Link to Bob Eaton's uninstall batch file:
	TCHAR * pszUninstallLink = NULL;

	// See if Bob Eaton's uninstall batch file is present:
	bool fFoundSomething = false;
	HRESULT hResult;
	TCHAR szFolder[MAX_PATH];
	const TCHAR * pszLink = _T("SIL Converters\\Uninstall SILConverters.lnk");

	g_Log.Write(_T("Searching for Bob's uninstall batch file link:"));

	hResult = SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_DEFAULT, szFolder);
	if (hResult == S_OK)
	{
		pszUninstallLink = new_sprintf(_T("%s\\%s"), szFolder, pszLink);
		FILE * f = NULL;
		if (_tfopen_s(&f, pszUninstallLink, _T("r")) == 0)
		{
			g_Log.Write(_T("Found '%s'."), pszUninstallLink);
			fFoundSomething = true;
			fclose(f);
		}
		else
		{
			g_Log.Write(_T("Not found '%s'."), pszUninstallLink);
			delete[] pszUninstallLink;
			pszUninstallLink = NULL;
		}
	}
	if (!fFoundSomething)
	{
		hResult = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_DEFAULT, szFolder);
		if (hResult == S_OK)
		{
			pszUninstallLink = new_sprintf(_T("%s\\%s"), szFolder, pszLink);
			FILE * f = NULL;
			if (_tfopen_s(&f, pszUninstallLink, _T("r")) == 0)
			{
				g_Log.Write(_T("Found '%s'."), pszUninstallLink);
				fFoundSomething = true;
				fclose(f);
			}
			else
			{
				g_Log.Write(_T("Not found '%s'."), pszUninstallLink);
				delete[] pszUninstallLink;
				pszUninstallLink = NULL;
			}
		}
	}

	if (!fFoundSomething)
	{
		// Detect if any known modules are installed:
		g_Log.Write(_T("Searching product codes for older Encoding Converters:"), pszUninstallLink);
		for (int i = 0; !fFoundSomething && (i < (sizeof(pszProductCode) / sizeof(pszProductCode[0]))); i++)
		{
			INSTALLSTATE state = WindowsInstaller.MsiQueryProductState(pszProductCode[i]);
			switch (state)
			{
			case INSTALLSTATE_ADVERTISED:
			case INSTALLSTATE_LOCAL:
			case INSTALLSTATE_SOURCE:
			case INSTALLSTATE_DEFAULT:				
				fFoundSomething = true;
				g_Log.Write(_T("Found %s."), pszProductCode[i]);
			}
		}
	}

	if (fFoundSomething)
	{
		HideStatusDialog();
		g_Log.Write(_T("Informing user that older Encoding Converters exist."));
		if (MessageBox(NULL,
			_T("An older version of the SIL Encoding Converters is installed on this machine. It must be removed before the installation process can continue. A newer version of the Encoding Converters enigine will be installed. Click Yes to continue and uninstall the older version. Click No to quit this installation."),
			g_pszTitle,
			MB_ICONSTOP | MB_YESNO) == IDNO)
		{
			g_Log.Write(_T("User chose to quit."));
			return -999;
		}
		g_Log.Write(_T("User chose to continue with uninstallation of older Encoding Converters."));
		ShowStatusDialog();
		g_Log.Write(DisplayStatusText(0, _T("Removing obsolete Encoding Converters programs.")));
		g_Log.Write(DisplayStatusText(1, _T("Please follow any instructions on uninstallation dialogs.")));
		g_Log.Write(DisplayStatusText(2, _T("")));

		// If Bob Eaton's uninstall link exists, run it:
		if (pszUninstallLink)
		{
			g_Log.Write(_T("Reading '%s'."), pszUninstallLink);

			// Use the Shell to find out the application path enbedded in the link:
			const int cchApplication = 1024;
			TCHAR szApplication[cchApplication] = { 0 };
			const int cchCmdArguments = INFOTIPSIZE;
			TCHAR szCmdArguments[cchCmdArguments] = { 0 };
			const int cchWorkingDirectory = 1024;
			TCHAR szWorkingDirectory[cchWorkingDirectory] = { 0 };
			IShellLink* psl;
			HRESULT hr;
			CoInitialize(NULL);

			// Create the ShellLink object:
			hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
						IID_IShellLink, (LPVOID*) &psl);

			if (SUCCEEDED(hr))
			{
				IPersistFile* ppf;
				// Bind the ShellLink object to the Persistent File:
				hr = psl->QueryInterface(IID_IPersistFile, (LPVOID *) &ppf);
				if (SUCCEEDED(hr))
				{
#ifdef UNICODE
					// Read the link into the persistent file:
					hr = ppf->Load(pszUninstallLink, 0);
#else
					WORD wsz[MAX_PATH];
					// Get a UNICODE wide string wsz from the Link path
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszUninstallLink, -1, (LPWSTR)wsz, MAX_PATH);
					// Read the link into the persistent file:
					hr = ppf->Load((LPCOLESTR)wsz, 0);
#endif
					if (SUCCEEDED(hr))
					{
						// Read the target information from the link object
						// UNC paths are supported (SLGP_UNCPRIORITY)
						psl->GetPath(szApplication, cchApplication, NULL, SLGP_UNCPRIORITY);
						psl->GetArguments(szCmdArguments, cchCmdArguments);
						psl->GetWorkingDirectory(szWorkingDirectory, cchWorkingDirectory);
						g_Log.Write(_T("Application is '%s'"), szApplication);
						g_Log.Write(_T("Command line arguments are '%s'"), szCmdArguments);
						g_Log.Write(_T("Working directory is '%s'"), szWorkingDirectory);
					}
				}
				psl->Release();
			}

			if (_tcslen(szApplication) > 0)
			{
				// Set up data for creating new process:
				BOOL bReturnVal = false;
				STARTUPINFO si;
				DWORD dwExitCode =  0;
				PROCESS_INFORMATION process_info;

				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);

				// Launch new process. The CREATE_SEPARATE_WOW_VDM should be ignored for 32-bit programs,
				// and also when running on Windows 98, but it is essential for 16-bit programs running on
				// Windows 2000 or later, else we cannot easily monitor when termination occurs:
				TCHAR * pszFullCommand = new_sprintf(_T("\"%s\" %s"), szApplication, szCmdArguments);
				g_Log.Write(_T("Full command is %s"), pszFullCommand);
				bReturnVal = CreateProcess(NULL, (LPTSTR)pszFullCommand, NULL, NULL, false,
					CREATE_SEPARATE_WOW_VDM, NULL, szWorkingDirectory, &si, &process_info);
				delete[] pszFullCommand;

				if (bReturnVal)
				{
					CloseHandle(process_info.hThread);

					g_Log.Write(_T("Waiting for '%s' to finish."), szApplication);

					// New code based on Microsoft support article 824042:
					// http://support.microsoft.com/kb/824042
					WaitForInputIdle(process_info.hProcess, INFINITE);
					WaitForSingleObject(process_info.hProcess, INFINITE);

					// Get the exit code:
					GetExitCodeProcess(process_info.hProcess, &dwExitCode);

					g_Log.Write(_T("'%s' finished with exit code %d."), szApplication, dwExitCode);

					// If the result of executing the process was that a reboot was triggered, we'll
					// inform the user and just wait:
					if (ERROR_SUCCESS_REBOOT_INITIATED == dwExitCode)
						PauseForReboot();

					CloseHandle(process_info.hProcess);
				}
				else
				{
					g_Log.Write(_T("Could not create process for '%s'."), szApplication);
					delete[] pszUninstallLink;
					pszUninstallLink = NULL;
					return -2;
				}
			}
			else
				g_Log.Write(_T("Could not read application path from '%s'."), pszUninstallLink);
			delete[] pszUninstallLink;
			pszUninstallLink = NULL;
		}

		// Now clear up any other installations that we know about:
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
	}
	return nResult;
}
