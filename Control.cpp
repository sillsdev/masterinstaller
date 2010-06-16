#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

#include "Control.h"
#include "ErrorHandler.h"
#include "ProductKeys.h"
#include "Globals.h"
#include "Resource.h"
#include "Dialogs.h"
#include "UsefulStuff.h"
#include "DiskManager.h"
#include "PersistantProgress.h"
#include "UniversalFixes.h"

const _TCHAR * MasterInstaller_t::m_kszMutexName = _T("SIL Installation Bootstrapper");
const _TCHAR * MasterInstaller_t::m_kszRegProductKeys = _T("Software\\SIL\\Installer\\ProductKeys");

MasterInstaller_t::MasterInstaller_t()
{
	m_hMutex = NULL;
	m_hShutDown = NULL;
	m_phHelpViewerThreads = NULL;
	m_cHelpViewerThreads = 0;
	m_hmodInstallerHelp2 = 0;
	Help = NULL;
	m_nPhase = -1;
	m_fInstallRequiredProducts = true;
	m_ppmProductManager = NULL;
}

MasterInstaller_t::~MasterInstaller_t()
{
	if (m_hShutDown && m_cHelpViewerThreads && m_phHelpViewerThreads)
	{
		SetEvent(m_hShutDown);
		WaitForMultipleObjects(m_cHelpViewerThreads, m_phHelpViewerThreads, true, 30000);
		CloseHandle(m_hShutDown);
		for (int i = 0; i < m_cHelpViewerThreads; i++)
			CloseHandle(m_phHelpViewerThreads[i]);
	}

	m_hShutDown = NULL;
	delete[] m_phHelpViewerThreads;
	m_phHelpViewerThreads = NULL;
	m_cHelpViewerThreads = 0;

	if (m_hmodInstallerHelp2)
	{
		FreeLibrary(m_hmodInstallerHelp2);
		m_hmodInstallerHelp2 = NULL;
	}

	// Release mutex:
	::CloseHandle(m_hMutex);
	m_hMutex = NULL;
}

void MasterInstaller_t::LaunchHelp(const _TCHAR * pszHelpTag)
{
	if (pszHelpTag && Help)
	{
		HANDLE hThread = Help(pszHelpTag, m_hShutDown);
		if (hThread)
		{
			HANDLE * temp = new HANDLE [1 + m_cHelpViewerThreads];
			for (int i = 0; i < m_cHelpViewerThreads; i++)
				temp[i] = m_phHelpViewerThreads[i];
			delete[] m_phHelpViewerThreads;
			m_phHelpViewerThreads = temp;
			m_phHelpViewerThreads[m_cHelpViewerThreads++] = hThread;
		}
	}
}

