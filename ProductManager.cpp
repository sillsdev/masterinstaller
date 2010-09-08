#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <msi.h>

#include "ProductManager.h"
#include "ErrorHandler.h"
#include "ProductKeys.h"
#include "LogFile.h"
#include "UsefulStuff.h"
#include "Resource.h"
#include "DiskManager.h"
#include "Dialogs.h"
#include "PersistantProgress.h"


// Forward declaration:
class SoftwareProduct;

//	SoftwareProductTypes
//	These are the types needed in the SoftwareProduct Class.
typedef bool (* pfnAuxTestPresence)(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion, SoftwareProduct * Product);
typedef int (* pfnPreInstallation)(SoftwareProduct * Product);
typedef DWORD (* pfnInstall)(bool fFlag, SoftwareProduct * Product);
typedef int (* pfnPostInstallation)(SoftwareProduct * Product);


// Declaration of information about any software product on our CD:
class SoftwareProduct
{
public:
	//	These are the members that are initialized automatically by the processed XML file (in
	// ConfigProducts.cpp):
	const _TCHAR * m_kpszInternalName;
	const _TCHAR * m_kpszNiceName;
	const _TCHAR * m_kpszMsiProductCode;
	const _TCHAR * m_kpszMsiFeature;
	const _TCHAR * m_kpszVersionInKey;
	int m_nKeyId;
	bool m_kfOneOfOurs;
	bool m_kfIsContainer;
	bool * m_fCriticalFileFlag;
	const _TCHAR * m_pszCriticalFileFlagTrue;
	const _TCHAR * m_pszCriticalFileFlagFalse;
	int m_iCd;
	const _TCHAR * m_pszMinOS;
	const _TCHAR * m_pszMaxOS;
	bool m_fMustBeAdmin;

	bool m_fFlushPendingReboot;
	bool m_fRebootTestRegPending;
	bool m_fRebootWininit;

	bool m_fMustKillHangingWindows;

	pfnPreInstallation m_pfnPreInstallation;
	const _TCHAR * m_kpszPreInstallation;
	bool m_fIngnorePreInstallationErrors;

	bool * m_pfInstallerFlag;
	const _TCHAR * m_kpszInstallerFlagTrue;
	const _TCHAR * m_kpszInstallerFlagFalse;
	const _TCHAR * m_kpszMsiFlags;
	bool m_fTestAnsiConversion;
	const _TCHAR * m_kpszMsiVersion;
	const _TCHAR * m_kpszMsiUpgrade;
	pfnInstall m_pfnInstall;
	DWORD dwSuccessCodeOverride;

	bool m_fMustNotDelayReboot;

	const _TCHAR * m_kpszDownloadUrl;

	const _TCHAR * m_kpszCommentary;
	const _TCHAR * m_kpszStatusWindowControl;
	bool m_fStatusPauseWin98;

	pfnAuxTestPresence m_pfnAuxTestPresence;
	const _TCHAR * m_kpszAuxTestPresence;
	const _TCHAR * m_kpszTestPresenceVersion;

	pfnPostInstallation m_pfnPostInstallation;
	const _TCHAR * m_kpszPostInstallation;

	const _TCHAR * m_kpszHelpTag;
	bool m_fInternalHelpFile;
	// End of members that are initialized automatically by the processed XML file.

public:
	IndexList_t m_rgiPrerequisiteMappings;
	IndexList_t m_rgiRequirementMappings;

	enum v_Installation
	{
		InstallNotAttempted,
		InstallSucceeded,
		InstallFailed,
		InstallFailedInterrupted,
		InstallFailedUserAbandoned,
		InstallFailedNeededBetterOS,
		InstallFailedNeededWorseOS,
		InstallFailedNeededAdmin,
		InstallFailedFileNotFound,
	};
	v_Installation m_InstallStatus;
	bool m_fCriticalFileLanguageUnavailable;
	_TCHAR * m_pszCriticalFile;

	bool CriticalFileLanguageUnavailable();
	const _TCHAR * GetDownloadUrl() { return m_kpszDownloadUrl; }
	_TCHAR * new_LanguageDecodedString(const _TCHAR * pszTemplate);
	_TCHAR * new_InterpretString(const _TCHAR * pszTemplate);
	bool PossibleToTestPresence() const;
	bool TestPresence();
	bool TestPresence(const _TCHAR * pszVersion);
	bool TestPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion);
	const _TCHAR * GetCriticalFile();
	bool IsContainer();
	bool IsInstallable();
	DWORD RunInstaller();
	bool Install();
	void KillHangingWindows();
	const _TCHAR * EffectiveMinOS();
	const _TCHAR * EffectiveMaxOS();
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
_TCHAR * SoftwareProduct::new_LanguageDecodedString(const _TCHAR * pszTemplate)
{
	if (!pszTemplate)
		return NULL;

	// Make a working copy that we can manipulate:
	_TCHAR * pszTemplateCopy = my_strdup(pszTemplate);

	g_Log.Write(_T("Examining %s for language identifiers..."), pszTemplate);
	g_Log.Indent();

	// Search for "$LANG$" string:
	const _TCHAR * kpszToken = _T("$LANG$");
	_TCHAR * pszNextToken = _tcsstr(pszTemplateCopy, kpszToken);
	if (!pszNextToken)
	{
		// There are no language specifiers in the string:
		g_Log.Write(_T("No language identifiers found."));
		g_Log.Unindent();
		g_Log.Write(_T("...Done."));

		return pszTemplateCopy;
	}
	do
	{
		g_Log.Write(_T("Found language identifier %s."), pszNextToken);

		_TCHAR * pszToken = pszNextToken;
		bool fSubLang = false;
		int nTokenLen = (int)_tcslen(kpszToken);
		const _TCHAR * kpszSubToken = _T("$LANG$SUB$");
		// See if Sublanguage is included:
		if (_tcsncmp(pszToken, kpszSubToken, _tcslen(kpszSubToken)) == 0)
		{
			g_Log.Write(_T("Found sub-language."));
			fSubLang = true;
			nTokenLen = (int)_tcslen(kpszSubToken);
		}

		// Read number part of token:
		pszToken += nTokenLen;
		DWORD langid = _tstoi(pszToken);

		g_Log.Write(_T("Language ID = %d."), langid);

		// Jump past end of token:
		pszToken = _tcschr(pszToken, _TCHAR('$'));
		pszToken++;

		// Find next "$LANG$" token:
		pszNextToken = _tcsstr(pszToken, kpszToken);

		g_Log.Write(_T("Windows Language ID = %d."), g_langidWindowsLanguage);

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
			_TCHAR * pszResult = my_strdup(pszToken);		
			delete[] pszTemplateCopy;

			g_Log.Write(_T("Windows Language ID matches candidate ID. Returning %s"), pszResult);

			g_Log.Unindent();
			g_Log.Write(_T("...Done."));

			return pszResult;
		}
		else
			g_Log.Write(_T("Windows Language ID does not match candidate ID."));

	} while (pszNextToken);

	g_Log.Write(_T("Windows Language ID did not match any candidate IDs. Returning NULL."));

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));

	delete[] pszTemplateCopy;
	return NULL;
}

