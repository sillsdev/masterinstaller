#include <windows.h>
#include <stdio.h>

#include "ProductManager.h"
#include "ErrorHandler.h"
#include "Globals.h"
#include "ProductKeys.h"
#include "LogFile.h"
#include "WIWrapper.h"
#include "UsefulStuff.h"
#include "Resource.h"
#include "DiskManager.h"
#include "Dialogs.h"
#include "PersistantProgress.h"


#include "SoftwareProductTypes.h"

// Declaration of information about any software product on our CD:
class SoftwareProduct
{
public:
#include "SoftwareProductMembers.h"	

	IndexList_t m_rgiPrerequisites;
	IndexList_t m_rgiRequirements;

	enum v_Installation
	{
		InstallNotAttempted,
		InstallSucceeded,
		InstallFailed,
		InstallFailedInterrupted,
		InstallFailedUserAbandoned,
		InstallFailedNeededWin2KOrMore,
		InstallFailedNeededAdmin,
		InstallFailedFileNotFound,
	};
	v_Installation m_InstallStatus;
	bool m_fCriticalFileLanguageUnavailable;
	char * m_pszCriticalFile;

	bool CriticalFileLanguageUnavailable();
	const char * GetDownloadUrl() { return m_kpszDownloadUrl; }
	char * new_LanguageDecodedString(const char * pszTemplate);
	char * new_InterpretString(const char * pszTemplate);
	bool PossibleToTestPresence() const;
	bool TestPresence();
	bool TestPresence(const char * pszVersion);
	bool TestPresence(const char * pszMinVersion, const char * pszMaxVersion);
	const char * GetCriticalFile();
	bool IsInstallable();
	DWORD RunInstaller();
	bool Install();
};


// Parses the given string for a token representing the current Windows language identifier.
// All values in base 10. Use the format $LANG$value$ to match only primary language, and
// $LANG$SUB$value$ to match primary language and sublanguage.
// Examples: $LANG$SUB$1033$ for English (United States); $LANG$9$ for any English.
// If found, the part of the string following that token is returned as a new string.
// If no token is found, the string is assumed to be language neutral, and a copy of the
// whole string is returned.
// If there is at least one token present, but no language matches, NULL is returned.
// Caller must delete[] return value.
char * SoftwareProduct::new_LanguageDecodedString(const char * pszTemplate)
{
	if (!pszTemplate)
		return NULL;

	// Make a working copy that we can manipulate:
	char * pszTemplateCopy = my_strdup(pszTemplate);

	// Search for "$LANG$" string:
	const char * kpszToken = "$LANG$";
	char * pszNextToken = strstr(pszTemplateCopy, kpszToken);
	if (!pszNextToken)
	{
		// There are no language specifiers in the string:
		return pszTemplateCopy;
	}
	do
	{
		char * pszToken = pszNextToken;
		bool fSubLang = false;
		int nTokenLen = (int)strlen(kpszToken);
		const char * kpszSubToken = "$LANG$SUB$";
		// See if Sublanguage is included:
		if (strncmp(pszToken, kpszSubToken, strlen(kpszSubToken)) == 0)
		{
			fSubLang = true;
			nTokenLen = (int)strlen(kpszSubToken);
		}

		// Read number part of token:
		pszToken += nTokenLen;
		DWORD langid = atoi(pszToken);

		// Jump past end of token:
		pszToken = strchr(pszToken, '$');
		pszToken++;

		// Find next "$LANG$" token:
		pszNextToken = strstr(pszToken, kpszToken);

		// See if the language description matches current Windows language:
		bool fLanguageMatches = false;
		if (g_langidWindowsLanguage == 0)
			fLanguageMatches = true; // There is no Windows Language, as such.
		else if (fSubLang)
		{
			if (langid == g_langidWindowsLanguage)
				fLanguageMatches = true;
		}
		else
		{
			if (langid == (g_langidWindowsLanguage & 0xFF))
				fLanguageMatches = true;
		}
		if (fLanguageMatches)
		{
			// Make copy of relevant part of string:
			if (pszNextToken)
				*pszNextToken = 0;
			char * pszResult = my_strdup(pszToken);		
			delete[] pszTemplateCopy;
			return pszResult;
		}
	} while (pszNextToken);

	delete[] pszTemplateCopy;
	return NULL;
}

// Parses the given string, replacing special tokens with runtime data. Returns newly created
// string with interpretted data.
// Caller must delete[] return value.
char * SoftwareProduct::new_InterpretString(const char * pszTemplate)
{
	if (!pszTemplate)
		return NULL;

	char * pszResult = NULL;
	// Make a working copy that we can manipulate:
	char * pszTemplateCopy = my_strdup(pszTemplate);

	// Search for "<CriticalFile>" string:
	const char * kpszCritFileToken = "$CriticalFile$";
	char * pszCritFileToken = strstr(pszTemplateCopy, kpszCritFileToken);
	if (pszCritFileToken)
	{
		// Cut off string at the token:
		*pszCritFileToken = 0;
		// Got the token, so perform the substitution:
		char * pszIntermediate = new_sprintf("%s%s%s", pszTemplateCopy, GetCriticalFile(),
			(pszCritFileToken + strlen(kpszCritFileToken)));

		// Now recurse, to see if any more text needs interpretting:
		pszResult = new_InterpretString(pszIntermediate);
		delete[] pszIntermediate;
	}

	// Search for '%' character (environment variable):
	char * pszFirstPercent = strchr(pszTemplateCopy, '%');
	if (pszFirstPercent)
	{
		// We have a '%' character. See if there is a second:
		char * pszSecondPercent = strchr(pszFirstPercent + 1, '%');
		if (pszSecondPercent)
		{
			// There are two '%' characters, so we will assume we have to substitute text with
			// the value of the given environment variable:
			*pszFirstPercent = 0;
			*pszSecondPercent = 0;
			char * pszEnvVarName = pszFirstPercent + 1;
			const int cchEnvValue = 32767;
			char szEnvValue[cchEnvValue];
			if (!GetEnvironmentVariable(pszEnvVarName, szEnvValue, cchEnvValue))
			{
				char * pszMsg = new_sprintf("A request to evaluate Environment Variable \"%s\""
					" failed.", pszEnvVarName);
				HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, pszMsg);
				delete[] pszMsg;
				return pszTemplateCopy;
			}
			// Got the value, so perform the substitution:
			char * pszIntermediate = new_sprintf("%s%s%s", pszTemplateCopy, szEnvValue,
				(pszSecondPercent + 1));

			// Now recurse, to see if any more text needs interpretting:
			pszResult = new_InterpretString(pszIntermediate);
			delete[] pszIntermediate;
		}
	} // End search for environment variables

	// Search for '[' character (registry data):
	char * pszOpenBracket = strchr(pszTemplateCopy, '[');
	if (pszOpenBracket)
	{
		// We have a '[' character. See if there is a ']':
		char * pszCloseBracket = strchr(pszOpenBracket + 1, ']');
		if (pszCloseBracket)
		{
			// There are matching brackets, so we will assume we have to substitute text with
			// the value of the given registry data:
			*pszOpenBracket = 0;
			*pszCloseBracket = 0;
			char * pszRegKey = pszOpenBracket + 1;
			char * pszRegKeyCopy = my_strdup(pszRegKey);
			HKEY hKeyRoot = NULL;
			char * pszValue = strstr(pszRegKey, "::");
			if (pszValue)
			{
				*pszValue = 0;
				pszValue += 2;
			}
			char * pszRegSubKey = strchr(pszRegKey, '\\');
			if (pszRegSubKey)
			{
				*pszRegSubKey = 0;
				pszRegSubKey++;
			}
			if (strcmp(pszRegKey, "HKEY_CLASSES_ROOT") == 0 || strcmp(pszRegKey, "HKCR") == 0)
				hKeyRoot = HKEY_CLASSES_ROOT;
			else if (strcmp(pszRegKey, "HKEY_CURRENT_USER") == 0 || strcmp(pszRegKey, "HKCU") == 0)
				hKeyRoot = HKEY_CURRENT_USER;
			else if (strcmp(pszRegKey, "HKEY_LOCAL_MACHINE") == 0 || strcmp(pszRegKey, "HKLM") == 0)
				hKeyRoot = HKEY_LOCAL_MACHINE;
			else if (strcmp(pszRegKey, "HKEY_USERS") == 0 || strcmp(pszRegKey, "HKU") == 0)
				hKeyRoot = HKEY_USERS;
			else if (strcmp(pszRegKey, "HKEY_DYN_DATA") == 0 || strcmp(pszRegKey, "HKDD") == 0)
				hKeyRoot = HKEY_DYN_DATA;

			HKEY hKey;
			LONG lResult = RegOpenKeyEx(hKeyRoot, pszRegSubKey, 0, KEY_READ, &hKey);
			const int cchKeyData = 32767;
			char szKeyData[cchKeyData];
			if (ERROR_SUCCESS == lResult)
			{
				DWORD cbData = sizeof(szKeyData);
				
				lResult = RegQueryValueEx(hKey, pszValue, NULL, NULL, (LPBYTE)szKeyData,
					&cbData);
				RegCloseKey(hKey);
			}
			if (ERROR_SUCCESS != lResult)
			{
				char * pszMsg = new_sprintf("A request to evaluate Registry Data \"%s\""
					" failed.", pszRegKeyCopy);
				delete[] pszRegKeyCopy;
				pszRegKeyCopy = NULL;
				HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, pszMsg);
				delete[] pszMsg;
				return pszTemplateCopy;
			}
			delete[] pszRegKeyCopy;
			pszRegKeyCopy = NULL;

			// Got the value, so perform the substitution:
			char * pszIntermediate = new_sprintf("%s%s%s", pszTemplateCopy, szKeyData,
				(pszCloseBracket + 1));

			// Now recurse, to see if any more text needs interpretting:
			pszResult = new_InterpretString(pszIntermediate);
			delete[] pszIntermediate;
		}
	} // End search for registry data

	// If we didn't perform any substitution, return the working copy of the original:
	if (!pszResult)
		pszResult = pszTemplateCopy;
	else
	{
		delete[] pszTemplateCopy;
		pszTemplateCopy = NULL;
	}

	return pszResult;
}