void MasterInstaller_t::Run()
{
	try
	{
		m_nPhase = -1;
		g_Log.Start();
		if (!CreateMutex())
			return;

		DoUniversalFixes();

		if (g_OSVersion >= OSVersion_t::Win2k)
		{
			// Because the API function GetSystemDefaultUILanguage does not exist
			// on Windows 98 or lower, we must not assume it is present. Instead,
			// we must interrogate the Kernel32.dll.
			// Get Windows system folder path:
			_TCHAR * pszSystemFolder = GetFolderPathNew(CSIDL_SYSTEM);
			if (pszSystemFolder)
			{
				// Generate full path of Kernel32.dll:
				_TCHAR * pszKernel32Dll = MakePath(pszSystemFolder, _T("Kernel32.dll"));

				// Get a handle to the DLL:
				HMODULE hmodKernel32 = LoadLibrary(pszKernel32Dll);

				// Delete garbage:
				delete[] pszSystemFolder;
				pszSystemFolder = NULL;
				delete[] pszKernel32Dll;
				pszKernel32Dll = NULL;

				// Check if we were successful:
				if (hmodKernel32)
				{
					typedef DWORD (WINAPI * GetSystemDefaultUILanguageFn)(void);
					GetSystemDefaultUILanguageFn _GetSystemDefaultUILanguage;

					// Now get a pointer to the function we want to use:
					_GetSystemDefaultUILanguage =
						(GetSystemDefaultUILanguageFn)GetProcAddress(hmodKernel32,
						"GetSystemDefaultUILanguage");

					if (_GetSystemDefaultUILanguage)
						g_langidWindowsLanguage = _GetSystemDefaultUILanguage();

					// While we have the handle to Kernel32.dll open, we'll do some
					// other tests to see if the g_OSVersion is accurate, or whether
					// it was duped because of the program being run in a different
					// compatibility mode, which can happen to a naïve user on Vista:
					if (g_OSVersion < OSVersion_t::Vista && 
						NULL != GetProcAddress(hmodKernel32, "GetLocaleInfoEx"))
					{
						// OS is really Vista or higher, but we're duped into thinking it's less:
						g_Log.Write(_T("OS version is reported as %s, yet GetLocaleInfoEx function exists in Kernel32.dll, indicating version 6.0 (Vista) or higher."),
							g_OSVersion.Numeric());
						HandleError(kFatal, false, IDC_ERROR_OS_VERSION_LIE_VISTA);
					}

					FreeLibrary(hmodKernel32);
					hmodKernel32 = NULL;
				}
			}
		}
		g_fAdministrator = IsCurrentUserLocalAdministrator();
		g_Log.Write(_T("Admin privileges: %s."), (g_fAdministrator? _T("true") : _T("false")));

		m_ppmProductManager = CreateProductManager();
		if (!m_ppmProductManager)
		{
			HandleError(kFatal, false, IDC_ERROR_INTERNAL,
				_T("cannot instantiate Product Manager"));
		}

		// See if we were interrupted last time around:
		if (g_ProgRecord.RecordExists())
		{
			// We were previously terminated in the middle of something, so let's see where:
			m_nPhase = g_ProgRecord.ReadPhase();
			g_Log.Write(_T("Discovered previous progress record: Phase=%d."), m_nPhase);

			// Check validity of saved record:
			if (m_nPhase < 0 || m_nPhase >= phaseMaxPhases)
			{
				HandleError(kNonFatal, false, IDC_ERROR_CORRUPT_RECORD);

				// Remove our data from registry altogether:
				g_ProgRecord.RemoveData(false);

				// Now run ourself again!
				ReRun(); // Does not return
			}
		}

		if (m_nPhase == -1)
		{
			g_Log.Write(_T("Clean start established."));

			// This is a clean start. Check if user must start from first CD in set:
			if (!g_fStartFromAnyCd)
			{			
				// Clean start should only happen from CD 1, or possibly if user has copied
				// stuff to hard drive, so if we are currently running off a CD with no startup,
				// we must tell user to insert CD index 0.
				if (g_DiskManager.CheckCdPresent(0, true) == DiskManager_t::knUserQuit)
					throw UserQuitException;
			}
			m_nPhase = phaseChoices;
		}

		// Create event so that later we can signal to our sub-threads that we're shutting down:
		m_hShutDown = CreateEvent(NULL, true, false, NULL);
		if (!m_hShutDown)
			g_Log.Write(_T("Failed to create ShutDown event."));

		// Get a pointer to the Help function in the InstallerHelp2.dll:
		g_Log.Write(_T("Attempting to load InstallerHelp2.dll."));
		m_hmodInstallerHelp2 = LoadLibrary(_T("InstallerHelp2.dll"));
		if (m_hmodInstallerHelp2)
		{
			g_Log.Write(_T("Loaded InstallerHelp2.dll."));
			Help = (HelpFn)GetProcAddress(m_hmodInstallerHelp2, "Help");
			if (!Help)
			{
				FreeLibrary(m_hmodInstallerHelp2);
				m_hmodInstallerHelp2 = NULL;
				g_Log.Write(_T("Failed to get pointer to Help function."));
			}
		}
		else
			g_Log.Write(_T("InstallerHelp2.dll not present."));

		// Flag to indicate third-party stuff is to be included. User may get a chance to reset
		// this later:
		m_fInstallRequiredProducts = true;
		bool fShowDependencies = false;

		if (m_nPhase == phaseChoices)
		{
			g_ProgRecord.WritePhase(m_nPhase);
			g_Log.Write(_T("Starting user choices phase."));

			SelectMainProducts();

			if (m_rgiChosenMainProducts.GetCount() == 0 && !m_fInstallRequiredProducts)
			{
				// User didn't select anything, so quit:
				g_Log.Write(_T("User didn't select anything."));
				MessageBox(NULL, FetchString(IDC_MESSAGE_NOTHING_SELECTED), g_pszTitle, MB_OK);
				throw UserQuitException;
			}
			// Check if any selected item has a prerequisite or requirement that has a
			// language requirement we can't meet:
			TestAndReportLanguageConflicts(m_rgiChosenMainProducts);

			// Save user's selection(s):
			g_ProgRecord.WriteMainSelectionList(m_rgiChosenMainProducts);
			m_nPhase = phaseMain;
			fShowDependencies = true;

			if (g_fManualInstall)
				g_Log.Write(_T("Manual mode: not testing for prerequisites of chosen products."));
			else
			{
				// Display Prerequisites, if any:
				g_Log.Write(_T("Testing for prerequisites of chosen products:"));
				g_Log.Indent();
				if (m_ppmProductManager->PrerequisitesNeeded(m_rgiChosenMainProducts))
				{
					g_Log.Unindent();

					// There are some prerequisites:
					g_Log.Write(_T("Prerequisites are needed."));

					// Display report dialog:
					_TCHAR * pszTitle = new_sprintf(_T("%s - %s"), g_pszTitle,
						FetchString(IDC_MESSAGE_PREREQUISITES));
					_TCHAR * pszIntro = new_sprintf(FetchString(IDC_MESSAGE_PREREQUISITE_INTRO),
						g_pszTitle, g_pszTitle);
					_TCHAR * pszContinue = new_sprintf(FetchString(IDC_MESSAGE_CONTINUE));
					m_ppmProductManager->ShowReport(pszTitle, pszIntro, pszContinue, true, true,
						IProductManager::rptPrerequisitesShort, true, &m_rgiChosenMainProducts);
					delete[] pszTitle;
					delete[] pszIntro;
					delete[] pszContinue;
				} // End if any pre-requisite products are needed.
				else
				{
					g_Log.Unindent();
					g_Log.Write(_T("No prerequisites are needed."));
				}
			}
		}
		else if (m_nPhase == phaseMain) // We were running previously
		{
			// Retrieve remains of user's previous selection(s)
			g_ProgRecord.ReadMainSelectionList(m_rgiChosenMainProducts);

			g_Log.Write(_T("Remains of user's previous selection(s):"));
			for (int i = 0; i < m_rgiChosenMainProducts.GetCount(); i++)
			{
				g_Log.Write(_T("%d: %s"), i,
					m_ppmProductManager->GetName(m_rgiChosenMainProducts[i]));
			}
		}

		if (m_nPhase == phaseMain)
		{
			// Save status of installing main installers:
			g_Log.Write(_T("Starting Main phase."));
			g_ProgRecord.WritePhase(m_nPhase);

			ShowStatusDialog();
			DisplayStatusText(0, FetchString(IDC_MESSAGE_INITIALIZING));
			DisplayStatusText(1, _T(""));

			while (m_rgiChosenMainProducts.GetCount() > 0)
			{
				CheckIfStopRequested();

				// Get index of next chosen product to be installed:
				int iCurrentProduct = m_rgiChosenMainProducts[0];

				// Install prerequisite software, unless in manual mode:
				if (!InstallPrerequisites(iCurrentProduct) && !g_fManualInstall)
				{
					// Could not install all prerequisites, so there's no point in continuing
					// with this product:
					m_rgiChosenMainProducts.RemoveNthItem(0);
					g_ProgRecord.WriteMainSelectionList(m_rgiChosenMainProducts);
					g_Log.Write(
						_T("Removed %s from chosen main products due to prerequisite failure."),
						m_ppmProductManager->GetName(iCurrentProduct));

					continue; // go to next product.
				}
				CheckIfStopRequested();

				// See if this product requires a pending reboot to be flushed first:
				TestAndPerformPendingReboot(iCurrentProduct);

				// Remove current product from selection list:
				g_Log.Write(_T("Removing %s from chosen main products prior to installation."),
					m_ppmProductManager->GetName(m_rgiChosenMainProducts[0]));
				m_rgiChosenMainProducts.RemoveNthItem(0);

				// Record current selection list:
				g_ProgRecord.WriteMainSelectionList(m_rgiChosenMainProducts);

				// Install product.
				m_ppmProductManager->InstallProduct(iCurrentProduct);
				// Any error that occurred will already have been reported to the user, so we
				// can just continue.
			} // End while there are more than 0 chosen products left

			g_Log.Write(_T("Finished installing main products."));

			m_nPhase = phaseDependencies;
			fShowDependencies = true;
		}

		CheckIfStopRequested();

		// Install dependent software, unless in manual mode:
		if (m_nPhase == phaseDependencies && m_fInstallRequiredProducts && !g_fManualInstall)
		{
			g_Log.Write(_T("Starting dependent software phase."));

			g_ProgRecord.WritePhase(m_nPhase);

			// Determine the software dependencies:
			IndexList_t rgiRequiredProducts;

			ShowStatusDialog();
			DisplayStatusText(0, FetchString(IDC_MESSAGE_DEPENDENCIES));
			DisplayStatusText(1, _T(""));

			g_Log.Write(_T("Testing for initial active requirements..."));
			g_Log.Indent();
			m_ppmProductManager->GetActiveRequirements(rgiRequiredProducts);
			g_Log.Unindent();
			g_Log.Write(_T("... Done (initial active requirements)"));

			// See if there are any dependencies we need to show:
			if (fShowDependencies && rgiRequiredProducts.GetCount() > 0)
			{
				g_Log.Write(_T("Informing user of needed software."));

				// Display dependency report dialog:
				_TCHAR * pszTitle = new_sprintf(_T("%s - %s"), g_pszTitle,
					FetchString(IDC_MESSAGE_REQUIREMENTS));
				_TCHAR * pszIntro = new_sprintf(FetchString(IDC_MESSAGE_REQUIREMENT_INTRO),
					g_pszTitle, g_pszTitle);
				_TCHAR * pszContinue = new_sprintf(FetchString(IDC_MESSAGE_CONTINUE));
				m_ppmProductManager->ShowReport(pszTitle, pszIntro, pszContinue, true, true,
					IProductManager::rptRequirementsShort, true);
				delete[] pszTitle;
				delete[] pszIntro;
				delete[] pszContinue;
				ShowStatusDialog();
			}
			// Check if any required product has a prerequisite or requirement that has a
			// language requirement we can't meet:
			TestAndReportLanguageConflicts(rgiRequiredProducts, true);

			while (rgiRequiredProducts.GetCount() > 0)
			{
				CheckIfStopRequested();

				// Get index of next required product:
				int iCurrentProduct = rgiRequiredProducts.RemoveNthItem(0);

				// It is remotely possible that an earlier installation means we no longer need
				// The current product, so let's just check:
				IndexList_t rgiTempCurrentNeeds;
				g_Log.Write(_T("Testing active requirements to see if %s is still needed..."),
					m_ppmProductManager->GetName(iCurrentProduct));
				g_Log.Indent();
				m_ppmProductManager->GetActiveRequirements(rgiTempCurrentNeeds);
				g_Log.Unindent();
				g_Log.Write(_T("...Done (requirements)"));

				if (!rgiTempCurrentNeeds.Contains(iCurrentProduct))
				{
					// Product is no longer needed
					g_Log.Write(_T("Product %s is no longer a requirement."),
						m_ppmProductManager->GetName(iCurrentProduct));
					continue; // Product is already removed from requirements list.
				}

				// Inform user of number of products remaining:
				DisplayStatusText(2, FetchString(IDC_MESSAGE_REMAINING),
					rgiRequiredProducts.GetCount());

				// Install prerequisites for current product:
				if (!InstallPrerequisites(iCurrentProduct))
					continue; // Failed, so go to next product.

				CheckIfStopRequested();

				// See if this product requires a pending reboot to be flushed first:
				TestAndPerformPendingReboot(iCurrentProduct);

				g_Log.Write(_T("About to install %s."),
					m_ppmProductManager->GetName(iCurrentProduct));

				m_ppmProductManager->InstallProduct(iCurrentProduct);
				// No need to deal with an error - it has already been reported,
				// so we can just carry on with the next product.
			} // End while there are more than 0 required products left
		}
		CheckIfStopRequested();

		_TCHAR * pszWksp = NULL;
		bool fShowFinalMessage = true;
		if (m_ppmProductManager->ShowFinalReport() || !g_fShowInstallCompleteMessage)
			fShowFinalMessage = false;
		else // No error report needed
			pszWksp = new_sprintf(FetchString(IDC_MESSAGE_FINISHED), g_pszTitle);

		UINT uType = MB_OK; // Default Message Box button.

		// See if there is a pending reboot:
		if (g_fRebootPending)
		{
			fShowFinalMessage = true;
			new_sprintf_concat(pszWksp, 0, _T("\n\n%s"),
				FetchString(IDC_MESSAGE_PENDING_REBOOT));
			uType = MB_OKCANCEL;
		}
		HideStatusDialog();

		int nResult = IDCANCEL;
			
		if (fShowFinalMessage)
			nResult = MessageBox(NULL, pszWksp, g_pszTitle, uType);
		delete[] pszWksp;
		pszWksp = NULL;

		if (g_fRebootPending && nResult == IDOK)
		{
			g_ProgRecord.RemoveData(false);
			Reboot();
		}
	}
	catch (UserQuitException_t &)
	{
		// User asked to quit, so just quit quietly.
	}
	catch (...)
	{
		// Some fatal error, which has probably already been reported, but we'll try to put the
		// log info into the Clipboard:
		bool fWritten = g_Log.WriteClipboard();
		HideStatusDialog();
		if (fWritten)
			MessageBox(NULL, FetchString(IDC_MESSAGE_LOG_IN_CLIPBOARD), g_pszTitle, MB_OK);
	}
	g_ProgRecord.RemoveData(false);
}