// Parses the given string, replacing special tokens with runtime data. Returns newly created
// string with interpretted data.
// Caller must delete[] return value.
_TCHAR * SoftwareProduct::new_InterpretString(const _TCHAR * pszTemplate)
{
	if (!pszTemplate)
		return NULL;

	_TCHAR * pszResult = NULL;
	// Make a working copy that we can manipulate:
	_TCHAR * pszTemplateCopy = my_strdup(pszTemplate);

	// Search for "$CriticalFile$" string:
	const _TCHAR * kpszCritFileToken = _T("$CriticalFile$");
	_TCHAR * pszCritFileToken = _tcsstr(pszTemplateCopy, kpszCritFileToken);
	if (pszCritFileToken)
	{
		// Cut off string at the token:
		*pszCritFileToken = 0;
		// Got the token, so perform the substitution:
		_TCHAR * pszIntermediate = new_sprintf(_T("%s%s%s"), pszTemplateCopy, GetCriticalFile(),
			(pszCritFileToken + _tcslen(kpszCritFileToken)));

		// Now recurse, to see if any more text needs interpretting:
		pszResult = new_InterpretString(pszIntermediate);
		delete[] pszIntermediate;
	}

	// Search for "$SID$" string, used to represent the localized name of a well-known SID account:
	const _TCHAR * kpszSidToken = _T("$SID$");
	_TCHAR * pszSidToken = _tcsstr(pszTemplateCopy, kpszSidToken);
	if (pszSidToken)
	{
		// Cut off string at the token:
		*pszSidToken = 0;
		_TCHAR * pszRemainder = &(pszSidToken[_tcslen(kpszSidToken)]);
		// Get the SID index:
		int SidIndex = _tstoi(pszRemainder);
		// Get the remainder of the string after the SID index:
		pszRemainder = _tcsstr(pszRemainder, _T("$"));
		pszRemainder++;
		// Get the account name with domain:
		_TCHAR * pszAccount = CreateAccountNameFromWellKnownSidIndex(SidIndex);
		if (!pszAccount)
		{
			_TCHAR * pszMsg = new_sprintf(_T("A request to evaluate Well Known SID %d failed."), SidIndex);
			HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, pszMsg);
			delete[] pszMsg;
			return pszTemplateCopy;
		}

		// Perform the substitution:
		_TCHAR * pszIntermediate = new_sprintf(_T("%s%s%s"), pszTemplateCopy, pszAccount,
			pszRemainder);

		delete[] pszAccount;

		// Now recurse, to see if any more text needs interpretting:
		pszResult = new_InterpretString(pszIntermediate);
		delete[] pszIntermediate;
	}

	// Search for '%' character (environment variable):
	_TCHAR * pszFirstPercent = _tcschr(pszTemplateCopy, _TCHAR('%'));
	if (pszFirstPercent)
	{
		// We have a '%' _character. See if there is a second:
		_TCHAR * pszSecondPercent = _tcschr(pszFirstPercent + 1, _TCHAR('%'));
		if (pszSecondPercent)
		{
			// There are two '%' characters, so we will assume we have to substitute text with
			// the value of the given environment variable:
			*pszFirstPercent = 0;
			*pszSecondPercent = 0;
			_TCHAR * pszEnvVarName = pszFirstPercent + 1;
			const int cchEnvValue = 32767;
			_TCHAR szEnvValue[cchEnvValue];
			if (!GetEnvironmentVariable(pszEnvVarName, szEnvValue, cchEnvValue))
			{
				_TCHAR * pszMsg = new_sprintf(_T("A request to evaluate Environment Variable \"%s\"")
					_T(" failed."), pszEnvVarName);
				HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, pszMsg);
				delete[] pszMsg;
				return pszTemplateCopy;
			}
			// Got the value, so perform the substitution:
			_TCHAR * pszIntermediate = new_sprintf(_T("%s%s%s"), pszTemplateCopy, szEnvValue,
				(pszSecondPercent + 1));

			// Now recurse, to see if any more text needs interpretting:
			pszResult = new_InterpretString(pszIntermediate);
			delete[] pszIntermediate;
		}
	} // End search for environment variables

	// Search for '[' character (registry data):
	_TCHAR * pszOpenBracket = _tcschr(pszTemplateCopy, _TCHAR('['));
	if (pszOpenBracket)
	{
		// We have a '[' character. See if there is a ']':
		_TCHAR * pszCloseBracket = _tcschr(pszOpenBracket + 1, _TCHAR(']'));
		if (pszCloseBracket)
		{
			// There are matching brackets, so we will assume we have to substitute text with
			// the value of the given registry data:
			*pszOpenBracket = 0;
			*pszCloseBracket = 0;
			_TCHAR * pszRegKey = pszOpenBracket + 1;
			_TCHAR * pszRegKeyCopy = my_strdup(pszRegKey);
			HKEY hKeyRoot = NULL;
			_TCHAR * pszValue = _tcsstr(pszRegKey, _T("::"));
			if (pszValue)
			{
				*pszValue = 0;
				pszValue += 2;
			}
			_TCHAR * pszRegSubKey = _tcschr(pszRegKey, _TCHAR('\\'));
			if (pszRegSubKey)
			{
				*pszRegSubKey = 0;
				pszRegSubKey++;
			}
			if (_tcscmp(pszRegKey, _T("HKEY_CLASSES_ROOT")) == 0 || _tcscmp(pszRegKey, _T("HKCR")) == 0)
				hKeyRoot = HKEY_CLASSES_ROOT;
			else if (_tcscmp(pszRegKey, _T("HKEY_CURRENT_USER")) == 0 || _tcscmp(pszRegKey, _T("HKCU")) == 0)
				hKeyRoot = HKEY_CURRENT_USER;
			else if (_tcscmp(pszRegKey, _T("HKEY_LOCAL_MACHINE")) == 0 || _tcscmp(pszRegKey, _T("HKLM")) == 0)
				hKeyRoot = HKEY_LOCAL_MACHINE;
			else if (_tcscmp(pszRegKey, _T("HKEY_USERS")) == 0 || _tcscmp(pszRegKey, _T("HKU")) == 0)
				hKeyRoot = HKEY_USERS;
			else if (_tcscmp(pszRegKey, _T("HKEY_DYN_DATA")) == 0 || _tcscmp(pszRegKey, _T("HKDD")) == 0)
				hKeyRoot = HKEY_DYN_DATA;

			_TCHAR * pszKeyData = NewRegString(hKeyRoot, pszRegSubKey, pszValue);
			if (!pszKeyData)
			{
				_TCHAR * pszMsg = new_sprintf(_T("A request to evaluate Registry Data \"%s\"")
					_T(" failed."), pszRegKeyCopy);
				delete[] pszRegKeyCopy;
				pszRegKeyCopy = NULL;
				HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, pszMsg);
				delete[] pszMsg;
				return pszTemplateCopy;
			}
			delete[] pszRegKeyCopy;
			pszRegKeyCopy = NULL;

			// Got the value, so perform the substitution:
			_TCHAR * pszIntermediate = new_sprintf(_T("%s%s%s"), pszTemplateCopy, pszKeyData,
				(pszCloseBracket + 1));

			delete[] pszKeyData;
			pszKeyData = NULL;

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
bool SoftwareProduct::TestPresence(const _TCHAR * pszVersion)
{
	return TestPresence(pszVersion, pszVersion);
}

// Test for presence of a product within a range of versions. Note that if it is not possible to
// test for a particluar product, this funtion returns false - as if it is not present.
bool SoftwareProduct::TestPresence(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion)
{
	bool fPresent = false; // Assume the worst, to start with.

	// If the software product has a custom function or program, use that:
	if (m_pfnAuxTestPresence || m_kpszAuxTestPresence)
	{
		if (m_pfnAuxTestPresence)
			fPresent = m_pfnAuxTestPresence(pszMinVersion, pszMaxVersion, this);
		if (m_kpszAuxTestPresence)
			fPresent |= (0 != ExecCmd(m_kpszAuxTestPresence, _T(""), true));
	}
	else if (m_kpszMsiProductCode)
	{
		// We should have an MSI product code, and optionally a feature name.
		// Use installer API (via wrapper class):
		INSTALLSTATE state = INSTALLSTATE_UNKNOWN;

		if (m_kpszMsiFeature)
			state = MsiQueryFeatureState(m_kpszMsiProductCode, m_kpszMsiFeature);
		else
		{
			// No feature is specified, so check install state of product code:
			state = MsiQueryProductState(m_kpszMsiProductCode);
		}
		switch (state)
		{
		case INSTALLSTATE_ADVERTISED:
		case INSTALLSTATE_LOCAL:
		case INSTALLSTATE_SOURCE:
		case INSTALLSTATE_DEFAULT:
			{
				fPresent = true;

				g_Log.Write(_T(".msi installation is present."));

				// Product is installed, but check if an upgrade is required:
				if (m_kpszMsiVersion)
				{
					const int kcchVersion = 256;
					_TCHAR szIntalledVersion[kcchVersion];
					DWORD cch = kcchVersion;
					if (MsiGetProductInfo(m_kpszMsiProductCode, INSTALLPROPERTY_VERSIONSTRING,
						szIntalledVersion, &cch) == ERROR_SUCCESS)
					{
						if (!VersionInRange(szIntalledVersion, pszMinVersion,
							pszMaxVersion))
						{
							fPresent = false;
							g_Log.Write(_T("Version %s is installed, but version %s is available."),
								szIntalledVersion, m_kpszMsiVersion);
						}
					}
				} // End if the is an MsiVersion specified
			} // End case scope
		} // End switch
	} // End if product code exists

	if (!pszMinVersion)
		pszMinVersion = _T("any");
	if (!pszMaxVersion)
		pszMaxVersion = _T("any");
	g_Log.Write(_T("%s (version %s through %s): %s"), m_kpszNiceName, pszMinVersion, pszMaxVersion,
		(fPresent ? _T("present") : _T("absent")));

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
const _TCHAR * SoftwareProduct::GetCriticalFile()
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
		return _T("");
	}

	return m_pszCriticalFile;
}