// Determine if any mechanism exists to test for the presence of the given product.
bool SoftwareProduct::PossibleToTestPresence() const
{
	if (m_pfnAuxTestPresence || m_kpszAuxTestPresence || m_kpszMsiProductCode)
		return true;
	return false;
}

// Test for presence of a product whose version is set at compile time.
bool SoftwareProduct::TestPresence()
{
	return TestPresence(m_kpszTestPresenceVersion, m_kpszTestPresenceVersion);
}

// Test for presence of a specific version of a product.
bool SoftwareProduct::TestPresence(const char * pszVersion)
{
	return TestPresence(pszVersion, pszVersion);
}

// Test for presence of a product within a range of versions. Note that if it is not possible to
// test for a particluar product, this funtion returns false - as if it is not present.
bool SoftwareProduct::TestPresence(const char * pszMinVersion, const char * pszMaxVersion)
{
	bool fPresent = false; // Assume the worst, to start with.

	// If the software product has a custom function or program, use that:
	if (m_pfnAuxTestPresence || m_kpszAuxTestPresence)
	{
		if (m_pfnAuxTestPresence)
			fPresent = m_pfnAuxTestPresence(pszMinVersion, pszMaxVersion, GetCriticalFile());
		if (m_kpszAuxTestPresence)
			fPresent |= (0 != ExecCmd(m_kpszAuxTestPresence, true, true));
	}
	else if (m_kpszMsiProductCode)
	{
		// We should have an MSI product code, and optionally a feature name.
		// Use installer API (via wrapper class):
		INSTALLSTATE state = INSTALLSTATE_UNKNOWN;

		if (m_kpszMsiFeature)
		{
			state = WindowsInstaller.MsiQueryFeatureState(m_kpszMsiProductCode,
				m_kpszMsiFeature);
		}
		else
		{
			// No feature is specified, so check install state of product code:
			state = WindowsInstaller.MsiQueryProductState(m_kpszMsiProductCode);
		}
		switch (state)
		{
		case INSTALLSTATE_ADVERTISED:
		case INSTALLSTATE_LOCAL:
		case INSTALLSTATE_SOURCE:
		case INSTALLSTATE_DEFAULT:
			{
				fPresent = true;

				g_Log.Write(".msi installation is present.");

				// Product is installed, but check if an upgrade is required:
				if (m_kpszMsiVersion)
				{
					const int kcchVersion = 256;
					char szIntalledVersion[kcchVersion];
					DWORD cch = kcchVersion;
					if (WindowsInstaller.MsiGetProductInfo(m_kpszMsiProductCode,
						INSTALLPROPERTY_VERSIONSTRING, szIntalledVersion, &cch)
						== ERROR_SUCCESS)
					{
						if (!VersionInRange(szIntalledVersion, pszMinVersion,
							pszMaxVersion))
						{
							fPresent = false;
							g_Log.Write("Version %s is installed, but version %s is available.",
								szIntalledVersion, m_kpszMsiVersion);
						}
					}
				} // End if the is an MsiVersion specified
			} // End case scope
		} // End switch
	} // End if product code exists

	if (!pszMinVersion)
		pszMinVersion = "any";
	if (!pszMaxVersion)
		pszMaxVersion = "any";
	g_Log.Write("%s (version %s through %s): %s", m_kpszNiceName, pszMinVersion, pszMaxVersion,
		(fPresent ? "present" : "absent"));

	return fPresent;
}

// Determines if there is a critical file that will match any Windows language criteria in its
// definition.
bool SoftwareProduct::CriticalFileLanguageUnavailable()
{ 
	GetCriticalFile(); // Analyze the language situation, but no need to keep the file path.
	return m_fCriticalFileLanguageUnavailable;
}

// Returns the critical file of the product. Without it, we deduce that the product is not
// on the current CD.
const char * SoftwareProduct::GetCriticalFile()
{
	if (m_pszCriticalFile)
		return m_pszCriticalFile;

	m_fCriticalFileLanguageUnavailable = false;

	// See if a condition was specified:
	if (m_fCriticalFileFlag)
	{
		// The file depends on a condition, so evaluate that condition:
		if (*m_fCriticalFileFlag)
			m_pszCriticalFile = new_LanguageDecodedString(m_pszCriticalFileFlagTrue);
		else
			m_pszCriticalFile = new_LanguageDecodedString(m_pszCriticalFileFlagFalse);
	}
	else
		m_pszCriticalFile = new_LanguageDecodedString(m_pszCriticalFileFlagTrue);

	if (!m_pszCriticalFile)
	{
		m_fCriticalFileLanguageUnavailable = true;
		return "";
	}

	return m_pszCriticalFile;
}

// Returns true if the product has an install method.
// (Products which don't can only have their '?' button pressed.)
bool SoftwareProduct::IsInstallable()
{
	if (!m_pfnInstall && !m_kpszMsiProductCode && !m_kpszInstallerFlagTrue)
		return false;
	return true;
}

// Runs a product's installer.
DWORD SoftwareProduct::RunInstaller()
{
	DWORD nResult = 0;

	// See if we're to call an internal function, or launch an external program:
	if (m_pfnInstall)
	{
		// We have to call an internal function:
		if (m_pfInstallerFlag)
			nResult = m_pfnInstall(*m_pfInstallerFlag, GetCriticalFile());
		else
			nResult = m_pfnInstall(true, GetCriticalFile());
	}
	else
	{
		// External program:
		const char * pszCmd = m_kpszInstallerFlagTrue;

		// See if we're to use the standard Windows Installer mechanism:
		if (!m_kpszInstallerFlagTrue && m_kpszMsiProductCode)
		{
			// We'll be using Windows Installer.
			g_Log.Write("Windows installer will be used.");
			const char * kpszInstallFlags = "/i";

			// See if we need to test for a minor upgrade:
			if (m_kpszMsiVersion)
			{
				g_Log.Write("MsiVersion %s specified.", m_kpszMsiVersion);

				// Get version number of installed instance:
				const int kcchVersion = 256;
				char szIntalledVersion[kcchVersion];
				DWORD cch = kcchVersion;
				if (WindowsInstaller.MsiGetProductInfo(m_kpszMsiProductCode,
					INSTALLPROPERTY_VERSIONSTRING, szIntalledVersion, &cch)	== ERROR_SUCCESS)
				{
					g_Log.Write("Version %s already installed.", szIntalledVersion);
					// Another version is installed, so check if its version is less than ours:
					__int64 nIntalledVersion = GetHugeVersion(szIntalledVersion);
					__int64 nOurVersion = GetHugeVersion(m_kpszMsiVersion);
					if (nOurVersion > nIntalledVersion)
					{
						g_Log.Write("Minor upgrade will be performed.");
						kpszInstallFlags = "/fvomus";
						if (m_kpszMsiUpgrade)
							kpszInstallFlags = m_kpszMsiUpgrade;
					}
				}
			}
			// Run msiexec:
			char * pszMsiExec = new_sprintf("MsiExec.exe %s \"%s\" %s", kpszInstallFlags,
				GetCriticalFile(), m_kpszMsiFlags? m_kpszMsiFlags : "");

			g_Log.Write("About to run \"%s\"", pszMsiExec);

			nResult = ExecCmd(pszMsiExec, true, true, m_kpszNiceName,
				m_kpszStatusWindowControl);
		}
		else // Not using Windows Installer.
		{
			// See if a condition was specified:
			if (m_pfInstallerFlag)
			{
				// The command depends on a condition, so evaluate that condition:
				if (!(*m_pfInstallerFlag))
					pszCmd = m_kpszInstallerFlagFalse;
			}
			if (pszCmd)
			{
				// Parse the given string, replacing special tokens with runtime data:
				char * pszModCmd = new_InterpretString(pszCmd);
				nResult = ExecCmd(pszModCmd, true, true, m_kpszNiceName,
					m_kpszStatusWindowControl);
				delete[] pszModCmd;
				pszModCmd = NULL;

				// See if we need to get the user to press continue after the installer
				// has run (because we cannot detect it ourselves):
				if (m_fStatusPauseWin98 && g_fLessThanWin2k)
				{
					PauseOnStatusDialog(IDC_MESSAGE_MUST_WAIT);
					// Fiddle the installer's exit code:
					nResult = ERROR_SUCCESS;
				}
			}
			else
				g_Log.Write("Installer command string is empty!");
		}
	}
	return nResult;
}

