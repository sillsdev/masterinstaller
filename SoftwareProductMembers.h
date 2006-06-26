/*
	SoftwareProductMembers.h

	These are the members of the SoftwareProduct Class that are initialized automatically by the
	processed XML file. They are separated out into this file so that if edited, the Report
	system will be kept up to date.
*/
	const char * m_kpszInternalName;
	const char * m_kpszNiceName;
	const char * m_kpszMsiProductCode;
	const char * m_kpszMsiFeature;
	const char * m_kpszVersionInKey;
	int m_nKeyId;
	bool m_kfOneOfOurs;
	bool * m_fCriticalFileFlag;
	const char * m_pszCriticalFileFlagTrue;
	const char * m_pszCriticalFileFlagFalse;
	int m_iCd;
	bool m_fMustHaveWin2kOrBetter;
	bool m_fMustBeAdmin;

	bool m_fFlushPendingReboot;
	bool m_fRebootTestRegPending;
	bool m_fRebootWininit;

	bool m_fMustKillHangingWindows;

	pfnPreInstallation m_pfnPreInstallation;
	const char * m_kpszPreInstallation;
	bool m_fIngnorePreInstallationErrors;

	bool * m_pfInstallerFlag;
	const char * m_kpszInstallerFlagTrue;
	const char * m_kpszInstallerFlagFalse;
	const char * m_kpszMsiFlags;
	const char * m_kpszMsiVersion;
	pfnInstall m_pfnInstall;

	bool m_fMustNotDelayReboot;

	const char * m_kpszDownloadUrl;

	const char * m_kpszCommentary;
	const char * m_kpszStatusWindowControl;
	bool m_fStatusPauseWin98;

	pfnAuxTestPresence m_pfnAuxTestPresence;
	const char * m_kpszAuxTestPresence;
	const char * m_kpszTestPresenceVersion;

	pfnPostInstallation m_pfnPostInstallation;
	const char * m_kpszPostInstallation;

	const char * m_kpszHelpTag;
	bool m_fInternalHelpFile;