// Returns true if the product is a container, i.e. a dummy for forcing installation 
// of prerequisites and/or requirements.
bool SoftwareProduct::IsContainer()
{
	return m_kfIsContainer;
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
			nResult = m_pfnInstall(*m_pfInstallerFlag, this);
		else
			nResult = m_pfnInstall(true, this);
	}
	else
	{
		// External program:
		const _TCHAR * pszCmd = m_kpszInstallerFlagTrue;

		// See if we're to use the standard Windows Installer mechanism:
		if (!m_kpszInstallerFlagTrue && m_kpszMsiProductCode)
		{
			// We'll be using Windows Installer.
			g_Log.Write(_T("Windows installer will be used."));
			const _TCHAR * kpszInstallFlags = _T("/i");

			// See if we need to test for a minor upgrade:
			if (m_kpszMsiVersion)
			{
				g_Log.Write(_T("MsiVersion %s specified."), m_kpszMsiVersion);

				// Get version number of installed instance:
				const int kcchVersion = 256;
				_TCHAR szIntalledVersion[kcchVersion];
				DWORD cch = kcchVersion;
				if (MsiGetProductInfo(m_kpszMsiProductCode, INSTALLPROPERTY_VERSIONSTRING,
					szIntalledVersion, &cch) == ERROR_SUCCESS)
				{
					g_Log.Write(_T("Version %s already installed."), szIntalledVersion);
					// Another version is installed, so check if its version is less than ours:
					__int64 nIntalledVersion = GetHugeVersion(szIntalledVersion);
					__int64 nOurVersion = GetHugeVersion(m_kpszMsiVersion);
					if (nOurVersion > nIntalledVersion)
					{
						g_Log.Write(_T("Minor upgrade will be performed."));
						kpszInstallFlags = _T("/fvomus");
						if (m_kpszMsiUpgrade)
							kpszInstallFlags = m_kpszMsiUpgrade;
					}
				}
			}
			// Run msiexec:
			_TCHAR * pszMsiExec = new_sprintf(_T("MsiExec.exe %s \"%s\" %s"), kpszInstallFlags,
				GetCriticalFile(), m_kpszMsiFlags? m_kpszMsiFlags : _T(""));

			g_Log.Write(_T("About to run \"%s\""), pszMsiExec);

			nResult = ExecCmd(pszMsiExec, _T(""), true, m_kpszNiceName,
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
				_TCHAR * pszModCmd = new_InterpretString(pszCmd);

				// This will only loop if the ANSI conversion of the command is bad, the user
				// opts to bring up the language settings dialog, and no reboot follows from
				// that:
				while (m_fTestAnsiConversion)
				{
					// We have to test the processed command line to make sure it will convert
					// safely to ANSI:
					g_Log.Write(_T("Command line '%s' must be tested for ANSI compatibility."), pszModCmd);
					BOOL fUsedDefaultChar = false;
					// This line does not compile unless Unicode is specified in compilation settings:
					WideCharToMultiByte(CP_ACP, 0, pszModCmd, -1, NULL, 0, NULL,
						&fUsedDefaultChar);

					if (fUsedDefaultChar)
					{
						// The conversion is not safe, so warn the user:
						g_Log.Write(_T("Command line did not convert properly to ANSI."));

						if (::MessageBox(NULL, FetchString(IDC_MESSAGE_ANSI_ERROR), g_pszTitle,
							MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
						{
							g_Log.Write(_T("User opted to run Regional and Language Options dialog."), pszModCmd);

							DisplayStatusText(0, FetchString(IDC_MESSAGE_INSTALLATION_PAUSED));
							DisplayStatusText(1, FetchString(IDC_MESSAGE_SET_LANGUAGE1));
							DisplayStatusText(2, FetchString(IDC_MESSAGE_SET_LANGUAGE2));

							DWORD dwResult;
							if (g_OSVersion >= OSVersion_t::Vista)
							{
								// For Vista, there is an extra tab on the Language settings dialog:
								dwResult = ExecCmd(
								_T("rundll32.exe shell32.dll,Control_RunDLL intl.cpl,,3"),
								NULL);
							}
							else // Windows XP or earlier:
							{
								dwResult = ExecCmd(
								_T("rundll32.exe shell32.dll,Control_RunDLL intl.cpl,,2"),
								NULL);
							}

							if (TestResultForRebootRequest(dwResult))
							{
								g_Log.Write(_T("Regional and Language Options dialog requested a reboot."), m_kpszNiceName);
								g_fRebootPending = true;
								FriendlyReboot();
							}
						}
						else
						{
							g_Log.Write(_T("User opted not to run Regional and Language Options dialog."), pszModCmd);
							return ERROR_CANCELLED;
						}
					}
					else // fUsedDefaultChar is false - conversion was fine.
						break;
				}

				nResult = ExecCmd(pszModCmd, _T(""), true, m_kpszNiceName,
					m_kpszStatusWindowControl);
				delete[] pszModCmd;
				pszModCmd = NULL;

				// See if we need to get the user to press continue after the installer
				// has run (because we cannot detect it ourselves):
				if (m_fStatusPauseWin98 && g_OSVersion < OSVersion_t::Win2k)
				{
					PauseOnStatusDialog(IDC_MESSAGE_MUST_WAIT);
					// Fiddle the installer's exit code:
					nResult = ERROR_SUCCESS;
				}
			}
			else
				g_Log.Write(_T("Installer command string is empty!"));
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
		g_Log.Write(_T("Testing Min OS needed (%s) against actual OS version (%s)..."),
			EffectiveMinOS(), g_OSVersion.Numeric());
		if (g_OSVersion < EffectiveMinOS())
		{
			_TCHAR * pszMinOS = g_OSVersion.MakeGeneralDescription(EffectiveMinOS());
			HandleError(kNonFatal, false, IDC_ERROR_NEED_BETTER_OS, m_kpszNiceName, pszMinOS);
			delete[] pszMinOS;
			pszMinOS = NULL;
			m_InstallStatus = InstallFailedNeededBetterOS;
			return false;
		}
		g_Log.Write(_T("...Done. Min OS requirement is satisfied."));
		g_Log.Write(_T("Testing Max OS allowed (%s) against actual OS version (%s)..."),
			EffectiveMaxOS(), g_OSVersion.Numeric());
		if (g_OSVersion > EffectiveMaxOS())
		{
			_TCHAR * pszMaxOS = g_OSVersion.MakeGeneralDescription(EffectiveMaxOS());
			HandleError(kNonFatal, false, IDC_ERROR_NEED_WORSE_OS, m_kpszNiceName, pszMaxOS);
			delete[] pszMaxOS;
			pszMaxOS = NULL;
			m_InstallStatus = InstallFailedNeededWorseOS;
			return false;
		}
		g_Log.Write(_T("...Done. Max OS requirement is satisfied."));
		if (m_fMustBeAdmin && !g_fAdministrator)
		{
			HandleError(kNonFatal, false, IDC_ERROR_NEED_ADMIN_ACCESS, m_kpszNiceName);
			m_InstallStatus = InstallFailedNeededAdmin;
			return false;
		}
		if (m_fMustKillHangingWindows)
			KillHangingWindows();
	} // End if product included in CD set.

	bool fCalledPreInstallFunction = false;

	bool fRepeat;
	do // Repeat until installation works, or user quits/skips.
	{
		fRepeat = false;

		// Make sure correct disk is inserted, unless critical file is on the Internet or product is a container:
		int nFileResult = DiskManager_t::knFileOmitted;
		if (IsContainer())
			nFileResult = DiskManager_t::knCorrectCdAlready;
		else if (_tcsncmp(GetCriticalFile(), _T("http:"), 5) == 0)
		{
			nFileResult = DiskManager_t::knFileOnInternet;
			g_Log.Write(_T("Critical file on Internet: %s"), GetCriticalFile());
		}
		else if (m_iCd >= 0)
		{
			nFileResult = g_DiskManager.EnsureCdForFile(GetCriticalFile(), m_iCd, 
				m_kpszNiceName);
			ShowStatusDialog();
		}

		if (nFileResult == DiskManager_t::knCorrectCdAlready ||
			nFileResult == DiskManager_t::knCorrectCdFinally ||
			nFileResult == DiskManager_t::knFileFoundWrongCd ||
			nFileResult == DiskManager_t::knFileOnInternet)
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
					g_Log.Write(_T("Calling pre-installation function..."));
					g_Log.Indent();
					nReturn = m_pfnPreInstallation(this);
					g_Log.Unindent();
					g_Log.Write(_T("...Done. Pre-installation function returned %d"), nReturn);
				}
				if (m_kpszPreInstallation)
				{
					g_Log.Write(_T("Executing pre-installation program '%s'..."), m_kpszPreInstallation);
					g_Log.Indent();
					nReturn = ExecCmd(m_kpszPreInstallation, _T(""), true);
					g_Log.Unindent();
					g_Log.Write(_T("...Done. Pre-installation program returned %d"), nReturn);
				}
				if (m_fIngnorePreInstallationErrors)
				{
					nReturn = 0;
					g_Log.Write(_T("Ignoring returned error code."));
				}
				fCalledPreInstallFunction = true;
				if (nReturn != 0)
				{
					g_Log.Write(_T("Skipping product installation"));
					return false;
				}
			}

			// Display status message, and write progress to log file:
			DisplayStatusText(0, FetchString(IDC_MESSAGE_INSTALLING), m_kpszNiceName);
			DisplayStatusText(1, m_kpszCommentary);

			g_Log.Write(_T("Launching installer for %s: %s."), m_kpszNiceName,
				(nFileResult == DiskManager_t::knCorrectCdAlready ? _T("correct CD already") :
				(nFileResult == DiskManager_t::knCorrectCdFinally ? _T("correct CD finally") :
				_T("file found on wrong cd"))));

			// Run installer:
			DWORD dwResult = RunInstaller();

			CheckIfStopRequested();

			// Check to see if the installation worked:
			DisplayStatusText(0, FetchString(IDC_MESSAGE_EVALUATING));
			DisplayStatusText(1, _T(""));

			bool fOfferAbortRetryOrIgnore = false;
			m_InstallStatus = InstallSucceeded;
			_TCHAR * pszRetry = NULL;

			DWORD dwSuccess = ERROR_SUCCESS;
			if (dwSuccessCodeOverride)
			{
				g_Log.Write(_T("Allowing error code of %d to mean success."),
					dwSuccessCodeOverride);
				dwSuccess = dwSuccessCodeOverride;
			}

			if (TestResultForRebootRequest(dwResult))
			{
				// Product installed and requested a reboot:
				g_Log.Write(_T("%s installed and requested a reboot."), m_kpszNiceName);
				g_fRebootPending = true;

				// If the MustNotDelayReboot flag is set, do the reboot now:
				if (m_fMustNotDelayReboot)
				{
					g_Log.Write(_T("Reboot must not be delayed."));
					FriendlyReboot();
				}
			}
			else if (dwResult != dwSuccess && dwResult != ERROR_INSTALL_USEREXIT &&
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
				g_Log.Write(_T("User quit installing %s."), m_kpszNiceName);
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
					g_Log.Write(_T("%s is still not there!"), m_kpszNiceName);
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
					g_Log.Write(_T("User opted to abort."));
					throw UserQuitException;
					break;
				case IDRETRY:
					fRepeat = true;
					g_Log.Write(_T("User opted to retry."));
					break;
				default: // IDIGNORE
					g_Log.Write(_T("User opted to ignore."));
					return false;
				}
			}
		} // End if file found
		else if (nFileResult == DiskManager_t::knUserQuit)
		{
			g_Log.Write(_T("User quit during checking disk for %s."), m_kpszNiceName);
			throw UserQuitException;
		}
		else if (nFileResult == DiskManager_t::knUserSkip)
		{
			g_Log.Write(_T("User chose to skip %s."), m_kpszNiceName);
			m_InstallStatus = InstallFailedUserAbandoned;
			return false;
		}
		else if (nFileResult == DiskManager_t::knFileNotFoundCorrectCd 
			|| nFileResult == DiskManager_t::knFileOmitted)
		{
			// The software is not on the CD, so assume it is a "Lite" CD, where
			// user has to do some work himself:
			const _TCHAR * pszUrl = GetDownloadUrl();
			if (pszUrl)
				HandleError(vErrorType(kNonFatal | kWithCopy), false, IDC_ERROR_FILE_MISSING_URL, pszUrl, m_kpszNiceName, pszUrl);
			else
				HandleError(kNonFatal, false, IDC_ERROR_FILE_MISSING, m_kpszNiceName);
			m_InstallStatus = InstallFailedFileNotFound;
			return false;
		}
	} while (fRepeat);

	if (m_pfnPostInstallation)
	{
		g_Log.Write(_T("Calling post-installation function..."));
		g_Log.Indent();
		int nReturn = m_pfnPostInstallation(this);
		g_Log.Unindent();
		g_Log.Write(_T("...Done. Post-installation function returned %d"), nReturn);
	}
	if (m_kpszPostInstallation)
	{
		g_Log.Write(_T("Executing post-installation program '%s'..."), m_kpszPostInstallation);
		g_Log.Indent();
		int nReturn = ExecCmd(m_kpszPostInstallation, _T(""), true);
		g_Log.Unindent();
		g_Log.Write(_T("...Done. Post-installation program returned %d"), nReturn);
	}

	return true;
}