// Install a product.
// Assumes that all prerequisite products have been installed.
bool SoftwareProduct::Install()
{
	// If the product was expressly omitted from the CD set then don't bother with the tests
	// for admin rights, correct OS, etc:
	if (m_iCd >= 0)
	{
		// Check if we have already tried unsuccessfully to install this product:
		if (m_InstallStatus >= InstallFailed)
			return false;

		// Check if the circumstances are OK for installing this product:
		if (m_fMustHaveWin2kOrBetter && g_fLessThanWin2k)
		{
			HandleError(kNonFatal, false, IDC_ERROR_NEED_WIN2K_OR_BETTER, m_kpszNiceName);
			m_InstallStatus = InstallFailedNeededWin2KOrMore;
			return false;
		}
		if (m_fMustBeAdmin && !g_fAdministrator)
		{
			HandleError(kNonFatal, false, IDC_ERROR_NEED_ADMIN_ACCESS, m_kpszNiceName);
			m_InstallStatus = InstallFailedNeededAdmin;
			return false;
		}
		if (m_fMustKillHangingWindows)
		{
			bool fTestHanging = true;
			while (fTestHanging)
			{
				g_Log.Write("Testing for hanging windows (for %s)", m_kpszNiceName);
				ShowStatusDialog();
				const char * pszMsg = DisplayStatusText(0,
					FetchString(IDC_MESSAGE_TEST_HANGING));
				char * pszHangingReport = GenerateHangingWindowsReport();
				if (pszHangingReport != NULL)
				{
					HideStatusDialog();

					char * pszIntro1 = new_sprintf(
						FetchString(IDC_MESSAGE_HANGING_WINDOWS_INTRO_1), m_kpszNiceName);
					char * pszIntro2 = new_sprintf(
						FetchString(IDC_MESSAGE_HANGING_WINDOWS_INTRO_2), m_kpszNiceName);
					char * pszAlert = new_sprintf("%s\n%s\n\n%s", pszIntro1, pszHangingReport,
						pszIntro2);

					delete[] pszIntro1;
					pszIntro1 = NULL;
					delete[] pszHangingReport;
					pszHangingReport = NULL;
					delete[] pszIntro2;
					pszIntro2 = NULL;

					g_Log.Write(pszAlert);

					int idResult;
					do
					{
						idResult = MessageBox(NULL, pszAlert, g_pszTitle,
							MB_ICONSTOP | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);

						if (idResult == IDABORT)
						{
							// Confirm quit:
							if (MessageBox(NULL, FetchString(IDC_MESSAGE_CONFIRM_QUIT_GENERAL),
								g_pszTitle, MB_YESNO) == IDYES)
							{
								g_Log.Write("User opted to quit.");
								delete[] pszAlert;
								pszAlert = NULL;
								throw UserQuitException;
							}
						}
					} while (idResult == IDABORT);

					delete[] pszAlert;
					pszAlert = NULL;

					switch (idResult)
					{
					case IDRETRY:
						g_Log.Write("User pressed Retry.");
						break;
					case IDIGNORE:
						g_Log.Write("User pressed Ignore.");
						fTestHanging = false;
						break;
					}
				} // End if hanging windows report contained anything.
				else
				{
					fTestHanging = false;
					g_Log.Write("No hanging windows.");
				}
			} // End while(fTestHanging)
		}// End if (m_fMustKillHangingWindows)
	} // End if product included in CD set.

	bool fCalledPreInstallFunction = false;

	bool fRepeat;
	do // Repeat until installation works, or user quits/skips.
	{
		fRepeat = false;

		// Make sure correct disk is inserted:
		int nFileResult = DiskManager_t::knFileOmitted;
		if (m_iCd >= 0)
		{
			nFileResult = g_DiskManager.EnsureCdForFile(GetCriticalFile(), m_iCd, 
				m_kpszNiceName);
			ShowStatusDialog();
		}

		if (nFileResult == DiskManager_t::knCorrectCdAlready ||
			nFileResult == DiskManager_t::knCorrectCdFinally ||
			nFileResult == DiskManager_t::knFileFoundWrongCd)
		{
			// Anticipate not finishing properly:
			m_InstallStatus = InstallFailedInterrupted;

			// Enable us to restart if a reboot is executed:
			g_ProgRecord.WriteRunOnce();

			if (!fCalledPreInstallFunction &&
				(m_pfnPreInstallation || m_kpszPreInstallation))
			{
				int nReturn = 0;
				if (m_pfnPreInstallation)
				{
					g_Log.Write("Calling pre-installation function...");
					nReturn = m_pfnPreInstallation(GetCriticalFile());
					g_Log.Write("...Done. Pre-installation function returned %d", nReturn);
				}
				if (m_kpszPreInstallation)
				{
					g_Log.Write("Executing pre-installation program '%s'...", m_kpszPreInstallation);
					nReturn = ExecCmd(m_kpszPreInstallation, true, true);
					g_Log.Write("...Done. Pre-installation program returned %d", nReturn);
				}
				if (m_fIngnorePreInstallationErrors)
				{
					nReturn = 0;
					g_Log.Write("Ignoring returned error code.", nReturn);
				}
				fCalledPreInstallFunction = true;
				if (nReturn != 0)
				{
					g_Log.Write("Skipping product installation");
					return false;
				}
			}

			// Display status message, and write progress to log file:
			const char * pszMsg = DisplayStatusText(0, FetchString(IDC_MESSAGE_INSTALLING),
				m_kpszNiceName);
			g_Log.Write(pszMsg);
			DisplayStatusText(1, m_kpszCommentary);

			g_Log.Write("Launching installer for %s: %s.", m_kpszNiceName,
				(nFileResult == DiskManager_t::knCorrectCdAlready ? "correct CD already" :
				(nFileResult == DiskManager_t::knCorrectCdFinally ? "correct CD finally" :
				"file found on wrong cd")));

			// Run installer:
			DWORD dwResult = RunInstaller();

			CheckIfStopRequested();

			// Check to see if the installation worked:
			DisplayStatusText(0, FetchString(IDC_MESSAGE_EVALUATING));
			DisplayStatusText(1, "");

			bool fOfferAbortRetryOrIgnore = false;
			m_InstallStatus = InstallSucceeded;
			char * pszRetry = NULL;

			if (dwResult == ERROR_SUCCESS_REBOOT_REQUIRED
				|| dwResult == ERROR_SUCCESS_RESTART_REQUIRED)
			{
				// Product installed and requested a reboot:
				g_Log.Write("%s installed and requested a reboot.", m_kpszNiceName);
				g_fRebootPending = true;

				// If the MustNotDelayReboot flag is set, do the reboot now:
				if (m_fMustNotDelayReboot)
				{
					g_Log.Write("Reboot must not be delayed.");
					FriendlyReboot();
				}
			}
			else if (dwResult != ERROR_SUCCESS && dwResult != ERROR_INSTALL_USEREXIT &&
				dwResult != ERROR_CANCELLED)
			{
				// Some error occurred:
				HandleError(kNonFatal, false, IDC_ERROR_INSTALLATION, m_kpszNiceName);
				m_InstallStatus = InstallFailed;
				return false;
			}
			else if (dwResult == ERROR_INSTALL_USEREXIT || dwResult == ERROR_CANCELLED)
			{
				// User quit installer:
				m_InstallStatus = InstallFailedUserAbandoned;
				g_Log.Write("User quit installing %s.", m_kpszNiceName);
				pszRetry = new_sprintf(FetchString(IDC_ERROR_USER_QUIT_INSTALLER),
					m_kpszNiceName);
				fOfferAbortRetryOrIgnore = true;
			}			
			else 
			{
				// See if the product is acutally there:
				if (PossibleToTestPresence() && !TestPresence())
				{
					m_InstallStatus = InstallFailed;
					g_Log.Write("%s is still not there!", m_kpszNiceName);
					pszRetry = new_sprintf(FetchString(IDC_ERROR_INSTALLATION), m_kpszNiceName);
					fOfferAbortRetryOrIgnore = true;
				}
			}
			if (fOfferAbortRetryOrIgnore)
			{
				new_sprintf_concat(pszRetry, 1, FetchString(IDC_MESSAGE_RETRY_INSTRUCTIONS));

				HideStatusDialog();

				int nResult = MessageBox(NULL, pszRetry, g_pszTitle,
					MB_ICONSTOP | MB_ABORTRETRYIGNORE | MB_DEFBUTTON3);
				delete[] pszRetry;
				pszRetry = NULL;

				switch(nResult)
				{
				case IDABORT:
					g_Log.Write("User opted to abort.");
					throw UserQuitException;
					break;
				case IDRETRY:
					fRepeat = true;
					g_Log.Write("User opted to retry.");
					break;
				default: // IDIGNORE
					g_Log.Write("User opted to ignore.");
					return false;
				}
			}
		} // End if file found
		else if (nFileResult == DiskManager_t::knUserQuit)
		{
			g_Log.Write("User quit during checking disk for %s.", m_kpszNiceName);
			throw UserQuitException;
		}
		else if (nFileResult == DiskManager_t::knUserSkip)
		{
			g_Log.Write("User chose to skip %s.", m_kpszNiceName);
			m_InstallStatus = InstallFailedUserAbandoned;
			return false;
		}
		else if (nFileResult == DiskManager_t::knFileNotFoundCorrectCd 
			|| nFileResult == DiskManager_t::knFileOmitted)
		{
			// The software is not on the CD, so assume it is a "Lite" CD, where
			// user has to do some work himself:
			const char * pszUrl = GetDownloadUrl();
			if (pszUrl)
				HandleError(kNonFatal, false, IDC_ERROR_FILE_MISSING_URL, m_kpszNiceName, pszUrl);
			else
				HandleError(kNonFatal, false, IDC_ERROR_FILE_MISSING, m_kpszNiceName);
			m_InstallStatus = InstallFailedFileNotFound;
			return false;
		}
	} while (fRepeat);

	if (m_pfnPostInstallation)
	{
		g_Log.Write("Calling post-installation function...");
		int nReturn = m_pfnPostInstallation(GetCriticalFile());
		g_Log.Write("...Done. Post-installation function returned %d", nReturn);
	}
	if (m_kpszPostInstallation)
	{
		g_Log.Write("Executing post-installation program '%s'...", m_kpszPostInstallation);
		int nReturn = ExecCmd(m_kpszPostInstallation, true, true);
		g_Log.Write("...Done. Post-installation program returned %d", nReturn);
	}

	return true;
}


