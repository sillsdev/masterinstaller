#pragma once

#include <tchar.h>
#if !defined _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#define MY_NEW_DEF_WIN32_WINNT
#endif
#include <Sddl.h>
#ifdef MY_NEW_DEF_WIN32_WINNT
#undef _WIN32_WINNT
#undef MY_NEW_DEF_WIN32_WINNT
#endif

// Search for installations of given products on other user's accounts.
// Returns a report detailing which user has which product, or NULL if none found.
// Caller must delete[] the returned value.
TCHAR * SearchOtherUsersInstallations(const TCHAR ** pszProductCode, int cProductCode,
									  const TCHAR * pszDefaultName, int & cIssues)
{
	TCHAR * pszReport = NULL;
	
	// Prepare for dynamic loading of AdvApi32.dll, and use of functions which will be missing
	// on Windows 98:
	typedef BOOL (WINAPI * LookupAccountSidFn)(LPCTSTR lpSystemName, PSID lpSid, LPTSTR lpName,
		LPDWORD cchName, LPTSTR lpReferencedDomainName, LPDWORD cchReferencedDomainName,
		PSID_NAME_USE peUse);
	LookupAccountSidFn _LookupAccountSid;
	typedef BOOL (WINAPI * ConvertStringSidToSidFn)(LPCTSTR StringSid, PSID* Sid);
	ConvertStringSidToSidFn _ConvertStringSidToSid;

	HMODULE hmodAdvApi = LoadLibrary(_T("ADVAPI32.DLL"));
	if (hmodAdvApi)
	{
		// Now get pointers to the functions we want to use:
#ifdef UNICODE
		_LookupAccountSid = (LookupAccountSidFn)GetProcAddress(hmodAdvApi, "LookupAccountSidW");
		_ConvertStringSidToSid = (ConvertStringSidToSidFn)GetProcAddress(hmodAdvApi, "ConvertStringSidToSidW");
#else
		_LookupAccountSid = (LookupAccountSidFn)GetProcAddress(hmodAdvApi, "LookupAccountSidA");
		_ConvertStringSidToSid = (ConvertStringSidToSidFn)GetProcAddress(hmodAdvApi, "ConvertStringSidToSidA");
#endif
	}
	if (!_LookupAccountSid)
		g_Log.Write(_T("LookupAccountSid function not found"));
	if (!_ConvertStringSidToSid)
		g_Log.Write(_T("ConvertStringSidToSid function not found"));

	for (int i = 0; i < cProductCode; i++)
	{
		g_Log.Write(_T("Searching for Product code %s."), pszProductCode[i]);

		// Form mangled string of product code - for example:
		// {25AEB008-3E0B-4057-A66B-2F3C50228E0E} is represented by 800BEA52B0E375046AB6F2C30522E8E0.
		TCHAR szMangledCode[33];
		szMangledCode[32] = 0;

		szMangledCode[0] = pszProductCode[i][8];
		szMangledCode[1] = pszProductCode[i][7];
		szMangledCode[2] = pszProductCode[i][6];
		szMangledCode[3] = pszProductCode[i][5];
		szMangledCode[4] = pszProductCode[i][4];
		szMangledCode[5] = pszProductCode[i][3];
		szMangledCode[6] = pszProductCode[i][2];
		szMangledCode[7] = pszProductCode[i][1];

		szMangledCode[8] = pszProductCode[i][13];
		szMangledCode[9] = pszProductCode[i][12];
		szMangledCode[10] = pszProductCode[i][11];
		szMangledCode[11] = pszProductCode[i][10];

		szMangledCode[12] = pszProductCode[i][18];
		szMangledCode[13] = pszProductCode[i][17];
		szMangledCode[14] = pszProductCode[i][16];
		szMangledCode[15] = pszProductCode[i][15];

		szMangledCode[16] = pszProductCode[i][21];
		szMangledCode[17] = pszProductCode[i][20];
		szMangledCode[18] = pszProductCode[i][23];
		szMangledCode[19] = pszProductCode[i][22];

		szMangledCode[20] = pszProductCode[i][26];
		szMangledCode[21] = pszProductCode[i][25];
		szMangledCode[22] = pszProductCode[i][28];
		szMangledCode[23] = pszProductCode[i][27];
		szMangledCode[24] = pszProductCode[i][30];
		szMangledCode[25] = pszProductCode[i][29];
		szMangledCode[26] = pszProductCode[i][32];
		szMangledCode[27] = pszProductCode[i][31];
		szMangledCode[28] = pszProductCode[i][34];
		szMangledCode[29] = pszProductCode[i][33];
		szMangledCode[30] = pszProductCode[i][36];
		szMangledCode[31] = pszProductCode[i][35];

		g_Log.Write(_T("Mangled code = %s."), szMangledCode);

		// Search HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData for users
		HKEY hKey = NULL;
		const TCHAR * pszUserData = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData");

		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszUserData, NULL, KEY_READ, &hKey))
		{
			// Get number of sub keys, and length of longest:
			DWORD cchMaxSubKey; // longest subkey size
			DWORD cSubKeys = 0; // number of subkeys 
			if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys,
				&cchMaxSubKey, NULL, NULL, NULL, NULL, NULL, NULL))
			{
				cchMaxSubKey++;
				TCHAR * pszUserId = new TCHAR [1 + cchMaxSubKey];
				for (DWORD iKey = 0; iKey < cSubKeys; iKey++)
				{
					DWORD cbData = cchMaxSubKey;
					if (ERROR_SUCCESS == RegEnumKeyEx(hKey, iKey, pszUserId, &cbData, NULL, NULL,
						NULL, NULL))
					{
						// With this user, see if key Products\<Mangled Code>\InstallProperties
						// exists:
						TCHAR * pszMangledKey = new_sprintf(_T("%s\\Products\\%s\\InstallProperties"),
							pszUserId, szMangledCode);
						HKEY hkMangled;

						if (ERROR_SUCCESS == RegOpenKeyEx(hKey, pszMangledKey, NULL,
							KEY_READ, &hkMangled))
						{
							g_Log.Write(_T("Found registry key %s."), pszMangledKey);
							cIssues++;

							// We have found our current product in a user account. Now collect
							// application name and version, starting with required buffer size:
							DWORD cchAppName = 0;
							TCHAR * pszAppName = NULL;
							const TCHAR * pszAppValue = _T("DisplayName");

							DWORD cchAppVersion = 0;
							TCHAR * pszAppVersion = NULL;
							const TCHAR * pszVersionValue = _T("DisplayVersion");

							if (ERROR_SUCCESS == RegQueryValueEx(hkMangled, pszAppValue,
								NULL, NULL, NULL, &cchAppName))
							{
								pszAppName = new TCHAR [cchAppName];
								RegQueryValueEx(hkMangled, pszAppValue, NULL, NULL,
									(LPBYTE)pszAppName, &cchAppName);
							}
							if (pszAppName)
								g_Log.Write(_T("Application name = %s"), pszAppName);
							else
								g_Log.Write(_T("Could not retrieve application name."));

							if (ERROR_SUCCESS == RegQueryValueEx(hkMangled, pszVersionValue,
								NULL, NULL, NULL, &cchAppVersion))
							{
								pszAppVersion = new TCHAR [cchAppVersion];
								RegQueryValueEx(hkMangled, pszVersionValue, NULL, NULL,
									(LPBYTE)pszAppVersion, &cchAppVersion);
							}
							if (pszAppVersion)
								g_Log.Write(_T("Application version = %s"), pszAppVersion);
							else
								g_Log.Write(_T("Could not retrieve application version."));

							RegCloseKey(hkMangled);
							hkMangled = NULL;

							// Get the user's log-on name:
							PSID psid;
							TCHAR * pszLogon = NULL;
							DWORD cchLogon = 0;
							TCHAR * pszDomain = NULL;
							DWORD cchDomain = 0;
							SID_NAME_USE snu;
							try
							{
								if (_ConvertStringSidToSid != NULL && _LookupAccountSid != NULL)
								{
									if (_ConvertStringSidToSid(pszUserId, &psid) != 0)
									{
										_LookupAccountSid(NULL, psid, NULL, &cchLogon, NULL, &cchDomain, &snu);
										{
											pszLogon = new TCHAR [cchLogon];
											pszDomain = new TCHAR [cchDomain];
											_LookupAccountSid(NULL, psid, pszLogon, &cchLogon, pszDomain, &cchDomain, &snu);
										}
										LocalFree(psid);
									}
								}
							}
							catch (...)
							{
								delete[] pszLogon;
								pszLogon = NULL;
								delete[] pszDomain;
								pszDomain = NULL;
							}
							if (pszLogon)
								g_Log.Write(_T("User name = %s"), pszLogon);
							else
								g_Log.Write(_T("Could not retrieve user name."));
							if (pszDomain)
								g_Log.Write(_T("Domain name = %s"), pszDomain);
							else
								g_Log.Write(_T("Could not retrieve domain name."));

							// Append details to report:
							new_sprintf_concat(pszReport, 1,
								_T("%s"), pszAppName ? pszAppName : pszDefaultName);
							if (pszAppVersion)
							{
								new_sprintf_concat(pszReport, 0, _T(" <version %s>"),
									pszAppVersion);
							}
							new_sprintf_concat(pszReport, 0, _T(" installed by "));
							if (pszLogon)
							{
								if (pszDomain)
								{
									new_sprintf_concat(pszReport, 0, _T("%s\\%s"), pszDomain,
										pszLogon);
								}
								else
									new_sprintf_concat(pszReport, 0, _T("%s"), pszLogon);
							}
							else
								new_sprintf_concat(pszReport, 0, _T("unknown user"));

							delete[] pszLogon;
							pszLogon = NULL;
							delete[] pszDomain;
							pszDomain = NULL;
							delete[] pszAppVersion;
							pszAppVersion = NULL;
							delete[] pszAppName;
							pszAppName = NULL;
						} // End if found product code for current user
						delete[] pszMangledKey;
						pszMangledKey = NULL;
					} // End if could enumerate next key
				} // Next subkey
				delete[] pszUserId;
				pszUserId = NULL;
			} // End if get number of sub keys and length of longest worked
			RegCloseKey(hKey);
			hKey = NULL;

		} // End if opened HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData
	} // Next Product Code

	if (hmodAdvApi)
		FreeLibrary(hmodAdvApi);

	return pszReport;
}