void SoftwareProduct::KillHangingWindows()
{
	bool fTestHanging = true;
	while (fTestHanging)
	{
		g_Log.Write(_T("Testing for hanging windows (for %s)"), m_kpszNiceName);
		ShowStatusDialog();
		const _TCHAR * pszMsg = DisplayStatusText(0, FetchString(IDC_MESSAGE_TEST_HANGING));
		_TCHAR * pszHangingReport = GenerateHangingWindowsReport();
		if (pszHangingReport != NULL)
		{
			HideStatusDialog();

			_TCHAR * pszIntro1 = new_sprintf(FetchString(IDC_MESSAGE_HANGING_WINDOWS_INTRO_1),
				m_kpszNiceName);
			_TCHAR * pszIntro2 = new_sprintf( FetchString(IDC_MESSAGE_HANGING_WINDOWS_INTRO_2),
				m_kpszNiceName);
			_TCHAR * pszAlert = new_sprintf(_T("%s\n%s\n\n%s"), pszIntro1, pszHangingReport,
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
						g_Log.Write(_T("User opted to quit."));
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
				g_Log.Write(_T("User pressed Retry."));
				break;
			case IDIGNORE:
				g_Log.Write(_T("User pressed Ignore."));
				fTestHanging = false;
				break;
			}
		} // End if hanging windows report contained anything.
		else
		{
			fTestHanging = false;
			g_Log.Write(_T("No hanging windows."));
		}
	} // End while(fTestHanging)
}