static bool s_fSoftwareProductsInitialized = false;

// Declaration of information about any software dependency:
struct Mapping
{
#include "MappingMembers.h"
};

// Include file auto-generated from XML specification:
#include "ConfigProducts.cpp"

// Work out Product array size (known at compile time):
static const int kctSoftwareProducts = sizeof(Products) / sizeof(Products[0]);
// Build a vector of dependency mapping lists:
static const Mapping * const s_DependecyMaps[] =
{
	Prerequisites,
	RunDependencies,
};
static const int s_kMappings[] =
{
	kctPrerequisites,
	kctRunDependencies,
};

// Wrapper class to manage the const array of software products. Accessed via interface
// IProductManager.
class ProductManager_t : public IProductManager
{
public:
	~ProductManager_t();
	bool Init();

	virtual bool GetFlushRebootFlag(int iProduct) const;
	virtual bool GetRebootTestRegPendingFlag(int iProduct) const;
	virtual bool GetRebootWininitFlag(int iProduct) const;
	virtual bool PossibleToTestPresence(int iProduct) const;
	virtual bool TestPresence(int iProduct, const char * pszMinVersion = NULL,
		const char * pszMaxVersion = NULL);
	virtual const char * GetName(int iProduct) const;
	virtual const char * GetCommentary(int iProduct) const;
	virtual const char * GetStatusWindowControl(int iProduct) const;
	virtual const char * GetCriticalFile(int iProduct) const;
	virtual bool CriticalFileLanguageUnavailable(int iProduct) const;
	virtual const char * GetDownloadUrl(int iProduct) const;
	virtual int GetCdIndex(int iProduct) const;
	virtual const char * GetHelpTag(int iProduct) const;
	virtual bool GetHelpTagInternalFlag(int iProduct) const;
	virtual const char * GetTestPresenceVersion(int iProduct) const;
	virtual bool GetMustHaveWin2kOrBetterFlag(int iProduct) const;
	virtual bool GetMustBeAdminFlag(int iProduct) const;
	virtual int GetNumProtectedMainProducts() const;
	virtual void DetermineAvailableMainProducts(ProductKeyHandler_t & ProductKeyHandler,
		const char * pszKey);
	virtual void GenAvailableMainProductList(IndexList_t & rgiProducts,
		bool fIncludeVisibles) const;
	virtual int GetNumPermittedMainProducts() const;
	virtual bool KeyUnlockedNothing() const;
	virtual void AutoSelectAllPermittedMainProducts(IndexList_t & rgiProducts) const;
	virtual bool PrerequisitesNeeded(const IndexList_t & rgiSelectedProducts) const;
	virtual bool RequirementsNeeded() const;
	virtual void GetActivePrerequisites(const IndexList_t & rgiProducts,
		IndexList_t & rgiOutputList, bool fRecurse) const;
	virtual void GetActiveRequirements(IndexList_t & rgiOutputList) const;
	void GetActiveRequirements(const IndexList_t & rgiProducts, IndexList_t & rgiOutputList,
		bool fRecursePrerequisites, bool fRecurseRequirements) const;
	virtual bool PriorInstallationFailed(int iProduct) const;
	virtual bool IsInstallable(int iProduct) const;
	virtual bool InstallProduct(int iProduct);
	virtual char * GenReport(int iReportType, IndexList_t * prgiProducts = NULL) const;
	virtual bool GetDependencyMinMaxVersions(int iDependType, int iProduct1, int iProduct2,
		const char *& pszMinVersion, const char *& pszMaxVersion) const;
	virtual void ShowReport(const char * pszTitle, const char * pszIntro,
		const char * pszOkButtonText, bool fConfirmQuit, bool fQuitIsError, int nType,
		bool fCanToggleType, IndexList_t * rgiProducts = NULL) const;
	virtual bool ShowFinalReport() const;

protected:
	void EstablishOverallDependencies();
	void CheckProductIndex(int iProduct) const;
	void CheckDependencyType(int iDependType) const;
	int GetSoftwareProductIndex(const char * pszProduct) const;

	bool m_fPermittedAfterKeyTest[kctSoftwareProducts];
	bool m_fOnlyVisibleAfterKeyTest[kctSoftwareProducts];
	int m_ctUnlockedProtectedProducts;
	int m_ctPermittedAfterKeyTest;
	int GetMappingIndex(int iDependType, int iProduct1, int iProduct2) const;
	char * GenFullPrerequisiteReport(const IndexList_t & rgiProducts, int nIndent = 0) const;
	char * GenFullRequirementReport(const IndexList_t & rgiProducts, int nIndent = 0) const;
	char * GenInstallFailureReport(int iProduct, int nIndent = 0) const;
	char * GenPrerequisiteFailureReport(int iProduct, int nIndent = 0) const;
	char * GenInstallFailureReport(IndexList_t & rgiCandidateProducts, int nIndent = 0) const;
	char * GenMissingRequirementConsequencesReport(int iProduct, int nIndent = 0) const;
	char * GenMissingRequirementReport(int iProduct, int nIndent = 0) const;
	char * GenMissingRequirementReport(int nIndent = 0) const;
};

// Creates an instance of a class that implements the IProductManager interface.
IProductManager * CreateProductManager()
{
	ProductManager_t * ppm = new ProductManager_t;
	if (!ppm->Init())
	{
		delete ppm;
		ppm = NULL;
	}
	return ppm;
}

void DestroyProductManager(IProductManager *& ppmProductManager)
{
	if (ppmProductManager)
	{
		ProductManager_t * ppm = reinterpret_cast <ProductManager_t *> (ppmProductManager);
		delete ppm;
		ppmProductManager = NULL;
	}
}

// Set all flags etc to false.
bool ProductManager_t::Init()
{
	// Mark as "permitted" all main products that aren't locked:
	m_ctPermittedAfterKeyTest = 0;
	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		if (Products[i].m_kfOneOfOurs && Products[i].m_nKeyId < 0)
		{
			m_fPermittedAfterKeyTest[i] = true;
			m_ctPermittedAfterKeyTest++;
		}
		else
			m_fPermittedAfterKeyTest[i] = false;
		m_fOnlyVisibleAfterKeyTest[i] = false;

		if (!s_fSoftwareProductsInitialized)
		{
			Products[i].m_InstallStatus = 
				(SoftwareProduct::v_Installation)g_ProgRecord.ReadInstallStatus(i);
		}

		Products[i].m_pszCriticalFile = NULL;
	}
	s_fSoftwareProductsInitialized = true;
	m_ctUnlockedProtectedProducts = 0;

	EstablishOverallDependencies();

	return true;
}

ProductManager_t::~ProductManager_t()
{
	for (int i = 0; i < kctSoftwareProducts; i++)
		delete[] Products[i].m_pszCriticalFile;
}

// Internal check to make sure the given Product index is valid.
void ProductManager_t::CheckProductIndex(int iProduct) const
{
	if (iProduct < 0 || iProduct >= kctSoftwareProducts)
	{
		HandleError(kFatal, false, IDC_ERROR_INTERNAL, "invalid product index");
		// Doesn't return
	}
}

// Internal check to make sure the given dependency type is valid.
void ProductManager_t::CheckDependencyType(int iDependType) const
{
	if (iDependType < 0 || iDependType >= depTotal)
	{
		HandleError(kFatal, false, IDC_ERROR_INTERNAL, "invalid dependency type");
		// Doesn't return
	}
}

// Returns true if the given product needs pending reboots to be activated before it can be
// installed.
bool ProductManager_t::GetFlushRebootFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fFlushPendingReboot;
}

// Returns true if the given product needs the PendingFileRenameOperations registry setting
// to be used in detecting a pending reboot.
bool ProductManager_t::GetRebootTestRegPendingFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fRebootTestRegPending;
}