// Iterates over each product in the given list to determine if any of its prerequisites or
// requirements cannot be installed because their language is incompatible with the user's
// Windows language. If fIncludeGivenProducts is true, also tests each product in the list.
// Returns true if there were any conflicts. User will get a message in this method, too, and
// the offending item will be removed from the given list.
bool MasterInstaller_t::TestAndReportLanguageConflicts(IndexList_t & rgiProducts,
													   bool fIncludeGivenProducts)
{
	bool fAnyConflicts = false;

	g_Log.Write(_T("Testing for prerequisites and requirements having language compatibility issues."));
	g_Log.Write(_T("Windows language is %d"), g_langidWindowsLanguage);
	g_Log.Indent();
	for (int iList = 0; iList < rgiProducts.GetCount(); iList++)
	{
		// Get index of next product:
		int iCurrentProduct = rgiProducts[iList];

		g_Log.Write(_T("Checking prerequisites and requirements for %s."),
			m_ppmProductManager->GetName(iCurrentProduct));
		g_Log.Indent();

		// Get full list of active prerequisites and requirements:
		IndexList_t rgiNeededProducts;
		if (fIncludeGivenProducts)
			rgiNeededProducts.Add(iCurrentProduct);
		IndexList_t rgiThisProductOnly;
		rgiThisProductOnly.Add(iCurrentProduct);

		g_Log.Write(_T("Prerequisites..."));
		g_Log.Indent();
		m_ppmProductManager->GetActivePrerequisites(rgiThisProductOnly, rgiNeededProducts,
			true);
		g_Log.Unindent();
		g_Log.Write(_T("...Done."));
		g_Log.Write(_T("Requirements..."));
		g_Log.Indent();

		m_ppmProductManager->GetActiveRequirements(rgiThisProductOnly, rgiNeededProducts, true,
			true);

		g_Log.Unindent();
		g_Log.Write(_T("...Done."));

		if (rgiNeededProducts.GetCount() > 0)
		{
			g_Log.Write(_T("Checking if any of the %d dependencies of %s have a language restriction we can't honor..."),
				rgiNeededProducts.GetCount(), m_ppmProductManager->GetName(iCurrentProduct));
			g_Log.Indent();
		}
		else
		{
			g_Log.Write(_T("%s has no dependencies. No need to check languge restrictions."),
				m_ppmProductManager->GetName(iCurrentProduct));
		}

		// See if any have a language restriction we can't honor:
		IndexList_t rgiFailures;
		for (int i = 0; i < rgiNeededProducts.GetCount(); i++)
		{
			int iProd = rgiNeededProducts[i];
			if (m_ppmProductManager->CriticalFileLanguageUnavailable(iProd))
			{
				rgiFailures.Add(iProd);
				g_Log.Write(_T("%s uses an incompatible language."),
					m_ppmProductManager->GetName(iProd));
			}
		}
		if (rgiFailures.GetCount() > 0)
		{
			fAnyConflicts = true;

			_TCHAR * pszList = NULL;
			for (int i = 0; i < rgiFailures.GetCount(); i++)
			{
				new_sprintf_concat(pszList, 1, m_ppmProductManager->GetName(rgiFailures[i]));
				new_sprintf_concat(pszList, 0,
					FetchString(IDC_ERROR_LANGUAGE_INCOMPATIBLE_URL));
				new_sprintf_concat(pszList, 0,
					m_ppmProductManager->GetDownloadUrl(rgiFailures[i]));
			}
			_TCHAR * pszPart1 = my_strdup(FetchString(IDC_ERROR_LANGUAGE_INCOMPATIBLE_1));
			_TCHAR * pszPart2 = my_strdup(FetchString(IDC_ERROR_LANGUAGE_INCOMPATIBLE_2));
			_TCHAR * pszError = new_sprintf(_T("%s:\n%s\n%s\n\n%s"),
				m_ppmProductManager->GetName(iCurrentProduct), pszPart1, pszList, pszPart2);
			delete[] pszPart2;
			delete[] pszPart1;
			// Copy list contents to clipboard:
			WriteClipboardText(pszList);
			delete[] pszList;
			_TCHAR * pszTitle = new_sprintf(_T("%s - %s"), g_pszTitle,
						FetchString(IDC_ERROR_LANGUAGE_INCOMPATIBLE));
			MessageBox(NULL, pszError, pszTitle, MB_ICONSTOP | MB_OK);
			g_Log.Write(_T("Language compatibility issue reported to user: '%s' %s"), pszTitle,
				pszError);
			delete[] pszTitle;
			delete[] pszError;

			rgiProducts.RemoveNthItem(iList);
			iList--;
		} // End if there were any conflicts
			g_Log.Unindent();
			g_Log.Write(_T("...Done checking if any dependencies of %s have a language restriction we can't honor..."),
				m_ppmProductManager->GetName(iCurrentProduct));

		if (rgiNeededProducts.GetCount() > 0)
		{
			g_Log.Unindent();
			g_Log.Write(_T("Done checking prerequisites and requirements for %s."),
				m_ppmProductManager->GetName(iCurrentProduct));
		}
	} // Next item in given list
	g_Log.Unindent();
	g_Log.Write(_T("End of testing for language compatibility issues."));

	return fAnyConflicts;
}

