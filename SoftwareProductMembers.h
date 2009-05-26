/*
	SoftwareProductMembers.h

	These are the members of the SoftwareProduct Class that are initialized automatically by the
	processed XML file. They are separated out into this file so that if edited, the Report
	system will be kept up to date.
*/
	const _TCHAR * m_kpszInternalName;
	const _TCHAR * m_kpszNiceName;
	const _TCHAR * m_kpszMsiProductCode;
	const _TCHAR * m_kpszMsiFeature;
	const _TCHAR * m_kpszVersionInKey;
	int m_nKeyId;
	bool m_kfOneOfOurs;
	bool * m_fCriticalFileFlag;
	const _TCHAR * m_pszCriticalFileFlagTrue;
	const _TCHAR * m_pszCriticalFileFlagFalse;
	int m_iCd;
	bool m_fMustHaveWin2kOrBetter;
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