// Returns true if the given product needs to detect the presence of file wininit.ini
// when testing for a pending reboot.
bool ProductManager_t::GetRebootWininitFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fRebootWininit;
}

// Populate dependency lists in each product. This is independent of which products are
// selected by the user, and which products are already installed. Those factors are taken into
// account later.
void ProductManager_t::EstablishOverallDependencies()
{
	int iMapping;
	int iDependType;
	for (iDependType = 0; iDependType < depTotal; iDependType++)
	{
		for (iMapping = 0; iMapping < s_kMappings[iDependType]; iMapping++)
		{
			const char * kpszKeyProduct =
				s_DependecyMaps[iDependType][iMapping].m_kpszKeyProduct;
			const char * kpszDependentProduct =
				s_DependecyMaps[iDependType][iMapping].m_kpszDependentProduct;

			// Find indexes of key and dependent products:
			int iKey = GetSoftwareProductIndex(kpszKeyProduct);
			int iDependent = GetSoftwareProductIndex(kpszDependentProduct);

			if (iKey != -1 && iDependent != -1)
			{
				if (iDependType == depPrerequisite)
					Products[iKey].m_rgiPrerequisites.Add(iDependent);
				if (iDependType == depRequirement)
					Products[iKey].m_rgiRequirements.Add(iDependent);
			}
		} // Next mapping
	} // Next type
}

// Returns the index into the mapping array for the given combination of products.
int ProductManager_t::GetMappingIndex(int iDependType, int iProduct1, int iProduct2) const
{
	CheckProductIndex(iProduct1);
	CheckProductIndex(iProduct2);
	CheckDependencyType(iDependType);

	const char * pszProduct1 = Products[iProduct1].m_kpszInternalName;
	const char * pszProduct2 = Products[iProduct2].m_kpszInternalName;

	for (int iMapping = 0; iMapping < s_kMappings[iDependType]; iMapping++)
	{
		if (strcmp(s_DependecyMaps[iDependType][iMapping].m_kpszKeyProduct, pszProduct1) == 0 &&
			strcmp(s_DependecyMaps[iDependType][iMapping].m_kpszDependentProduct, pszProduct2)
			== 0)
		{
			return iMapping;
		}
	}
	return -1;
}

// Assigns the minimum and maximum version numbers of a required/prerequisite product for
// the given dependency relationship.
// Returns true if the versions could be determined.
bool ProductManager_t::GetDependencyMinMaxVersions(int iDependType, int iProduct1,
												   int iProduct2, const char *& pszMinVersion,
												   const char *& pszMaxVersion) const
{
	int iMapping = GetMappingIndex(iDependType, iProduct1, iProduct2);
	if (iMapping == -1)
		return false;

	pszMinVersion = s_DependecyMaps[iDependType][iMapping].m_kpszMinVersion;
	pszMaxVersion = s_DependecyMaps[iDependType][iMapping].m_kpszMaxVersion;

	return true;
}


bool ProductManager_t::PossibleToTestPresence(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].PossibleToTestPresence();
}

// Returns true if the given product is installed.
bool ProductManager_t::TestPresence(int iProduct, const char * pszMinVersion,
									const char * pszMaxVersion)
{
	CheckProductIndex(iProduct);
	return Products[iProduct].TestPresence(pszMinVersion, pszMaxVersion);
}

// Get the formal name of the given product.
const char * ProductManager_t::GetName(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszNiceName;
}

// Return the commentary text for installing the given product.
const char * ProductManager_t::GetCommentary(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszCommentary;
}

// Return the commentary text for installing the given product.
const char * ProductManager_t::GetStatusWindowControl(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszStatusWindowControl;
}

// Returns the critical file of the given product. Without it, we deduce that the product is not
// on the current CD.
const char * ProductManager_t::GetCriticalFile(int iProduct) const
{
	CheckProductIndex(iProduct);

	return Products[iProduct].GetCriticalFile();
}

bool ProductManager_t::CriticalFileLanguageUnavailable(int iProduct) const
{
	CheckProductIndex(iProduct);

	return Products[iProduct].CriticalFileLanguageUnavailable();
}

const char * ProductManager_t::GetDownloadUrl(int iProduct) const
{
	CheckProductIndex(iProduct);

	return Products[iProduct].GetDownloadUrl();
}

// Returns the CD index where the given product is located.
int ProductManager_t::GetCdIndex(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_iCd;
}

const char * ProductManager_t::GetHelpTag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszHelpTag;
}

bool ProductManager_t::GetHelpTagInternalFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fInternalHelpFile;
}

const char * ProductManager_t::GetTestPresenceVersion(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszTestPresenceVersion;
}

bool ProductManager_t::GetMustHaveWin2kOrBetterFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fMustHaveWin2kOrBetter;
}

bool ProductManager_t::GetMustBeAdminFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fMustBeAdmin;
}

// Returns the number of locked main products (needing a product key).
int ProductManager_t::GetNumProtectedMainProducts() const
{
	int cProtected = 0;

	for (int i = 0; i < kctSoftwareProducts; i++)
		if (Products[i].m_kfOneOfOurs && Products[i].m_nKeyId >= 0)
			cProtected++;

	return cProtected;
}

// Builds an internal list of products available for selection by user. Products are available
// if either they are not locked, or the given product key unlocks them.
void ProductManager_t::DetermineAvailableMainProducts(ProductKeyHandler_t & ProductKeyHandler,
													  const char * pszKey)
{
	m_ctUnlockedProtectedProducts = 0;
	m_ctPermittedAfterKeyTest = 0;

	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		m_fPermittedAfterKeyTest[i] = false; // Assume the worst, initially.
		if (Products[i].m_kfOneOfOurs)
		{
			if (Products[i].m_nKeyId >= 0)
			{
				DWORD dwUnlocked;
				if (ProductKeyHandler.TestKey(pszKey, &dwUnlocked, Products[i].m_nKeyId,
					Products[i].m_kpszVersionInKey) == 0)
				{
					if (dwUnlocked == 2)
					{
						// Product is unlocked:
						m_fPermittedAfterKeyTest[i] = true;
						m_ctUnlockedProtectedProducts++;
						m_ctPermittedAfterKeyTest++;
					}
					else if (dwUnlocked == 1)
					{
						// Product is only visible:
						m_fOnlyVisibleAfterKeyTest[i] = true;
					}
				}
			}
			else // No key required
			{
				m_fPermittedAfterKeyTest[i] = true;
				m_ctPermittedAfterKeyTest++;
			}
		} // End if current product is one of ours
		else if (g_fManualInstall && !Products[i].m_kpszMsiFeature)
		{
			// Manual mode, so add product anyway:
			m_fPermittedAfterKeyTest[i] = true;
			m_ctPermittedAfterKeyTest++;
		}
	} // Next product
}

// Returns dynamically created list of available main products (i.e. unlocked by key or not
// requiring a key. If fIncludeVisibles is true, products that are only visible but not
// installable are included, with offset knVisibleOnlyOffset added to their index.
void ProductManager_t::GenAvailableMainProductList(IndexList_t & rgiProducts,
												   bool fIncludeVisibles) const
{
	rgiProducts.Flush();

	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		if (m_fPermittedAfterKeyTest[i])
			rgiProducts.Add(i);
		else if (fIncludeVisibles && m_fOnlyVisibleAfterKeyTest[i])
			rgiProducts.Add(i + knVisibleOnlyOffset);
		else if (g_fManualInstall && !Products[i].m_kfOneOfOurs &&
			!Products[i].m_kpszMsiFeature)
		{
			rgiProducts.Add(i);
		}
	}
}

int ProductManager_t::GetNumPermittedMainProducts() const
{
	return m_ctPermittedAfterKeyTest;
}

bool ProductManager_t::KeyUnlockedNothing() const
{
	return (m_ctUnlockedProtectedProducts == 0);
}

// Select all permitted (after product key test) main products.
void ProductManager_t::AutoSelectAllPermittedMainProducts(IndexList_t & rgiProducts) const
{
	rgiProducts.Flush();
	for (int i = 0; i < kctSoftwareProducts; i++)
		if (m_fPermittedAfterKeyTest[i])
			rgiProducts.Add(i);
}

// Determine if any of the prerequisites of the specified products are still needing to be
// installed.
bool ProductManager_t::PrerequisitesNeeded(const IndexList_t & rgiProducts) const
{
	// Create a list of needed prerequisites:
	IndexList_t rgiNeeded;
	GetActivePrerequisites(rgiProducts, rgiNeeded, true);

	// See if any are needed:
	return (rgiNeeded.GetCount() > 0);
}

// Determine if any required products are still needing to be installed.
bool ProductManager_t::RequirementsNeeded() const
{
	g_Log.Write("Generating list of active requirements...");
	g_Log.Indent();

	// Create a list of needed prerequisites:
	IndexList_t rgiNeeded;
	GetActiveRequirements(rgiNeeded);

	g_Log.Unindent();
	g_Log.Write("...Done. Number of active requirements = %d", rgiNeeded.GetCount());

	// See if any are needed:
	return (rgiNeeded.GetCount() > 0);
}