bool MasterInstaller_t::CreateMutex()
{
	// Create mutex, used to prevent more than one instance of this program from running:
	m_hMutex = ::CreateMutex(NULL, false, m_kszMutexName);

	// Check that the mutex handle didn't already exist:
	if (ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
		g_Log.Write(_T("Discovered another instance already running."));
		// We are already running in another instance, so we'll just terminate quietly:
		return false;
	}
	if (!m_hMutex)
		g_Log.Write(_T("Could not create main Mutex. Continuing anyway."));

	return true;
}

// Returns true if the caller's process is a member of the Administrators local group.
// Caller is NOT expected to be impersonating anyone and is expected to be able to
// open its own process and process token.
// Return Value: 
//   true - Caller has Administrators local group. 
//   false - Caller does not have Administrators local group.
// Taken from the MS website:
// http://msdn2.microsoft.com/en-us/library/aa376389.aspx
bool MasterInstaller_t::IsCurrentUserLocalAdministrator()
{
	// First check if we are running Windows 98 or earlier. If so, we are an administrator:
	if (g_OSVersion < OSVersion_t::Win2k)
		return true;

	if (!_CheckTokenMembership)
		return false;

	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);

	if (b)
	{
		if (!_CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = false;
		}
		FreeSid(AdministratorsGroup);
	}
	return !!b;
}