const _TCHAR * SoftwareProduct::EffectiveMinOS()
{
	if (m_pszMinOS)
		return m_pszMinOS;

	return _T("0.0.0.0");
}

const _TCHAR * SoftwareProduct::EffectiveMaxOS()
{
	if (m_pszMaxOS)
		return m_pszMaxOS;

	return _T("32767.32767.32767.32767");
}

static bool s_fSoftwareProductsInitialized = false;

// Declaration of information about any software dependency:
struct Mapping
{
	// All members are initialized automatically by the processed XML file (in 
	// ConfigProducts.cpp).
	const _TCHAR * m_kpszKeyProduct;
	const _TCHAR * m_kpszDependentProduct;
	const _TCHAR * m_kpszMinVersion;
	const _TCHAR * m_kpszMaxVersion;
	const _TCHAR * m_kpszFailureMessage;
	const _TCHAR * m_kpszMinOs;
	const _TCHAR * m_kpszMaxOs;
};

// Forward declaration for ConfigFunctions.cpp:
bool GetFileVersion(const _TCHAR * pszFilePath, __int64 & nVersion);

// Include functions file auto-generated from XML specification:
#include "ConfigFunctions.cpp"

// Include products file auto-generated from XML specification:
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
	virtual bool TestPresence(int iProduct, const _TCHAR * pszMinVersion = NULL,
		const _TCHAR * pszMaxVersion = NULL);
	virtual const _TCHAR * GetName(int iProduct) const;
	virtual const _TCHAR * GetCommentary(int iProduct) const;
	virtual const _TCHAR * GetStatusWindowControl(int iProduct) const;
	virtual const _TCHAR * GetCriticalFile(int iProduct) const;
	virtual bool CriticalFileLanguageUnavailable(int iProduct) const;
	virtual const _TCHAR * GetDownloadUrl(int iProduct) const;
	virtual int GetCdIndex(int iProduct) const;
	virtual const _TCHAR * GetHelpTag(int iProduct) const;
	virtual bool GetHelpTagInternalFlag(int iProduct) const;
	virtual const _TCHAR * GetTestPresenceVersion(int iProduct) const;
	virtual bool IsOsHighEnough(int iProduct) const;
	virtual bool IsOsLowEnough(int iProduct) const;
	virtual const _TCHAR * GetMinOsRequirement(int iProduct) const;
	virtual const _TCHAR * GetMaxOsRequirement(int iProduct) const;
	virtual bool GetMustBeAdminFlag(int iProduct) const;
	virtual int GetNumProtectedMainProducts() const;
	virtual void DetermineAvailableMainProducts(ProductKeyHandler_t & ProductKeyHandler,
		const _TCHAR * pszKey);
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
	virtual bool IsContainer(int iProduct) const;
	virtual bool IsInstallable(int iProduct) const;
	virtual bool InstallProduct(int iProduct);
	virtual _TCHAR * GenReport(int iReportType, IndexList_t * prgiProducts = NULL) const;
	virtual void ShowReport(const _TCHAR * pszTitle, const _TCHAR * pszIntro,
		const _TCHAR * pszOkButtonText, bool fConfirmQuit, bool fQuitIsError, int nType,
		bool fCanToggleType, IndexList_t * rgiProducts = NULL) const;
	virtual bool ShowFinalReport() const;

protected:
	void EstablishOverallDependencies();
	void CheckProductIndex(int iProduct) const;
	void CheckDependencyType(int iDependType) const;
	int GetSoftwareProductIndex(const _TCHAR * pszProduct) const;

	bool m_fPermittedAfterKeyTest[kctSoftwareProducts];
	bool m_fOnlyVisibleAfterKeyTest[kctSoftwareProducts];
	int m_ctUnlockedProtectedProducts;
	int m_ctPermittedAfterKeyTest;
	int GetMappingIndex(int iDependType, int iProduct1, int iProduct2) const;
	_TCHAR * GenFullPrerequisiteReport(const IndexList_t & rgiProducts, int nIndent = 0) const;
	_TCHAR * GenFullRequirementReport(const IndexList_t & rgiProducts, int nIndent = 0) const;
	_TCHAR * GenInstallFailureReport(int iProduct, int nIndent = 0) const;
	_TCHAR * GenPrerequisiteFailureReport(int iProduct, int nIndent = 0) const;
	_TCHAR * GenInstallFailureReport(IndexList_t & rgiCandidateProducts, int nIndent = 0) const;
	_TCHAR * GenMissingRequirementConsequencesReport(int iProduct, int nIndent = 0) const;
	_TCHAR * GenMissingRequirementReport(int iProduct, int nIndent = 0) const;
	_TCHAR * GenMissingRequirementReport(int nIndent = 0) const;
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
		HandleError(kFatal, false, IDC_ERROR_INTERNAL, _T("invalid product index"));
		// Doesn't return
	}
}

// Internal check to make sure the given dependency type is valid.
void ProductManager_t::CheckDependencyType(int iDependType) const
{
	if (iDependType < 0 || iDependType >= depTotal)
	{
		HandleError(kFatal, false, IDC_ERROR_INTERNAL, _T("invalid dependency type"));
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
	g_Log.Write(_T("Populating dependency lists in each product..."));
	g_Log.Indent();

	int iMapping;
	int iDependType;
	for (iDependType = 0; iDependType < depTotal; iDependType++)
	{
		for (iMapping = 0; iMapping < s_kMappings[iDependType]; iMapping++)
		{
			const _TCHAR * kpszKeyProduct =
				s_DependecyMaps[iDependType][iMapping].m_kpszKeyProduct;
			const _TCHAR * kpszDependentProduct =
				s_DependecyMaps[iDependType][iMapping].m_kpszDependentProduct;

			// Find indexes of key and dependent products:
			int iKey = GetSoftwareProductIndex(kpszKeyProduct);
			int iDependent = GetSoftwareProductIndex(kpszDependentProduct);

			if (iKey != -1 && iDependent != -1)
			{
				if (iDependType == depPrerequisite)
				{
					g_Log.Write(_T("%s must have %s pre-installed"), kpszKeyProduct, kpszDependentProduct);
					Products[iKey].m_rgiPrerequisiteMappings.Add(iMapping);
				}
				if (iDependType == depRequirement)
				{
					g_Log.Write(_T("%s needs %s in order to run"), kpszKeyProduct, kpszDependentProduct);
					Products[iKey].m_rgiRequirementMappings.Add(iMapping);
				}
			}
		} // Next mapping
	} // Next type

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

// Returns the index into the mapping array for the given combination of products.
int ProductManager_t::GetMappingIndex(int iDependType, int iProduct1, int iProduct2) const
{
	CheckProductIndex(iProduct1);
	CheckProductIndex(iProduct2);
	CheckDependencyType(iDependType);

	const _TCHAR * pszProduct1 = Products[iProduct1].m_kpszInternalName;
	const _TCHAR * pszProduct2 = Products[iProduct2].m_kpszInternalName;

	for (int iMapping = 0; iMapping < s_kMappings[iDependType]; iMapping++)
	{
		if (_tcscmp(s_DependecyMaps[iDependType][iMapping].m_kpszKeyProduct, pszProduct1) == 0 &&
			_tcscmp(s_DependecyMaps[iDependType][iMapping].m_kpszDependentProduct, pszProduct2)
			== 0)
		{
			return iMapping;
		}
	}
	return -1;
}

bool ProductManager_t::PossibleToTestPresence(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].PossibleToTestPresence();
}

// Returns true if the given product is installed.
bool ProductManager_t::TestPresence(int iProduct, const _TCHAR * pszMinVersion,
									const _TCHAR * pszMaxVersion)
{
	CheckProductIndex(iProduct);
	return Products[iProduct].TestPresence(pszMinVersion, pszMaxVersion);
}

// Get the formal name of the given product.
const _TCHAR * ProductManager_t::GetName(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszNiceName;
}

// Return the commentary text for installing the given product.
const _TCHAR * ProductManager_t::GetCommentary(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszCommentary;
}

// Return the commentary text for installing the given product.
const _TCHAR * ProductManager_t::GetStatusWindowControl(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszStatusWindowControl;
}

// Returns the critical file of the given product. Without it, we deduce that the product is not
// on the current CD.
const _TCHAR * ProductManager_t::GetCriticalFile(int iProduct) const
{
	CheckProductIndex(iProduct);

	return Products[iProduct].GetCriticalFile();
}

bool ProductManager_t::CriticalFileLanguageUnavailable(int iProduct) const
{
	CheckProductIndex(iProduct);

	return Products[iProduct].CriticalFileLanguageUnavailable();
}

const _TCHAR * ProductManager_t::GetDownloadUrl(int iProduct) const
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

const _TCHAR * ProductManager_t::GetHelpTag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszHelpTag;
}

