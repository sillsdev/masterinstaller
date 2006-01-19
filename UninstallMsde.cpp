void UninstallMsde()
{
	// Get the installer product code:
	LONG lResult;
	HKEY hKey = NULL;

	try
	{
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Microsoft SQL Server\\SILFW\\Setup", NULL, KEY_READ, &hKey);

		// We don't proceed unless the call above succeeds:
		if (ERROR_SUCCESS != lResult && ERROR_FILE_NOT_FOUND != lResult)
		{
			throw HRESULT_FROM_WIN32(lResult);
		}

		if (ERROR_SUCCESS == lResult)
		{
			const int knProductCodeLen = 256;
			char szProductCode[knProductCodeLen];
			DWORD cbData = knProductCodeLen;

			lResult = RegQueryValueEx(hKey, "ProductCode", NULL, NULL, (LPBYTE)szProductCode,
				&cbData);
			RegCloseKey(hKey);

			if (ERROR_SUCCESS == lResult)
			{
				char szCommand[100];
				sprintf(szCommand, "MsiExec.exe /X%s /qb-", szProductCode);
				ExecCmd(szCommand, false);
				return;
			}
		}
	}
	catch (...)
	{
	}
	// report error?
}