// Determines which main products are to be installed. If any main products are protected, the
// user is asked for a product key. If more than one product is available (or the
// g_fListEvenOneProduct flag is set) then the available products are offered to the user.
// Member variable m_rgiChosenMainProducts holds the definitive list at the end.
// Errors and user quit result in thrown exceptions.
void MasterInstaller_t::SelectMainProducts()
{
	bool fAskUserForKey = false;

	if (m_ppmProductManager->GetNumProtectedMainProducts() > 0)
		fAskUserForKey = true;
	
	// If the Master Installer is configured so that Shift and Control have to be pressed
	// in order to be prompted for the product key, perform that test:
	if (g_fKeyPromptNeedsShiftCtrl)
	{
		SHORT vkControl = GetAsyncKeyState(VK_CONTROL);
		SHORT vkShift = GetAsyncKeyState(VK_SHIFT);
		fAskUserForKey = ((vkShift & 0x8000) && (vkControl & 0x8000));
	}

	if (fAskUserForKey)
	{
RestartEnterKey:

		g_Log.Write(_T("Asking user for product key."));

		_TCHAR szKey[100] = { 0 };
		GetProductKey(szKey);

		int * piAvailableMainProducts = NULL;
		bool * pfMainProductOnlyVisible = NULL;
		ProductKeyHandler_t ProductKeyHandler;
		m_ppmProductManager->DetermineAvailableMainProducts(ProductKeyHandler, szKey);

		if (m_ppmProductManager->KeyUnlockedNothing())
		{
			g_Log.Write(_T("Key did not unlock anything."));
			// User entered an invalid key, so let them enter another:
			if (MessageBox(NULL, FetchString(IDC_MESSAGE_NOTHING_UNLOCKED), g_pszTitle,
				MB_ICONWARNING | MB_YESNO) == IDYES)
			{
				g_Log.Write(_T("User chose to re-enter key."));
				goto RestartEnterKey;
			}
			g_Log.Write(_T("User chose not to re-enter key."));
		}
	}
	else // no products require security
	{
		g_Log.Write(_T("Not asking user for product key."));
	}

	// If only one product is permitted, and the configuration requires it to be shown,
	// auto select it to bypass selection dialog, unless it is already installed or
	// the user has an inappropriate operating system or privileges:
	bool fAutoSelectionDone = false;
	if (!g_fListEvenOneProduct && m_ppmProductManager->GetNumPermittedMainProducts() == 1)
	{
		m_ppmProductManager->AutoSelectAllPermittedMainProducts(m_rgiChosenMainProducts);
		int iOnlyProduct = m_rgiChosenMainProducts[0];
		g_Log.Write(_T("Only 1 product available: %s."),
			m_ppmProductManager->GetName(iOnlyProduct));
		fAutoSelectionDone = true;

		// Deal with the case where product cannot be detected:
		if (!m_ppmProductManager->PossibleToTestPresence(iOnlyProduct))
		{
			fAutoSelectionDone = false; // Cancel auto selection
			g_Log.Write(_T("Cannot detect if product is already installed, so user will get selection dialog."));
		}
		else
		{
			// Deal with the case where product is already installed:
			const _TCHAR * pszVersion = m_ppmProductManager->GetTestPresenceVersion(iOnlyProduct);
			bool fInstalled = m_ppmProductManager->TestPresence(iOnlyProduct, pszVersion,
				pszVersion);
			if (fInstalled)
			{
				fAutoSelectionDone = false; // Cancel auto selection
				g_Log.Write(_T("Product is already installed, so user will get selection dialog."));
			}
		}

		// Deal with the case where the user has an inappropriate OS:
		if (!m_ppmProductManager->IsOsHighEnough(iOnlyProduct)
			|| !m_ppmProductManager->IsOsLowEnough(iOnlyProduct))
		{
			fAutoSelectionDone = false; // Cancel auto selection
			g_Log.Write(_T("Product outside suitable OS range, so user will get selection dialog."));
		}

		// Deal with the case where the user needs admin privileges but doesn't have them:
		if (m_ppmProductManager->GetMustBeAdminFlag(iOnlyProduct) && !g_fAdministrator)
		{
			fAutoSelectionDone = false; // Cancel auto selection
			g_Log.Write(_T("Product needs admin privileges, so user will get selection dialog."));
		}
	}
	if (!fAutoSelectionDone)
	{
		// If there are no permitted products after the user didn't know to press shift & ctrl
		// to get a key prompt, then just quit silently:
		if (g_fKeyPromptNeedsShiftCtrl && !fAskUserForKey &&
			m_ppmProductManager->GetNumPermittedMainProducts() == 0)
		{
			throw UserQuitException;
		}

		if (g_fSilent && g_pCmdLineProductSelection)
		{
			// Get product choices indicated in command line:
			g_Log.Write(_T("Using product choices indicated in command line."));
			m_rgiChosenMainProducts = g_pCmdLineProductSelection->m_rgiChosen;
			m_fInstallRequiredProducts = g_pCmdLineProductSelection->m_fInstallRequiredSoftware;
		}
		else
		{
			// Offer permitted products to user:
			g_Log.Write(_T("Offering list of permitted products."));

			DlgMainProductParams_t DlgMainProductParams;
			DlgMainProductParams.pHelpLauncher = this;
			DlgMainProductParams.pProductManager = m_ppmProductManager;
			DlgMainProductParams.m_fReenterKeyAllowed = fAskUserForKey;

			MainSelectionReturn_t * MainSelectionReturn =
				reinterpret_cast<MainSelectionReturn_t *>(DialogBoxParam(GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDD_DIALOG_MAIN_PRODUCT_SELECT), NULL, DlgProcMainProductSelect,
				(LPARAM)(&DlgMainProductParams)));

			if (!MainSelectionReturn)
			{
				g_Log.Write(_T("User canceled."));
				throw UserQuitException;
			}
			if (MainSelectionReturn->m_fReenterKey)
			{
				// User pressed Re-enter Key button:
				g_Log.Write(_T("User chose to re-enter key."));
				goto RestartEnterKey;
			}
			m_rgiChosenMainProducts = MainSelectionReturn->m_rgiChosen;
			m_fInstallRequiredProducts = MainSelectionReturn->m_fInstallRequiredSoftware;

			delete MainSelectionReturn;
			MainSelectionReturn = NULL;
		}
	} // End else more than one product is available
}