// Add into rgiOutputList the indexes of products that are non-installed prerequisites of the
// products indexed in prgiProducts.
// If fRecurse is true, the non-installed prerequisites of those required products are included,
// recursively.
void ProductManager_t::GetActivePrerequisites(const IndexList_t & prgiProducts,
											  IndexList_t & rgiOutputList, bool fRecurse) const
{
	// Iterate through every product in the given list:
	for (int i = 0; i < prgiProducts.GetCount(); i++)
	{
		// Get the current product's list of prerequisites:
		int iProduct = prgiProducts[i];
		IndexList_t & rgiPrereqs = Products[iProduct].m_rgiPrerequisites;
		IndexList_t rgiActivePrereqs;

		// Iterate through each prerequisite product:
		for (int i2 = 0; i2 < rgiPrereqs.GetCount(); i2++)
		{
			// If the output list already contains this prerequisite, then go on to the next:
			int iPreReq = rgiPrereqs[i2];
			if (rgiOutputList.Contains(iPreReq))
				continue;

			// See if the required version of the prerequisite is present already:
			const char * pszMinVersion;
			const char * pszMaxVersion;
			if (GetDependencyMinMaxVersions(depPrerequisite, iProduct, iPreReq, pszMinVersion,
				pszMaxVersion))
			{
				if (!Products[iPreReq].TestPresence(pszMinVersion, pszMaxVersion))
				{
					rgiOutputList.Add(iPreReq);
					rgiActivePrereqs.Add(iPreReq);
				}
			}
		}
		if (fRecurse)
		{
			// Recursively add the prerequisites of the current active prerequisites:
			GetActivePrerequisites(rgiActivePrereqs, rgiOutputList, true);
		}
	}
}

// Add into rgiOutputList the indexes of products that are non-installed requirements of the
// products indexed in prgiProducts.
// If fRecursePrerequisites is true, the non-installed prerequisites of those required
// products are included, recursively.
// If fRecurseRequirements is true, the non-installed requirments of those required
// products are included, recursively.
void ProductManager_t::GetActiveRequirements(const IndexList_t & rgiProducts,
											 IndexList_t & rgiOutputList,
											 bool fRecursePrerequisites,
											 bool fRecurseRequirements) const
{
	// Iterate through every product in the given list:
	for (int i = 0; i < rgiProducts.GetCount(); i++)
	{
		// Get the current product's list of requirements:
		int iProduct = rgiProducts[i];
		IndexList_t & rgiRequirements = Products[iProduct].m_rgiRequirements;

		// Iterate through each required product:
		for (int i2 = 0; i2 < rgiRequirements.GetCount(); i2++)
		{
			// If the output list already contains this requirement, then go on to the next:
			int iReq = rgiRequirements[i2];
			if (rgiOutputList.Contains(iReq))
				continue;

			// See if the required version of the required product is present already:
			const char * pszMinVersion;
			const char * pszMaxVersion;
			if (GetDependencyMinMaxVersions(depRequirement, iProduct, iReq, pszMinVersion,
				pszMaxVersion))
			{
				if (!Products[iReq].TestPresence(pszMinVersion, pszMaxVersion))
				{
					rgiOutputList.Add(iReq);
				}
			}
		}
		if (fRecursePrerequisites)
		{
			// Recursively add the prerequisites of the current required products:
			GetActivePrerequisites(rgiRequirements, rgiOutputList, true);
		}
		if (fRecurseRequirements)
		{
			// Recursively add the requirements of the current required products:
			GetActiveRequirements(rgiRequirements, rgiOutputList, fRecursePrerequisites,
				true);
		}
	}
}

// Add into rgiOutputList the indexes of all products that are required (at any nested level)
// by any installed product. Includes any prerequisites of the required products.
void ProductManager_t::GetActiveRequirements(IndexList_t & rgiOutputList) const
{
	// Make a list of installed main products:
	IndexList_t rgiMainInstalledProducts;

	// Iterate through every product:
	for (int iProduct = 0; iProduct < kctSoftwareProducts; iProduct++)
	{
		// Only deal with this product if it is installed:
		if (!Products[iProduct].TestPresence())
			continue;

		rgiMainInstalledProducts.Add(iProduct);
	}

	GetActiveRequirements(rgiMainInstalledProducts, rgiOutputList, true, true);
}

bool ProductManager_t::PriorInstallationFailed(int iProduct) const
{
	CheckProductIndex(iProduct);

	return (Products[iProduct].m_InstallStatus >= SoftwareProduct::InstallFailed);
}

bool ProductManager_t::IsInstallable(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].IsInstallable();
}

// Installs specified product.
// Assumes that all prerequisite products have been installed.
bool ProductManager_t::InstallProduct(int iProduct)
{
	CheckProductIndex(iProduct);

	bool fResult = Products[iProduct].Install();
	g_ProgRecord.WriteInstallStatus(iProduct, Products[iProduct].m_InstallStatus);

	return fResult;
}

// Produce dynamically allocated text describing the full prerequisite list for the given
// products.
// Caller must delete[] the returned text.
char * ProductManager_t::GenFullPrerequisiteReport(const IndexList_t & rgiProducts,
												   int nIndent) const
{
	if (rgiProducts.GetCount() == 0)
		return NULL;

	char * pszReport = NULL;
	char * pszWksp = NULL;

	// Create a 1st-order prerequisite list for each given product:
	IndexList_t * rgrgiPrerequisites = new IndexList_t [rgiProducts.GetCount()];
	// Also create one list containing all 1st-order prerequistes:
	IndexList_t rgiPrerequisites;

	for (int i = 0; i < rgiProducts.GetCount(); i++)
	{
		IndexList_t rgiSingleProduct;
		rgiSingleProduct.Add(rgiProducts[i]);
		GetActivePrerequisites(rgiSingleProduct, rgrgiPrerequisites[i], false);
		rgiPrerequisites.Add(rgrgiPrerequisites[i]);
	}

	// Iterate through the complete list, listing each prerequisite product:
	for (int i = 0; i < rgiPrerequisites.GetCount(); i++)
	{
		pszWksp = new_ind_sprintf(nIndent, "%s - %s:", GetName(rgiPrerequisites[i]),
			FetchString(IDC_MESSAGE_NEEDED_BY));
		new_sprintf_concat(pszReport, 1, pszWksp);
		delete[] pszWksp;
		pszWksp = NULL;
		
		// List each product that needs the current prerequisite:
		for (int i2 = 0; i2 < rgiProducts.GetCount(); i2++)
		{
			if (rgrgiPrerequisites[i2].Contains(rgiPrerequisites[i]))
			{
				pszWksp = new_ind_sprintf(nIndent + 4, GetName(rgiProducts[i2]));
				new_sprintf_concat(pszReport, 1, pszWksp);
				delete[] pszWksp;
				pszWksp = NULL;
			}
		}
	}

	// Clean up:
	for (int i = 0; i < rgiProducts.GetCount(); i++)
		rgrgiPrerequisites[i].Flush();
	delete[] rgrgiPrerequisites;
	rgrgiPrerequisites = NULL;

	// Recurse, to see if any listed prerequisites have prerequistes of their own:
	char * pszNextLevel = GenFullPrerequisiteReport(rgiPrerequisites, nIndent + 4);
	if (pszNextLevel)
	{
		pszWksp = new_ind_sprintf(nIndent, FetchString(IDC_MESSAGE_MORE_PREREQUISITES));
		new_sprintf_concat(pszReport, 2, pszWksp);
		delete[] pszWksp;
		pszWksp = NULL;

		new_sprintf_concat(pszReport, 1, pszNextLevel);
		delete[] pszNextLevel;
		pszNextLevel = NULL;
	}
	
	return pszReport;
}