bool ProductManager_t::GetHelpTagInternalFlag(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_fInternalHelpFile;
}

const _TCHAR * ProductManager_t::GetTestPresenceVersion(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].m_kpszTestPresenceVersion;
}

bool ProductManager_t::IsOsHighEnough(int iProduct) const
{
	CheckProductIndex(iProduct);
	return g_OSVersion >= Products[iProduct].EffectiveMinOS();
}

bool ProductManager_t::IsOsLowEnough(int iProduct) const
{
	CheckProductIndex(iProduct);
	return g_OSVersion <= Products[iProduct].EffectiveMaxOS();
}

const _TCHAR * ProductManager_t::GetMinOsRequirement(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].EffectiveMinOS();
}

const _TCHAR * ProductManager_t::GetMaxOsRequirement(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].EffectiveMaxOS();
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
													  const _TCHAR * pszKey)
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
	g_Log.Write(_T("Generating list of active prerequisites, just to see if any are still needed..."));
	g_Log.Indent();

	// Create a list of needed prerequisites:
	IndexList_t rgiNeeded;
	GetActivePrerequisites(rgiProducts, rgiNeeded, true);

	g_Log.Unindent();
	g_Log.Write(_T("...Done. Number of active prerequisites = %d"), rgiNeeded.GetCount());

	// See if any are needed:
	return (rgiNeeded.GetCount() > 0);
}

// Determine if any required products are still needing to be installed.
bool ProductManager_t::RequirementsNeeded() const
{
	g_Log.Write(_T("Generating list of active requirements, just to see if any are still needed..."));
	g_Log.Indent();

	// Create a list of needed prerequisites:
	IndexList_t rgiNeeded;
	GetActiveRequirements(rgiNeeded);

	g_Log.Unindent();
	g_Log.Write(_T("...Done. Number of active requirements = %d"), rgiNeeded.GetCount());

	// See if any are needed:
	return (rgiNeeded.GetCount() > 0);
}

// Add into rgiOutputList the indexes of products that are non-installed prerequisites of the
// products indexed in prgiProducts.
// If fRecurse is true, the non-installed prerequisites of those required products are included,
// recursively.
void ProductManager_t::GetActivePrerequisites(const IndexList_t & rgiProducts,
											  IndexList_t & rgiOutputList, bool fRecurse) const
{
	g_Log.Write(_T("Getting active prerequisite list for list of %d products, %srecursing..."),
		rgiProducts.GetCount(), (fRecurse? _T("") : _T("not ")));
	g_Log.Indent();

	// Iterate through every product in the given list:
	for (int i = 0; i < rgiProducts.GetCount(); i++)
	{
		// Get current product:
		int iProduct = rgiProducts[i];

		// Get the current product's list of prerequisite mappings:
		IndexList_t & rgiPrereqMaps = Products[iProduct].m_rgiPrerequisiteMappings;
		IndexList_t rgiActivePrereqProducts;

		g_Log.Write(_T("Product %d (%s) has %d prerequisite mappings:"), (i + 1),
			Products[iProduct].m_kpszNiceName, rgiPrereqMaps.GetCount());
		g_Log.Indent();

		// Iterate through each prerequisite product:
		for (int i2 = 0; i2 < rgiPrereqMaps.GetCount(); i2++)
		{
			// Find index of dependent product:
			int iMapping = rgiPrereqMaps[i2];
			const _TCHAR * kpszDependentProduct = s_DependecyMaps[depPrerequisite][iMapping].m_kpszDependentProduct;
			int iPreReq = GetSoftwareProductIndex(kpszDependentProduct);

			// If the output list already contains this prerequisite, then go on to the next:
			if (rgiOutputList.Contains(iPreReq))
			{
				g_Log.Write(_T("%s (which we've already dealt with, so we will ignore it this time)"), kpszDependentProduct);
				continue;
			}

			// See if an OS version test is required on the dependency:
			bool fConditionalPassed = true;
			const _TCHAR * pszMinOs = s_DependecyMaps[depPrerequisite][iMapping].m_kpszMinOs;
			const _TCHAR * pszMaxOs = s_DependecyMaps[depPrerequisite][iMapping].m_kpszMaxOs;
			if (pszMinOs || pszMaxOs)
			{
				if (!VersionInRange(g_OSVersion.Numeric(), pszMinOs, pszMaxOs))
					fConditionalPassed = false;
			}

			if (fConditionalPassed)
			{
				// See if the required version of the prerequisite is present already:
				const _TCHAR * pszMinVersion = s_DependecyMaps[depPrerequisite][iMapping].m_kpszMinVersion;
				const _TCHAR * pszMaxVersion = s_DependecyMaps[depPrerequisite][iMapping].m_kpszMaxVersion;
				g_Log.Indent();
				bool fProductPresent = Products[iPreReq].TestPresence(pszMinVersion, pszMaxVersion);
				g_Log.Unindent();
				if (!fProductPresent)
				{
					if (pszMinOs || pszMaxOs)
					{
						g_Log.Write(_T("%s which is missing and needed on OS versions %s through %s, so we will record this dependency"),
							kpszDependentProduct, (pszMinOs? pszMinOs : _T("any")), (pszMaxOs? pszMaxOs : _T("any")));
					}
					else
						g_Log.Write(_T("%s which is missing, so we will record this dependency"), kpszDependentProduct);

					rgiOutputList.Add(iPreReq);
					rgiActivePrereqProducts.Add(iPreReq);
				}
				else
					g_Log.Write(_T("%s which is already installed, so we will ignore this dependency"), kpszDependentProduct);
			}
			else
			{
				g_Log.Write(_T("%s (which only applies for OS versions %s through %s, so doesn't matter here)"), 
					kpszDependentProduct, (pszMinOs? pszMinOs : _T("any")), (pszMaxOs? pszMaxOs : _T("any")));
			}
		}
		g_Log.Unindent();

		if (fRecurse && rgiActivePrereqProducts.GetCount() > 0)
		{
			g_Log.Write(_T("Recursing the active prerequisites of those prerequisites..."));
			g_Log.Indent();

			// Recursively add the prerequisites of the current active prerequisites:
			GetActivePrerequisites(rgiActivePrereqProducts, rgiOutputList, true);

			g_Log.Unindent();
			g_Log.Write(_T("...Done recursing active prerequisites."));
		}
	}
	g_Log.Unindent();
	g_Log.Write(_T("...Done getting active prerequisite list."));
}