// Get a Product Key from the user.
// If the user opts to quit, an exception is thrown, and this method does not return.
void MasterInstaller_t::GetProductKey(_TCHAR szKey[100])
{
	// Limit the number of attempts the user can have at entering a key:
	static int ctRetries = 0;
	const int kctMaxRetries = 4;

	if (ctRetries >= kctMaxRetries)
	{
		// Report that limit is exceeded:
		g_Log.Write(_T("User has had %d attempts to enter key - limit reached."), ctRetries);
		HandleError(kUserAbort, false, IDC_ERROR_TOO_MANY_KEY_GUESSES);
	}

	ctRetries++;
	g_Log.Write(_T("Key entry - attempt %d."), ctRetries);

	// Load our application's icon to make dialog look nice:
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDR_MAIN_ICON);

	// Call the GetKey function in the DLL:
	HideStatusDialog();
	ProductKeyHandler_t ProductKeyHandler;
	int nKeyResult = ProductKeyHandler.GetKeyAltTitle(szKey, true, g_pszTitle, g_pszGetKeyTitle,
		hIcon);
	if (hIcon)
	{
		DestroyIcon(hIcon);
		hIcon = NULL;
	}

	// See if user intended to quit:
	if (nKeyResult == 2)
	{
		// User pressed cancel and meant it:
		g_Log.Write(_T("User canceled."));
		throw UserQuitException;
	}
}