// Produce dynamically allocated text describing the full requirement list for the given
// products.
// Caller must delete[] the returned text.
char * ProductManager_t::GenFullRequirementReport(const IndexList_t & rgiProducts, int nIndent) const
{
	if (rgiProducts.GetCount() == 0)
		return NULL;

	char * pszReport = NULL;
	char * pszWksp = NULL;

	// Create a 1st-order requirement list for each given product:
	IndexList_t * rgrgiRequirements = new IndexList_t [rgiProducts.GetCount()];
	// Also create one list containing all 1st-order requirements:
	IndexList_t rgiRequirements;

	for (int i = 0; i < rgiProducts.GetCount(); i++)
	{
		IndexList_t rgiSingleProduct;
		rgiSingleProduct.Add(rgiProducts[i]);
		GetActiveRequirements(rgiSingleProduct, rgrgiRequirements[i], false, false);
		rgiRequirements.Add(rgrgiRequirements[i]);
	}

	// Iterate through the complete list, listing each required product:
	for (int i = 0; i < rgiRequirements.GetCount(); i++)
	{
		pszWksp = new_ind_sprintf(nIndent, "%s - %s:", GetName(rgiRequirements[i]),
			FetchString(IDC_MESSAGE_NEEDED_BY));
		new_sprintf_concat(pszReport, 1, pszWksp);
		delete[] pszWksp;
		pszWksp = NULL;
		
		// List each product that needs the current requirement:
		for (int i2 = 0; i2 < rgiProducts.GetCount(); i2++)
		{
			if (rgrgiRequirements[i2].Contains(rgiRequirements[i]))
			{
				pszWksp = new_ind_sprintf(nIndent + 4, GetName(rgiProducts[i2]));
				new_sprintf_concat(pszReport, 1, pszWksp);
				delete[] pszWksp;
				pszWksp = NULL;
			}
		}
	}

	// Clean up:
	for (int i = 0; i < rgiProducts.GetCount(); i++)
		rgrgiRequirements[i].Flush();
	delete[] rgrgiRequirements;
	rgrgiRequirements = NULL;

	// See if any listed requirements have any prerequistes:
	char * pszPrerequisites = GenFullPrerequisiteReport(rgiRequirements, nIndent + 4);
	if (pszPrerequisites)
	{
		pszWksp = new_ind_sprintf(nIndent, FetchString(IDC_MESSAGE_MORE_PREREQUISITES));
		new_sprintf_concat(pszReport, 2, pszWksp);
		delete[] pszWksp;
		pszWksp = NULL;

		new_sprintf_concat(pszReport, 1, pszPrerequisites);
		delete[] pszPrerequisites;
		pszPrerequisites = NULL;
	}
	// Get the full list of prerequisites:
	IndexList_t rgiPrerequisites;
	GetActivePrerequisites(rgiProducts, rgiPrerequisites, true);
	// Add to the list of requirements:
	rgiRequirements.Add(rgiPrerequisites);

	// Recurse, to see if any listed requirements have requirements of their own:
	// (This might result in multi-order prerequisites being repeated)
	char * pszNextLevel = GenFullRequirementReport(rgiRequirements, nIndent + 4);
	if (pszNextLevel)
	{
		pszWksp = new_ind_sprintf(nIndent, FetchString(IDC_MESSAGE_MORE_REQUIREMENTS));
		new_sprintf_concat(pszReport, 2, pszWksp);
		delete[] pszWksp;
		pszWksp = NULL;

		new_sprintf_concat(pszReport, 1, pszNextLevel);
		delete[] pszNextLevel;
		pszNextLevel = NULL;
	}
	
	return pszReport;
}

// Produce dynamically allocated text describing the given product which failed to install
// and the reasons for the failure.
// Caller must delete[] the returned text.
char * ProductManager_t::GenInstallFailureReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	// Check that this product did in fact fail to install:
	if (Products[iProduct].m_InstallStatus == SoftwareProduct::InstallSucceeded)
		return NULL; // No, it installed fine.

	// State that the product failed to install:
	char * pszReport = new_ind_sprintf(nIndent, "%s: ", GetName(iProduct));

	// See what the reason for the failure was:
	int ridReason = 0;
	char * pszExtraInfo = NULL;
	switch (Products[iProduct].m_InstallStatus)
	{
	case SoftwareProduct::InstallNotAttempted:
		ridReason = IDC_ERROR_INSTALL_NEVER;
		break;
	case SoftwareProduct::InstallFailed:
		ridReason = IDC_ERROR_INSTALL_ERROR;
		break;
	case SoftwareProduct::InstallFailedInterrupted:
		ridReason = IDC_ERROR_INSTALL_INTERRUPTED;
		break;
	case SoftwareProduct::InstallFailedUserAbandoned:
		ridReason = IDC_ERROR_INSTALL_USER_SKIP;
		break;
	case SoftwareProduct::InstallFailedNeededWin2KOrMore:
		ridReason = IDC_ERROR_INSTALL_NEED_WIN2K;
		break;
	case SoftwareProduct::InstallFailedNeededAdmin:
		ridReason = IDC_ERROR_INSTALL_NEED_ADMIN;
		break;
	case SoftwareProduct::InstallFailedFileNotFound:
		ridReason = IDC_ERROR_INSTALL_FILE_NOT_FOUND;
		if (Products[iProduct].GetDownloadUrl())
			pszExtraInfo = new_sprintf(" %s %s", FetchString(IDC_ERROR_INSTALL_FILE_NOT_FOUND_EXTRA), Products[iProduct].GetDownloadUrl());
		break;
	}
	if (ridReason)
	{
		new_sprintf_concat(pszReport, 0, FetchString(ridReason));
		if (pszExtraInfo)
		{
			new_sprintf_concat(pszReport, 0, pszExtraInfo);
			delete[] pszExtraInfo;
			pszExtraInfo = NULL;
		}

		// See if any of its prerequisites are missing:
		char * pszPrereqRpt = GenPrerequisiteFailureReport(iProduct, nIndent);
		if (pszPrereqRpt)
		{
			new_sprintf_concat(pszReport, 1, pszPrereqRpt);
			delete[] pszPrereqRpt;
			pszPrereqRpt = NULL;
		}
	}
	return pszReport;
}


// Produce dynamically allocated text describing which prerequisites of the given product failed
// to install.
// Caller must delete[] the returned text.
char * ProductManager_t::GenPrerequisiteFailureReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	char * pszReport = NULL;

	IndexList_t rgiPrerequisites;
	IndexList_t rgiSingleProduct;
	rgiSingleProduct.Add(iProduct);
	GetActivePrerequisites(rgiSingleProduct, rgiPrerequisites, false);
	if (rgiPrerequisites.GetCount() > 0)
	{
		pszReport = new_ind_sprintf(nIndent, FetchString(IDC_ERROR_INSTALL_PREREQUISITES),
			GetName(iProduct));

		// List each prerequisite:
		for (int i = 0; i < rgiPrerequisites.GetCount(); i++)
		{
			char * pszPrereqRpt = GenInstallFailureReport(rgiPrerequisites[i], nIndent + 4);
			if (pszPrereqRpt)
			{
				new_sprintf_concat(pszReport, 1, pszPrereqRpt);
				delete[] pszPrereqRpt;
				pszPrereqRpt = NULL;
			}
		}
	}
	return pszReport;
}

// Produce dynamically allocated text describing which of the given products failed to install
// and why.
// Caller must delete[] the returned text.
char * ProductManager_t::GenInstallFailureReport(IndexList_t & rgiCandidateProducts,
												 int nIndent) const
{
	char * pszReport = NULL;

	// See if any candidate products failed to install:
	for (int i = 0; i < rgiCandidateProducts.GetCount(); i++)
	{
		int iProduct = rgiCandidateProducts[i];
		char * pszProductRpt = GenInstallFailureReport(iProduct, nIndent);
		if (pszProductRpt)
		{
			new_sprintf_concat(pszReport, 1, pszProductRpt);
			delete[] pszProductRpt;
			pszProductRpt = NULL;
		}
	}
	return pszReport;
}

// Produce dynamically allocated text describing which installed products were relying on the
// given product being installed, and the consequences now that it is missing.
// Caller must delete[] the returned text.
char * ProductManager_t::GenMissingRequirementConsequencesReport(int iProduct,
																 int nIndent) const
{
	CheckProductIndex(iProduct);

	// Check that the given product really is missing:
	if (Products[iProduct].TestPresence())
		return NULL; // No, it was installed fine.

	char * pszReport = NULL;
	char * pszWksp;

	// Iterate through all products:
	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		// See if the current product is installed:
		if (Products[i].TestPresence())
		{
			// See if the current product required the given product:
			if (Products[i].m_rgiRequirements.Contains(iProduct))
			{
				// Get the original dependency mapping:
				int iMapping = GetMappingIndex(depRequirement, i, iProduct);
				if (iMapping >= 0)
				{
					// Report the consequences:
					pszWksp = new_ind_sprintf(nIndent, "%s: %s", GetName(i),
						s_DependecyMaps[depRequirement][iMapping].m_kpszFailureMessage);
					new_sprintf_concat(pszReport, 1, pszWksp);
					delete[] pszWksp;
					pszWksp = NULL;
				}
			} // End if given product was required by current product
		} // End if current product was installed
	} // Next product
	return pszReport;
}

// Produce dynamically allocated text describing the reason for the omission of the given
// product, a list of which products require the given product, and the consequneces for
// each product in the list.
// Caller must delete[] the returned text.
char * ProductManager_t::GenMissingRequirementReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	char * pszReport = NULL;

	// Report why the product failed to install:
	char * pszProductRpt = GenInstallFailureReport(iProduct, nIndent);
	if (pszProductRpt)
	{
		new_sprintf_concat(pszReport, 1, pszProductRpt);
		delete[] pszProductRpt;
		pszProductRpt = NULL;

		// Now report on the consequences:
		char * pszConsequences = GenMissingRequirementConsequencesReport(iProduct, nIndent + 4);
		if (pszConsequences)
		{			
			char * pszWksp = new_ind_sprintf(nIndent, FetchString(IDC_MESSAGE_CONSEQUENCES));
			new_sprintf_concat(pszReport, 1, pszWksp);
			delete[] pszWksp;
			pszWksp = NULL;

			new_sprintf_concat(pszReport, 1, pszConsequences);
			delete[] pszConsequences;
			pszConsequences = NULL;
		}
	}
	return pszReport;
}

