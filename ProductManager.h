#pragma once

#include "Globals.h"

class ProductKeyHandler_t;

// Main interface for accessing the product details in the installation:
class IProductManager
{
public:
	virtual bool GetFlushRebootFlag(int iProduct) const = 0;
	virtual bool GetRebootTestRegPendingFlag(int iProduct) const = 0;
	virtual bool GetRebootWininitFlag(int iProduct) const = 0;
	virtual bool PossibleToTestPresence(int iProduct) const = 0;
	virtual bool TestPresence(int iProduct, const _TCHAR * pszMinVersion = NULL,
		const _TCHAR * pszMaxVersion = NULL) = 0;
	virtual bool IsMsiUpgradePermitted(int iProduct) const = 0;
	virtual int CompareMsiVersionWithInstalled(int iProduct) const = 0;
	virtual const _TCHAR * GetName(int iProduct) const = 0;
	virtual const _TCHAR * GetCommentary(int iProduct) const = 0;
	virtual const _TCHAR * GetStatusWindowControl(int iProduct) const = 0;
	virtual const _TCHAR * GetCriticalFile(int iProduct) const = 0;
	virtual bool CriticalFileLanguageUnavailable(int iProduct) const = 0;
	virtual const _TCHAR * GetDownloadUrl(int iProduct) const = 0;
	virtual int GetCdIndex(int iProduct) const = 0;
	virtual const _TCHAR * GetHelpTag(int iProduct) const = 0;
	virtual bool GetHelpTagInternalFlag(int iProduct) const = 0;
	virtual const _TCHAR * GetTestPresenceVersion(int iProduct) const = 0;
	virtual bool IsOsHighEnough(int iProduct) const = 0;
	virtual bool IsOsLowEnough(int iProduct) const = 0;
	virtual const _TCHAR * GetMinOsRequirement(int iProduct) const = 0;
	virtual const _TCHAR * GetMaxOsRequirement(int iProduct) const = 0;
	virtual bool GetMustBeAdminFlag(int iProduct) const = 0;
	virtual int GetNumProtectedMainProducts() const = 0;
	virtual void DetermineAvailableMainProducts(ProductKeyHandler_t & ProductKeyHandler,
		const _TCHAR * pszKey) = 0;
	virtual void GenAvailableMainProductList(IndexList_t & rgiProducts,
		bool fIncludeVisibles) const = 0;
	virtual int GetNumPermittedMainProducts() const = 0;
	virtual bool KeyUnlockedNothing() const = 0;
	virtual void AutoSelectAllPermittedMainProducts(IndexList_t & rgiProducts) const = 0;
	virtual bool PrerequisitesNeeded(const IndexList_t & rgiSelectedProducts) const = 0;
	virtual bool RequirementsNeeded() const = 0;
	virtual void GetActivePrerequisites(const IndexList_t & prgiProducts,
		IndexList_t & rgiOutputList, bool fRecurse) const = 0;
	virtual void GetActiveRequirements(IndexList_t & rgiOutputList) const = 0;
	virtual void GetActiveRequirements(const IndexList_t & rgiProducts,
		IndexList_t & rgiOutputList, bool fRecursePrerequisites, bool fRecurseRequirements)
		const = 0;
	virtual bool PriorInstallationFailed(int iProduct) const = 0;
	virtual bool IsContainer(int iProduct) const = 0;
	virtual bool IsInstallable(int iProduct) const = 0;
	virtual bool InstallProduct(int iProduct) = 0;
	virtual _TCHAR * GenReport(int iReportType, IndexList_t * prgiProducts = NULL) const = 0;
	virtual void ShowReport(const _TCHAR * pszTitle, const _TCHAR * pszIntro,
		const _TCHAR * pszOkButtonText, bool fConfirmQuit, bool fQuitIsError, int nType,
		bool fCanToggleType, IndexList_t * rgiProducts = NULL) const = 0;
	virtual bool ShowFinalReport() const = 0;

	enum // Dependency types
	{
		depPrerequisite,
		depRequirement,
		depTotal
	};
	enum // Report types
	{
		rptPrerequisitesShort,
		rptPrerequisitesFull,
		rptRequirementsShort,
		rptRequirementsFull,
		rptFinal,
	};
};

extern IProductManager * CreateProductManager();
extern void DestroyProductManager(IProductManager *& ppmProductManager);