// Launch a new instance of this process, and exit the current one.
// Effectively restarts our process. Useful when an error is detected and corrected at startup.
// This function does not return.
void MasterInstaller_t::ReRun()
{
	g_Log.Write(_T("Restarting master installer."));

	const int knLen = MAX_PATH + 20;
	_TCHAR szPath[knLen];
	GetModuleFileName(NULL, szPath, knLen);

	// Release mutex:
	::CloseHandle(m_hMutex);
	m_hMutex = NULL;

	// Run ourself!
	ExecCmd(szPath, NULL, false);

	// Quit, to let the new instance take over:
	ExitProcess(0);
}

// See if there is a pending reboot which must be activated before installing the given product.
// If so, reboot.
void MasterInstaller_t::TestAndPerformPendingReboot(int iProduct)
{
	bool fTestRegPendingRename = m_ppmProductManager->GetRebootTestRegPendingFlag(iProduct);
	bool fTestWininit = m_ppmProductManager->GetRebootWininitFlag(iProduct);

	bool fRebootPending = g_fRebootPending;
	if (!fRebootPending && fTestRegPendingRename)
	{
		// We need to see if the PendingFileRenameOperations registry setting is present:
		LONG lResult;
		HKEY hKey = NULL;

		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("System\\CurrentControlSet\\Control\\Session Manager"), NULL,
			KEY_READ, &hKey);

		// We don't proceed unless the call above succeeds:
		if (ERROR_SUCCESS == lResult)
		{
			lResult = RegQueryValueEx(hKey, _T("PendingFileRenameOperations"), NULL, NULL, NULL,
				NULL);
			RegCloseKey(hKey);

			if (ERROR_SUCCESS == lResult)
				fRebootPending = true;
		}
	}
	if (!fRebootPending && fTestWininit)
	{
		// We need to test if the file Wininit.ini is present.
		// Find out where Windows is located:
		_TCHAR * pszWinPath = GetFolderPathNew(CSIDL_WINDOWS);
		if (pszWinPath)
		{
			// Add the file name:
			_TCHAR * pszWininitPath = MakePath(pszWinPath, _T("Wininit.ini"));

			delete[] pszWinPath;
			pszWinPath = NULL;

			// See if the file exists:
			FILE * f;
			if (_tfopen_s(&f, pszWininitPath, _T("r")) == 0)
			{
				fclose(f);
				fRebootPending = true;
			}
			delete[] pszWininitPath;
			pszWininitPath = NULL;
		}
	}

	if (fRebootPending &&  m_ppmProductManager->GetFlushRebootFlag(iProduct))
	{
		g_Log.Write(_T("A reboot is pending and %s requires it to be flushed."),
			m_ppmProductManager->GetName(iProduct));
		g_ProgRecord.WriteRunOnce();
		FriendlyReboot(); // Doesn't return
	}
}

