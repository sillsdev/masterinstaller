#pragma once

#include <windows.h>
#include <stdio.h>

#include "LogFile.h"
#include "ProductManager.h"
#include "HelpLauncher.h"

typedef HANDLE (* HelpFn)(const _TCHAR * pszStem, HANDLE hShutdownEvent);


class MasterInstaller_t : public IHelpLauncher
{
public:
	MasterInstaller_t();
	~MasterInstaller_t();

	void Run();

	// IHelpLauncher implementation:
	virtual void LaunchHelp(const _TCHAR * pszHelpTag);

protected:
	// Mutex used to make sure we're only running one instance:
	static const _TCHAR * m_kszMutexName;
	HANDLE m_hMutex;
	// Flag set to true if OS is less than Windows 2000:
	bool m_fLessThanWin2k;
	// Flag set to true if user has local admin privileges:
	bool m_fAdministrator;

	// Registry sub-key name for product key storage:
	static const _TCHAR * m_kszRegProductKeys;

	// Handle to an event where we can signal that we're shutting down:
	HANDLE m_hShutDown;
	// Storage for threads of any help viewers we launch:
	HANDLE * m_phHelpViewerThreads;
	int m_cHelpViewerThreads;
	// Handle to InstallerHelp2.dll:
	HMODULE m_hmodInstallerHelp2;
	HelpFn Help;

	IProductManager * m_ppmProductManager;

	int m_nPhase;
	enum // Phases
	{
		phaseChoices,
		phasePrerequisites,
		phaseMain,
		phaseDependencies,
		phaseMaxPhases
	};

	// User's choices are stored in the following:
	IndexList_t m_rgiChosenMainProducts;
	bool m_fInstallRequiredProducts;

	// Products that fail to install are recorded here, to avoid repeated attempts:
	IndexList_t m_rgiInstallFailures;

	void ReRun();
	bool CreateMutex();
	bool IsCurrentUserLocalAdministrator();
	void SelectMainProducts();
	void GetProductKey(_TCHAR szKey[100]);
	void TestAndPerformPendingReboot(int iProduct);
	bool InstallPrerequisites(int iProduct);
	bool TestAndReportLanguageConflicts(IndexList_t & rgiProducts,
		bool fIncludeGivenProducts = false);
};