// Produce dynamically allocated text describing which required products are still missing,
// including the reason for their omission and the consequences.
// Caller must delete[] the returned text.
char * ProductManager_t::GenMissingRequirementReport(int nIndent) const
{
	char * pszReport = NULL;

	// List of all missing requirements:
	IndexList_t rgiMissingList;

	// Iterate through all products:
	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		// See if the current product is installed:
		if (Products[i].TestPresence())
		{
			// See if any immediate requirements are missing:
			IndexList_t rgiSingleProduct;
			rgiSingleProduct.Add(i);
			GetActiveRequirements(rgiSingleProduct, rgiMissingList, false, false);
		}
	}

	// Iterate through all missing products, and report the reason for the failure and the
	// consequences:
	for (int i = 0; i < rgiMissingList.GetCount(); i++)
	{
		char * pszDependRpt = GenMissingRequirementReport(rgiMissingList[i], nIndent);
		if (pszDependRpt)
		{
			new_sprintf_concat(pszReport, 1, pszDependRpt);
			delete[] pszDependRpt;
			pszDependRpt = NULL;
		}
	}
	return pszReport;
}

// Produce dynamically allocated text describing the dependency relationships as specified by
// iReportType.
// Caller must delete[] the returned text.
char * ProductManager_t::GenReport(int iReportType, IndexList_t * prgiProducts) const
{
	char * pszReport = NULL;
	char * pszTemp = NULL;

	switch (iReportType)
	{
	case rptPrerequisitesShort:
		if (prgiProducts)
		{
			IndexList_t rgiTempList;
			g_Log.Write("Testing for prerequisites for basic report...");
			g_Log.Indent();
			GetActivePrerequisites(*prgiProducts, rgiTempList, true);
			g_Log.Unindent();
			g_Log.Write("...Done (prerequisites)");
			for (int i = 0; i < rgiTempList.GetCount(); i++)
				new_sprintf_concat(pszReport, int(i > 0), GetName(rgiTempList[i]));
		}
		break;
	case rptPrerequisitesFull:
		if (prgiProducts)
		{
			g_Log.Write("Testing for prerequisites for full report...");
			g_Log.Indent();
			pszTemp = GenFullPrerequisiteReport(*prgiProducts);
			g_Log.Unindent();
			g_Log.Write("...Done (prerequisites)");
			new_sprintf_concat(pszReport, 0, pszTemp);
			delete[] pszTemp;
			pszTemp = NULL;
		}
		break;
	case rptRequirementsShort:
		{
			IndexList_t rgiTempList;
			g_Log.Write("Testing for requirements for basic report...");
			g_Log.Indent();
			GetActiveRequirements(rgiTempList);
			g_Log.Unindent();
			g_Log.Write("...Done (requirements)");
			for (int i = 0; i < rgiTempList.GetCount(); i++)
				new_sprintf_concat(pszReport, int(i > 0), GetName(rgiTempList[i]));
		}
		break;
	case rptRequirementsFull:
		g_Log.Write("Testing for requirements for full report...");
		g_Log.Indent();
		if (prgiProducts)
		{			
			pszTemp = GenFullRequirementReport(*prgiProducts);			
			new_sprintf_concat(pszReport, 0, pszTemp);
			delete[] pszTemp;
			pszTemp = NULL;
		}
		else
		{
			// Create a list of all installed main products:
			IndexList_t rgiMainInstalled;
			for (int i = 0; i < kctSoftwareProducts; i++)
			{
				// See if the current product is installed:
				if (Products[i].TestPresence())
					rgiMainInstalled.Add(i);
			}
			pszTemp = GenFullRequirementReport(rgiMainInstalled);
			new_sprintf_concat(pszReport, 0, pszTemp);
			delete[] pszTemp;
			pszTemp = NULL;
		}
		g_Log.Unindent();
		g_Log.Write("...Done (requirements)");
		break;
	case rptFinal:
		g_Log.Write("Testing for final report...");
		g_Log.Indent();
		if (prgiProducts)
		{
			pszTemp = GenInstallFailureReport(*prgiProducts);
			new_sprintf_concat(pszReport, 0, pszTemp);
			delete[] pszTemp;
			pszTemp = NULL;
		}
		pszTemp = GenMissingRequirementReport();
		g_Log.Unindent();
		g_Log.Write("...Done (final report)");
		new_sprintf_concat(pszReport, 1, pszTemp);
		delete[] pszTemp;
		pszTemp = NULL;
		break;
	}
	return pszReport;
}

// Returns the product index from its internal (tag) name
int ProductManager_t::GetSoftwareProductIndex(const char * pszProduct) const
{
	if (!pszProduct)
		return -1;

	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		if (strcmp(pszProduct, Products[i].m_kpszInternalName) == 0)
			return i;
	}
	return -1;
}

// Launch the dialog that generates and displays reports.
void ProductManager_t::ShowReport(const char * pszTitle, const char * pszIntro,
								  const char * pszOkButtonText, bool fConfirmQuit,
								  bool fQuitIsError, int nType, bool fCanToggleType,
								  IndexList_t * rgiProducts) const
{
	ReportPackage_t ReportPackage;
	ReportPackage.pszTitle = pszTitle;
	ReportPackage.pszIntro = pszIntro;
	ReportPackage.pszOkButtonText = pszOkButtonText;
	ReportPackage.fConfirmCancel = fConfirmQuit;
	ReportPackage.pProductManager = this;
	ReportPackage.nRptType = nType;
	ReportPackage.fCanToggleShortAndFull = fCanToggleType;
	ReportPackage.prgiProducts = rgiProducts;

	HideStatusDialog();

	// Display dialog containing report:
	if (DialogBoxParam(GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_DIALOG_DISPLAY_REPORT), NULL,  DlgProcDisplayReport,
		(LPARAM)(&ReportPackage)) == 0 && fQuitIsError)
	{
		// User quit which means we bomb out:
		HandleError(kUserAbort, true, IDC_ERROR_USER_ABORT);
	}
}

// If there are any required products no installed, inform user via the report dialog.
// Returns true if a report was displayed.
bool ProductManager_t::ShowFinalReport() const
{
	if (RequirementsNeeded())
	{
		// Set up final report dialog, giving details about errors, etc:
		char * pszWksp = new_sprintf(FetchString(IDC_MESSAGE_FINISHED), g_pszTitle);
		new_sprintf_concat(pszWksp, 2, FetchString(IDC_ERROR_DEPENDENT_INTRO));
		new_sprintf_concat(pszWksp, 0, " %s", FetchString(IDC_ERROR_DEPENDENT_INSTRUCTION));
		new_sprintf_concat(pszWksp, 0, " %s", FetchString(IDC_ERROR_DEPENDENT_WILD_GUESS));

		ShowReport(g_pszTitle, pszWksp, "OK", false, false, rptFinal, false);

		delete[] pszWksp;
		pszWksp = NULL;

		return true;
	}
	return false;
}



// Useful class for handling selections or other groups of products:
IndexList_t::IndexList_t()
{
	m_pi = NULL;
	m_ct = 0;
}

// Copy constructor
IndexList_t::IndexList_t(IndexList_t &Copy)
{
	m_pi = NULL;
	m_ct = 0;
	CopyObject(Copy);
}

// Destructor
IndexList_t::~IndexList_t()
{
	Flush();
}

void IndexList_t::Flush()
{
	delete[] m_pi;
	m_pi = NULL;
	m_ct = 0;
}

// Index operator
int IndexList_t::operator [] (int i) const
{
	if (i < 0 || i >= m_ct)
	{
		HandleError(kFatal, true, IDC_ERROR_INTERNAL,
			"Invalid index passed to product index list.");
	}
	return m_pi[i];
}

void IndexList_t::CopyObject(const IndexList_t & Copy)
{
	Flush();
	m_ct = Copy.m_ct;
	m_pi = new int [m_ct];
	for (int i = 0; i < m_ct; i++)
		m_pi[i] = Copy.m_pi[i];
}

IndexList_t & IndexList_t::operator = (const IndexList_t & Copy)
{
	CopyObject(Copy);
	return *this;
}

int IndexList_t::GetCount() const
{
	return m_ct;
}

// Returns tru if the list contains the given index:
bool IndexList_t::Contains(int index) const
{
	for (int i = 0; i < m_ct; i++)
		if (m_pi[i] == index)
			return true;

	return false;
}

// Add an item to the list
void IndexList_t::Add(int n, bool fIgnoreDuplicates)
{
	if (fIgnoreDuplicates)
		if (Contains(n))
			return;

	int * temp = new int [1 + m_ct];
	for (int i = 0; i < m_ct; i++)
		temp[i] = m_pi[i];
	delete[] m_pi;
	m_pi = temp;
	m_pi[m_ct++] = n;
}

// Add items from another list to this one.
void IndexList_t::Add(const IndexList_t & List, bool fIgnoreDuplicates)
{
	for (int i = 0; i < List.GetCount(); i++)
		Add(List[i], fIgnoreDuplicates);
}

// Returns the nth item in the list, and removes it from the list
int IndexList_t::RemoveNthItem(int n)
{
	if (n < 0 || n >= m_ct)
	{
		HandleError(kFatal, false, IDC_ERROR_INTERNAL,
			"attempting to remove invalid-indexed item from list.");
	}
	int nResult = m_pi[n];

	for (int i = n; i < m_ct - 1; i++)
		m_pi[i] = m_pi[i + 1];

	m_ct--;

	return nResult;
}

void IndexList_t::ReplaceItem(int i, int nNew)
{
	if (i < 0 || i >= m_ct)
	{
		HandleError(kFatal, false, IDC_ERROR_INTERNAL,
			"attempting to alter invalid-indexed item in list.");
	}
	m_pi[i] = nNew;
}