// Installs the prerequisites for the given product.
bool MasterInstaller_t::InstallPrerequisites(int iProduct)
{
	IndexList_t rgiPrerequisites;
	IndexList_t rgiSingleProduct;
	rgiSingleProduct.Add(iProduct);
	g_Log.Write(_T("Testing for prerequisites for %s..."), m_ppmProductManager->GetName(iProduct));
	g_Log.Indent();

	// Produce a flat list of all prerequisites:
	m_ppmProductManager->GetActivePrerequisites(rgiSingleProduct, rgiPrerequisites, true);

	g_Log.Unindent();
	g_Log.Write(_T("...Done (prerequisites)"));

	if (rgiPrerequisites.GetCount() > 0)
	{
		g_Log.Write(_T("Prerequisites for %s:"), m_ppmProductManager->GetName(iProduct));
		g_Log.Indent();
		for (int i = 0; i < rgiPrerequisites.GetCount(); i++)
			g_Log.Write(_T("%d: %s"), i, m_ppmProductManager->GetName(rgiPrerequisites[i]));
		g_Log.Unindent();
	}
	else
	{
		g_Log.Write(_T("No prerequisites for %s."), m_ppmProductManager->GetName(iProduct));
	}

	// Before attempting to install any of these, see if any are listed as failures. If so,
	// there is no point in installing any of them, as the main product requiring them will
	// not be installable:
	for (int i = 0; i < rgiPrerequisites.GetCount(); i++)
	{
		if (m_ppmProductManager->PriorInstallationFailed(rgiPrerequisites[i]))
		{
			HandleError(kNonFatal, false, IDC_ERROR_PREREQUISITE_PRIOR_FAIL,
				m_ppmProductManager->GetName(rgiPrerequisites[i]),
				m_ppmProductManager->GetName(iProduct));

			return false;
		}
	}

	while (rgiPrerequisites.GetCount() > 0)
	{
		// The listed prerequistes may have prerequisites of their own. If so, these
		// second-order prerequisites will also be somewhere in our list, as the list is a
		// flattened tree. So we'll simply iterate repeatedly, installing products that have
		// no prerequisites of their own:
		for (int i = 0; i < rgiPrerequisites.GetCount(); i++)
		{
			CheckIfStopRequested();

			IndexList_t rgiSinglePrerequisite;
			rgiSinglePrerequisite.Add(rgiPrerequisites[i]);
			g_Log.Write(_T("Testing for unfulfilled prerequisites of prerequisite %s..."),
				m_ppmProductManager->GetName(rgiPrerequisites[i]));
			g_Log.Indent();
			if (!m_ppmProductManager->PrerequisitesNeeded(rgiSinglePrerequisite))
			{
				g_Log.Unindent();
				g_Log.Write(_T("...Done - %s has no unfulfilled prerequisites of its own"),
					m_ppmProductManager->GetName(rgiPrerequisites[i]));

				int iPrerequisite = rgiPrerequisites.RemoveNthItem(i);

				// It is remotely possible that an earlier installation means we no longer need
				// The current product, so let's just check:
				IndexList_t rgiTempCurrentPrereqs;

				g_Log.Write(_T("Testing for prerequisites, to see if %s is still needed..."),
					m_ppmProductManager->GetName(iPrerequisite));
				g_Log.Indent();
				m_ppmProductManager->GetActivePrerequisites(rgiSingleProduct,
					rgiTempCurrentPrereqs, true);
				g_Log.Unindent();
				g_Log.Write(_T("...Done (prerequisites)"));

				if (!rgiTempCurrentPrereqs.Contains(iPrerequisite))
				{
					// Product is no longer needed
					g_Log.Write(_T("Product %s is no longer a prerequisite."),
						m_ppmProductManager->GetName(iPrerequisite));
					continue; // Product is already removed from prerequisite list.
				}

				// See if this product requires a pending reboot to be flushed first:
				TestAndPerformPendingReboot(iPrerequisite);

				if (!m_ppmProductManager->InstallProduct(iPrerequisite))
				{
					HandleError(kNonFatal, false, IDC_ERROR_PREREQUISITE_FAIL,
						m_ppmProductManager->GetName(iPrerequisite),
						m_ppmProductManager->GetName(iProduct));
					return false;
				}
			} // End if no second-order prerequisites were found for current prerequisite.
			else
			{
				g_Log.Unindent();
				g_Log.Write(_T("...Done - %s still has outstanding prerequisites."),
					m_ppmProductManager->GetName(rgiPrerequisites[i]));
			}
		} // Next product
	} // Repeat until no prerequisites left.

	return true;
}