// Add into rgiOutputList the indexes of products that are non-installed requirements of the
// products indexed in rgiProducts.
// If fRecursePrerequisites is true, the non-installed prerequisites of those required
// products are included, recursively.
// If fRecurseRequirements is true, the non-installed requirements of those required
// products are included, recursively.
void ProductManager_t::GetActiveRequirements(const IndexList_t & rgiProducts,
											 IndexList_t & rgiOutputList,
											 bool fRecursePrerequisites,
											 bool fRecurseRequirements) const
{
	g_Log.Write(_T("Getting active requirement list for list of %d products, %srecursing prerequisites, %srecursing requirements..."),
		rgiProducts.GetCount(), (fRecursePrerequisites? _T("") : _T("not ")), (fRecurseRequirements? _T("") : _T("not ")));
	g_Log.Indent();

	// Iterate through every product in the given list:
	for (int i = 0; i < rgiProducts.GetCount(); i++)
	{
		// Get current product:
		int iProduct = rgiProducts[i];

		// Get the current product's list of requirement mappings:
		IndexList_t & rgiReqMaps = Products[iProduct].m_rgiRequirementMappings;

		g_Log.Write(_T("Product %d (%s) has %d requirement mappings:"), (i + 1),
			Products[iProduct].m_kpszNiceName, rgiReqMaps.GetCount());
		g_Log.Indent();

		// Form a list of the current required products:
		IndexList_t rgiRequiredProducts;
		for (int ip = 0; ip < rgiReqMaps.GetCount(); ip++)
		{
			int iMapping = rgiReqMaps[ip];
			const _TCHAR * kpszDepProd = s_DependecyMaps[depRequirement][iMapping].m_kpszDependentProduct;
			int iReq = GetSoftwareProductIndex(kpszDepProd);
			rgiRequiredProducts.Add(iReq);
		}

		if (fRecursePrerequisites && rgiRequiredProducts.GetCount() > 0)
		{
			g_Log.Write(_T("Getting active prerequisites of those required products..."));
			g_Log.Indent();

			// Recursively add the prerequisites of the current required products:
			GetActivePrerequisites(rgiRequiredProducts, rgiOutputList, true);

			g_Log.Unindent();
			g_Log.Write(_T("...Done getting active prerequisites of required products."));
		}

		// Iterate through each required product:
		for (int i2 = 0; i2 < rgiReqMaps.GetCount(); i2++)
		{
			// Find index of dependent product:
			int iMapping = rgiReqMaps[i2];
			const _TCHAR * kpszDependentProduct = s_DependecyMaps[depRequirement][iMapping].m_kpszDependentProduct;
			int iReq = GetSoftwareProductIndex(kpszDependentProduct);

			// If the output list already contains this requirement, then go on to the next:
			if (rgiOutputList.Contains(iReq))
			{
				g_Log.Write(_T("%s (which we've already dealt with, so we will ignore it this time)"), kpszDependentProduct);
				continue;
			}

			// See if an OS version test is required on the dependency:
			bool fConditionalPassed = true;
			const _TCHAR * pszMinOs = s_DependecyMaps[depRequirement][iMapping].m_kpszMinOs;
			const _TCHAR * pszMaxOs = s_DependecyMaps[depRequirement][iMapping].m_kpszMaxOs;
			if (pszMinOs || pszMaxOs)
			{
				if (!VersionInRange(g_OSVersion.Numeric(), pszMinOs, pszMaxOs))
					fConditionalPassed = false;
			}

			if (fConditionalPassed)
			{
				// See if the required version of the required product is present already:
				const _TCHAR * pszMinVersion = s_DependecyMaps[depRequirement][iMapping].m_kpszMinVersion;
				const _TCHAR * pszMaxVersion = s_DependecyMaps[depRequirement][iMapping].m_kpszMaxVersion;

				g_Log.Indent();
				bool fProductPresent = Products[iReq].TestPresence(pszMinVersion, pszMaxVersion);
				g_Log.Unindent();
				if (!fProductPresent)
				{
					if (pszMinOs || pszMaxOs)
					{
						g_Log.Write(_T("%s which is missing and needed on OS versions %s through %s, so we will record this dependency"),
							kpszDependentProduct, (pszMinOs? pszMinOs : _T("any")), (pszMaxOs? pszMaxOs : _T("any")));
					}
					else
						g_Log.Write(_T("%s which is missing, so we will record this dependency"), kpszDependentProduct);

					rgiOutputList.Add(iReq);
				}
				else
					g_Log.Write(_T("%s which is already installed, so we will ignore this dependency"), kpszDependentProduct);
			}
			else
			{
				g_Log.Write(_T("%s (which only applies for OS versions %s through %s, so doesn't matter here)"), 
					kpszDependentProduct, (pszMinOs? pszMinOs : _T("any")), (pszMaxOs? pszMaxOs : _T("any")));
			}
		}
		g_Log.Unindent();

		if (fRecurseRequirements && rgiRequiredProducts.GetCount() > 0)
		{
			g_Log.Write(_T("Recursing the active requirements of those required products..."));
			g_Log.Indent();

			// Recursively add the requirements of the current required products:
			GetActiveRequirements(rgiRequiredProducts, rgiOutputList, fRecursePrerequisites,
				true);

			g_Log.Unindent();
			g_Log.Write(_T("...Done recursing active requirements of required products."));
		}
	}
	g_Log.Unindent();
	g_Log.Write(_T("...Done getting active requirement list."));
}

// Add into rgiOutputList the indexes of all products that are required (at any nested level)
// by any installed product. Includes any prerequisites of the required products.
void ProductManager_t::GetActiveRequirements(IndexList_t & rgiOutputList) const
{
	g_Log.Write(_T("About to get active requirement list for all installed products. Getting list:"));
	g_Log.Indent();

	// Make a list of installed main products:
	IndexList_t rgiMainInstalledProducts;

	// Iterate through every product:
	for (int iProduct = 0; iProduct < kctSoftwareProducts; iProduct++)
	{
		// Only deal with this product if it is installed:
		g_Log.Indent();
		bool fInstalled = Products[iProduct].TestPresence();
		g_Log.Unindent();
		if (!fInstalled)
		{
			g_Log.Write(_T("[%s is not installed]"), Products[iProduct].m_kpszNiceName);
			continue;
		}

		g_Log.Write(_T("%s is installed"), Products[iProduct].m_kpszNiceName);

		rgiMainInstalledProducts.Add(iProduct);
	}

	GetActiveRequirements(rgiMainInstalledProducts, rgiOutputList, true, true);

	g_Log.Unindent();
	g_Log.Write(_T("...Done."));
}

bool ProductManager_t::PriorInstallationFailed(int iProduct) const
{
	CheckProductIndex(iProduct);

	return (Products[iProduct].m_InstallStatus >= SoftwareProduct::InstallFailed);
}

