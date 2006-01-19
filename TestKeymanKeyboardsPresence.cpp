// If fTestOnly is true, tests to see if the keyman keyboards in pszKeyboardsPath are installed.
// If fTestOnly is false, installs the keyman keyboards from pszKeyboardsPath.
DWORD KeymanFunction(bool fTestOnly, const char * pszKeyboardsPath)
{
	const char * pszTitle = "Keyman Keyboard Installer";
	const char * pszErrReg = "Cannot find Keyman shell. Keyboards will not be installed.";
	const char * pszErrNoFiles = "No Keyman files found in specified folder.";
	const char * pszErrLaunchKeyman = "Could not run Keyman to install keyboards.";

	HKEY hKey;
	const int ctchShellCmd = 300;
	char pszShellCmd[ctchShellCmd];

	if (!fTestOnly)
	{
		// Locate Keyman Shell:
		LONG lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
			"Keyman.File.Package\\Shell\\open\\Command", 0, KEY_READ, &hKey);

		if (ERROR_SUCCESS != lResult)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Read the Keyman shell command line:
		DWORD cbData = ctchShellCmd;
		lResult = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)pszShellCmd, &cbData);
		RegCloseKey(hKey);
		hKey = NULL;
		if (ERROR_SUCCESS != lResult)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Look for the "-i" part of the shell command:
		char *ch = strstr(pszShellCmd, "-i");
		if (!ch)
		{
			MessageBox(NULL, pszErrReg, pszTitle, MB_ICONSTOP | MB_OK);
			return 0;
		}

		// Remove everything after the "-i":
		ch += 2;
		*ch = 0;
	}

	// Using path of .exe  plus pszKeyboardsPath as the source path, find
	// (and install if !fTestOnly) all .kmx files:
	const int knLen = MAX_PATH + 20;
	char pszSearch[knLen];
	char pszFolder[knLen];
	GetModuleFileName(NULL, pszSearch, knLen);
	char * pszFolderEnd = strrchr(pszSearch, '\\');
	if (pszFolderEnd)
		*pszFolderEnd = 0;
	strcat(pszSearch, "\\");
	strcat(pszSearch, pszKeyboardsPath);
	strcpy(pszFolder, pszSearch);
	strcat(pszSearch, "\\*.kmx");

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(pszSearch, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, pszErrNoFiles, pszTitle, MB_ICONSTOP | MB_OK);
		return 0;
	}
	do
	{
		// See if Keyboard is already installed:
		bool fKeyboardExists = false;
		char pszRegKey[1000];
		strcpy(pszRegKey, "Software\\Tavultesoft\\Keyman\\6.0\\Active Keyboards\\");
		strcat(pszRegKey, wfd.cFileName);
		// Remove file extension:
		char * ch = strrchr(pszRegKey, '.');
		if (ch)
			*ch = 0;
		HKEY hKeyTemp;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszRegKey, 0, KEY_READ, &hKeyTemp))
		{
			fKeyboardExists = true;
			RegCloseKey(hKeyTemp);
			hKeyTemp = NULL;
		}

		if (!fKeyboardExists)
		{
			if (fTestOnly)
			{
				FindClose(hFind);
				return -1; // At least one keyboard is not installed.
			}

			// Make up command line to install current keyboard:
			char pszCmd[1000];
			sprintf(pszCmd, "%s -s \"%s\\%s\"", pszShellCmd, pszFolder, wfd.cFileName);

			// Execute command:
			BOOL bReturnVal = false;
			STARTUPINFO si;
			DWORD dwExitCode =	0;
			PROCESS_INFORMATION	process_info;

			ZeroMemory(&si,	sizeof(si));
			si.cb =	sizeof(si);

			// Launch new process:
			bReturnVal = CreateProcess(NULL, (LPTSTR)pszCmd, NULL, NULL, false, 0, NULL, NULL, &si,
				&process_info);

			if (!bReturnVal)
			{
				MessageBox(NULL, pszErrLaunchKeyman, pszTitle, MB_ICONSTOP | MB_OK);
				return 0;
			}

			CloseHandle(process_info.hThread);
			// Wait for Keyman to finish:
			WaitForSingleObject(process_info.hProcess, INFINITE);
			CloseHandle(process_info.hProcess);
		} // End if !fKeyboardExists
	} while (FindNextFile(hFind, &wfd));
	
	FindClose(hFind);
	hFind = NULL;

	return 0;
}

bool TestKeymanKeyboardsPresence(const char * /*pszMinVersion*/, const char * /*pszMaxVersion*/, const char * pszCriticalFile)
{
	return (0 == KeymanFunction(true, pszCriticalFile));
}