bool ProductManager_t::IsContainer(int iProduct) const
{
	CheckProductIndex(iProduct);
	return Products[iProduct].IsContainer();
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
_TCHAR * ProductManager_t::GenFullPrerequisiteReport(const IndexList_t & rgiProducts,
												   int nIndent) const
{
	if (rgiProducts.GetCount() == 0)
		return NULL;

	_TCHAR * pszReport = NULL;
	_TCHAR * pszWksp = NULL;

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
		pszWksp = new_ind_sprintf(nIndent, _T("%s - %s:"), GetName(rgiPrerequisites[i]),
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
	_TCHAR * pszNextLevel = GenFullPrerequisiteReport(rgiPrerequisites, nIndent + 4);
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
_TCHAR * ProductManager_t::GenFullRequirementReport(const IndexList_t & rgiProducts, int nIndent) const
{
	if (rgiProducts.GetCount() == 0)
		return NULL;

	_TCHAR * pszReport = NULL;
	_TCHAR * pszWksp = NULL;

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
		pszWksp = new_ind_sprintf(nIndent, _T("%s - %s:"), GetName(rgiRequirements[i]),
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
	_TCHAR * pszPrerequisites = GenFullPrerequisiteReport(rgiRequirements, nIndent + 4);
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
	_TCHAR * pszNextLevel = GenFullRequirementReport(rgiRequirements, nIndent + 4);
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
_TCHAR * ProductManager_t::GenInstallFailureReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	// Check that this product did in fact fail to install:
	if (Products[iProduct].m_InstallStatus == SoftwareProduct::InstallSucceeded)
		return NULL; // No, it installed fine.

	// State that the product failed to install:
	_TCHAR * pszReport = new_ind_sprintf(nIndent, _T("%s: "), GetName(iProduct));

	// See what the reason for the failure was:
	int ridReason = 0;
	_TCHAR * pszExtraInfo = NULL;
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
	case SoftwareProduct::InstallFailedNeededBetterOS:
		ridReason = IDC_ERROR_INSTALL_NEED_BETTER_OS;
		break;
	case SoftwareProduct::InstallFailedNeededWorseOS:
		ridReason = IDC_ERROR_INSTALL_NEED_WORSE_OS;
		break;
	case SoftwareProduct::InstallFailedNeededAdmin:
		ridReason = IDC_ERROR_INSTALL_NEED_ADMIN;
		break;
	case SoftwareProduct::InstallFailedFileNotFound:
		ridReason = IDC_ERROR_INSTALL_FILE_NOT_FOUND;
		if (Products[iProduct].GetDownloadUrl())
			pszExtraInfo = new_sprintf(_T(" %s %s"), FetchString(IDC_ERROR_INSTALL_FILE_NOT_FOUND_EXTRA), Products[iProduct].GetDownloadUrl());
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
		_TCHAR * pszPrereqRpt = GenPrerequisiteFailureReport(iProduct, nIndent);
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
_TCHAR * ProductManager_t::GenPrerequisiteFailureReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	_TCHAR * pszReport = NULL;

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
			_TCHAR * pszPrereqRpt = GenInstallFailureReport(rgiPrerequisites[i], nIndent + 4);
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
_TCHAR * ProductManager_t::GenInstallFailureReport(IndexList_t & rgiCandidateProducts,
												 int nIndent) const
{
	_TCHAR * pszReport = NULL;

	// See if any candidate products failed to install:
	for (int i = 0; i < rgiCandidateProducts.GetCount(); i++)
	{
		int iProduct = rgiCandidateProducts[i];
		_TCHAR * pszProductRpt = GenInstallFailureReport(iProduct, nIndent);
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
_TCHAR * ProductManager_t::GenMissingRequirementConsequencesReport(int iProduct,
																   int nIndent) const
{
	CheckProductIndex(iProduct);

	// Check that the given product really is missing:
	if (Products[iProduct].TestPresence())
		return NULL; // No, it was installed fine.

	_TCHAR * pszReport = NULL;
	_TCHAR * pszWksp;

	g_Log.Write(_T("Forming report on consequences of %s having failed to install..."), Products[iProduct].m_kpszNiceName);
	g_Log.Indent();

	// Iterate through all products:
	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		// Get handy reference to the current product's requirement mappings:
		IndexList_t & rgiReqMaps = Products[i].m_rgiRequirementMappings;

		// See if the current product required the given product:
		for (int ip = 0; ip < rgiReqMaps.GetCount(); ip++)
		{
			int iMapping = rgiReqMaps[ip];
			const _TCHAR * kpszDepProd = s_DependecyMaps[depRequirement][iMapping].m_kpszDependentProduct;
			int iReq = GetSoftwareProductIndex(kpszDepProd);
			if (iReq == iProduct)
			{
				// See if the current product is missing:
				if (Products[i].TestPresence())
				{
					// Report the consequences:
					pszWksp = new_ind_sprintf(nIndent, _T("%s: %s"), GetName(i),
						s_DependecyMaps[depRequirement][iMapping].m_kpszFailureMessage);
					new_sprintf_concat(pszReport, 1, pszWksp);
					delete[] pszWksp;
					pszWksp = NULL;
				} // End if given product was required by current product
			} // Next mapping
		} // End if current product was installed
	} // Next product

	g_Log.Unindent();
	g_Log.Write(_T("...Done forming consequences report."));

	return pszReport;
}

// Produce dynamically allocated text describing the reason for the omission of the given
// product, a list of which products require the given product, and the consequneces for
// each product in the list.
// Caller must delete[] the returned text.
_TCHAR * ProductManager_t::GenMissingRequirementReport(int iProduct, int nIndent) const
{
	CheckProductIndex(iProduct);

	_TCHAR * pszReport = NULL;

	// Report why the product failed to install:
	_TCHAR * pszProductRpt = GenInstallFailureReport(iProduct, nIndent);
	if (pszProductRpt)
	{
		new_sprintf_concat(pszReport, 1, pszProductRpt);
		delete[] pszProductRpt;
		pszProductRpt = NULL;

		// Now report on the consequences:
		_TCHAR * pszConsequences = GenMissingRequirementConsequencesReport(iProduct, nIndent + 4);
		if (pszConsequences)
		{			
			_TCHAR * pszWksp = new_ind_sprintf(nIndent, FetchString(IDC_MESSAGE_CONSEQUENCES));
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
_TCHAR * ProductManager_t::GenMissingRequirementReport(int nIndent) const
{
	_TCHAR * pszReport = NULL;

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
		_TCHAR * pszDependRpt = GenMissingRequirementReport(rgiMissingList[i], nIndent);
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
_TCHAR * ProductManager_t::GenReport(int iReportType, IndexList_t * prgiProducts) const
{
	_TCHAR * pszReport = NULL;
	_TCHAR * pszTemp = NULL;

	switch (iReportType)
	{
	case rptPrerequisitesShort:
		if (prgiProducts)
		{
			IndexList_t rgiTempList;
			g_Log.Write(_T("Testing for prerequisites for basic report..."));
			g_Log.Indent();
			GetActivePrerequisites(*prgiProducts, rgiTempList, true);
			g_Log.Unindent();
			g_Log.Write(_T("...Done (prerequisites)"));
			for (int i = 0; i < rgiTempList.GetCount(); i++)
				new_sprintf_concat(pszReport, int(i > 0), GetName(rgiTempList[i]));
		}
		break;
	case rptPrerequisitesFull:
		if (prgiProducts)
		{
			g_Log.Write(_T("Testing for prerequisites for full report..."));
			g_Log.Indent();
			pszTemp = GenFullPrerequisiteReport(*prgiProducts);
			g_Log.Unindent();
			g_Log.Write(_T("...Done (prerequisites)"));
			new_sprintf_concat(pszReport, 0, pszTemp);
			delete[] pszTemp;
			pszTemp = NULL;
		}
		break;
	case rptRequirementsShort:
		{
			IndexList_t rgiTempList;
			g_Log.Write(_T("Testing for requirements for basic report..."));
			g_Log.Indent();
			GetActiveRequirements(rgiTempList);
			g_Log.Unindent();
			g_Log.Write(_T("...Done (requirements)"));
			for (int i = 0; i < rgiTempList.GetCount(); i++)
				new_sprintf_concat(pszReport, int(i > 0), GetName(rgiTempList[i]));
		}
		break;
	case rptRequirementsFull:
		g_Log.Write(_T("Testing for requirements for full report..."));
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
		g_Log.Write(_T("...Done (requirements)"));
		break;
	case rptFinal:
		g_Log.Write(_T("Testing for final report..."));
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
		g_Log.Write(_T("...Done (final report)"));
		new_sprintf_concat(pszReport, 1, pszTemp);
		delete[] pszTemp;
		pszTemp = NULL;
		break;
	}
	return pszReport;
}

// Returns the product index from its internal (tag) name
int ProductManager_t::GetSoftwareProductIndex(const _TCHAR * pszProduct) const
{
	if (!pszProduct)
		return -1;

	for (int i = 0; i < kctSoftwareProducts; i++)
	{
		if (_tcscmp(pszProduct, Products[i].m_kpszInternalName) == 0)
			return i;
	}
	return -1;
}

// Launch the dialog that generates and displays reports.
void ProductManager_t::ShowReport(const _TCHAR * pszTitle, const _TCHAR * pszIntro,
								  const _TCHAR * pszOkButtonText, bool fConfirmQuit,
								  bool fQuitIsError, int nType, bool fCanToggleType,
								  IndexList_t * rgiProducts) const
{
	if (g_fSilent)
		return;

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
		_TCHAR * pszWksp = new_sprintf(FetchString(IDC_MESSAGE_FINISHED), g_pszTitle);
		new_sprintf_concat(pszWksp, 2, FetchString(IDC_ERROR_DEPENDENT_INTRO));
		new_sprintf_concat(pszWksp, 0, _T(" %s"), FetchString(IDC_ERROR_DEPENDENT_INSTRUCTION));
		new_sprintf_concat(pszWksp, 0, _T(" %s"), FetchString(IDC_ERROR_DEPENDENT_WILD_GUESS));

		ShowReport(g_pszTitle, pszWksp, _T("OK"), false, false, rptFinal, false);

		delete[] pszWksp;
		pszWksp = NULL;

		return true;
	}
	return false;
}
