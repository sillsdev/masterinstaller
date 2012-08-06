#include <windows.h>
#include <tchar.h>

#include "Globals.h"
#include "LogFile.h"
#include "UsefulStuff.h"

// Global instantiations:
DWORD g_langidWindowsLanguage = 0;
bool g_fAdministrator = false;
bool g_fStopRequested = false;
bool g_fRebootPending = false;
bool g_fManualInstall = false;
bool g_fSilent = false;
struct MainSelectionReturn_t * g_pCmdLineProductSelection = NULL;
UserQuitException_t UserQuitException;

#include "ConfigGeneral.cpp"

#include "Resource.h"
#include "ErrorHandler.h"

HANDLE hStopRequestInProgress = NULL;

void CheckIfStopRequested()
{
	if (hStopRequestInProgress)
		WaitForSingleObject(hStopRequestInProgress, INFINITE);

	if (g_fStopRequested)
		HandleError(kUserAbort, true, IDC_ERROR_USER_ABORT);
}

const int knVisibleOnlyOffset = 0xFFFF;

OSVersion_t::OSVersion_t()
{
	// Get details of the version of Windows we're running on:
	m_OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	::GetVersionEx((LPOSVERSIONINFO)(&m_OSversion));

	g_Log.Write(_T("OS version = %d.%d SP %d.%d"), m_OSversion.dwMajorVersion,
		m_OSversion.dwMinorVersion, m_OSversion.wServicePackMajor,
		m_OSversion.wServicePackMinor);

	m_pszVersion = new_sprintf(_T("%d.%d.%d.%d"), m_OSversion.dwMajorVersion,
		m_OSversion.dwMinorVersion, m_OSversion.wServicePackMajor,
		m_OSversion.wServicePackMinor);
}

OSVersion_t::~OSVersion_t()
{
	delete[] m_pszVersion;
	m_pszVersion = NULL;
}

_TCHAR * OSVersion_t::Numeric()
{
	return m_pszVersion;
}

_TCHAR * OSVersion_t::MakeGeneralDescription(const _TCHAR * pszOsNum)
{
	if (!pszOsNum)
		return NULL;

	_TCHAR * pszOsNumCpy = my_strdup(pszOsNum);
	_TCHAR * pszRet = NULL;

	int nMajor = -1;
	int nMinor = -1;
	int nSpMajor = -1;
	int nSpMinor = -1;
	
	_TCHAR *pszContext = NULL;

	_TCHAR * pszToken = _tcstok_s(pszOsNumCpy, _T("."), &pszContext);
	if (pszToken)
	{
		nMajor = _tstoi(pszToken);
		pszToken = _tcstok_s(NULL, _T("."), &pszContext);
		if (pszToken)
		{
			nMinor = _tstoi(pszToken);
			pszToken = _tcstok_s(NULL, _T("."), &pszContext);
			if (pszToken)
			{
				nSpMajor = _tstoi(pszToken);
				pszToken = _tcstok_s(NULL, _T("."), &pszContext);
				if (pszToken)
					nSpMinor = _tstoi(pszToken);
			}
		}
	}
	delete[] pszOsNumCpy;
	pszOsNumCpy = NULL;

	if (nMajor == 6 && nMinor == 1)
		pszRet = new_sprintf(_T("Windows 7"));
	else if (nMajor == 6 && nMinor == 0)
		pszRet = new_sprintf(_T("Windows Vista"));
	else if (nMajor == 5 && nMinor == 1)
		pszRet = new_sprintf(_T("Windows XP"));
	else if (nMajor == 5 && nMinor == 0)
		pszRet = new_sprintf(_T("Windows 2000"));
	else if (nMajor == 4)
		pszRet = new_sprintf(_T("Windows 9x"));
	else if (nMinor >= 0)
		pszRet = new_sprintf(_T("Windows %d.%d"), nMajor, nMinor);
	else
		pszRet = new_sprintf(_T("Windows %d"), nMajor);
	
	if (nSpMajor >= 0 && nSpMajor < 32767)
	{
		if (nSpMinor >= 0 && nSpMinor < 32767)
			new_sprintf_concat(pszRet, 0, _T(" SP %d.%d"), nSpMajor, nSpMinor);
		else
			new_sprintf_concat(pszRet, 0, _T(" SP %d"), nSpMajor);
	}

	return pszRet;
}

// Creates a new string describing user's version of Windows.
// Caller must delete[] the returned value.
_TCHAR * OSVersion_t::MakeDescription()
{
	_TCHAR * pszRet = NULL;

	switch(m_OSversion.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		pszRet = new_sprintf(_T("Windows %d.%d"), m_OSversion.dwMajorVersion,
			m_OSversion.dwMinorVersion);
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if (m_OSversion.dwMinorVersion==0)
			pszRet = my_strdup(_T("Windows 95"));
		else if (m_OSversion.dwMinorVersion == 10)
			pszRet = my_strdup(_T("Windows 98"));
		else if (m_OSversion.dwMinorVersion == 90)  
			pszRet = my_strdup(_T("Windows ME"));
		else
		{
			// Unknown windows/newest windows version:
			pszRet = new_sprintf(_T("Windows %d.%d"), m_OSversion.dwMajorVersion,
				m_OSversion.dwMinorVersion);
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		if (m_OSversion.dwMajorVersion == 6 && m_OSversion.dwMinorVersion == 1)
		{
			if (m_OSversion.wProductType == VER_NT_WORKSTATION)
				pszRet = new_sprintf(_T("Windows 7 %s"), m_OSversion.szCSDVersion);
			else
				pszRet = new_sprintf(_T("Windows Server 2008 R2 %s"), m_OSversion.szCSDVersion);
		}
		else if (m_OSversion.dwMajorVersion == 6 && m_OSversion.dwMinorVersion == 0)
		{
			if (m_OSversion.wProductType == VER_NT_WORKSTATION)
				pszRet = new_sprintf(_T("Windows Vista %s"), m_OSversion.szCSDVersion);
			else
				pszRet = new_sprintf(_T("Windows Server 2008 %s"), m_OSversion.szCSDVersion);
		}
		else if (m_OSversion.dwMajorVersion == 5 && m_OSversion.dwMinorVersion == 1)
			pszRet = new_sprintf(_T("Windows XP %s"), m_OSversion.szCSDVersion);
		else if (m_OSversion.dwMajorVersion == 5 && m_OSversion.dwMinorVersion == 0)
			pszRet = new_sprintf(_T("Windows 2000 %s"), m_OSversion.szCSDVersion);
		else if (m_OSversion.dwMajorVersion <= 4)
		{
			pszRet = new_sprintf(_T("Windows NT %d.%d with %s"), m_OSversion.dwMajorVersion,
			m_OSversion.dwMinorVersion, m_OSversion.szCSDVersion);
		}
		else
		{
			// Unknown windows/newest windows version:
			pszRet = new_sprintf(_T("Windows %d.%d SP %d.%d"), m_OSversion.dwMajorVersion,
				m_OSversion.dwMinorVersion, m_OSversion.wServicePackMajor,
				m_OSversion.wServicePackMinor);
		}
		break;
	default:
		// Unknown windows/newest windows version:
		pszRet = new_sprintf(_T("Windows %d.%d SP %d.%d"), m_OSversion.dwMajorVersion,
			m_OSversion.dwMinorVersion, m_OSversion.wServicePackMajor,
			m_OSversion.wServicePackMinor);
		break;
	}
	return pszRet;
}

bool OSVersion_t::WithinRange(const _TCHAR * pszMin, const _TCHAR * pszMax)
{
	return VersionInRange(m_pszVersion, pszMin, pszMax);
}

bool OSVersion_t::operator == (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return VersionInRange(m_pszVersion, pszVer, pszVer);
}

bool OSVersion_t::operator != (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return !VersionInRange(m_pszVersion, pszVer, pszVer);
}

bool OSVersion_t::operator <= (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return VersionInRange(m_pszVersion, NULL, pszVer);
}

bool OSVersion_t::operator >= (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return VersionInRange(m_pszVersion, pszVer, NULL);
}

bool OSVersion_t::operator < (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return VersionInRangeEx(m_pszVersion, NULL, pszVer);
}

bool OSVersion_t::operator > (const _TCHAR * pszVer)
{
	if (!pszVer)
	{
		HandleError(kNonFatal, false, IDC_ERROR_INTERNAL, kpszOperatorError);
		return false;
	}
	return VersionInRangeEx(m_pszVersion, pszVer, NULL);
}

// Instantiate static mnemonics:
_TCHAR * OSVersion_t::Win2k = _T("5.0");
_TCHAR * OSVersion_t::XP = _T("5.1");
_TCHAR * OSVersion_t::Vista = _T("6.0");
_TCHAR * OSVersion_t::W7 = _T("6.1");
_TCHAR * OSVersion_t::kpszOperatorError = _T("Null value supplied to OSVersion_t comparison operator");


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
			_T("Invalid index passed to product index list."));
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
			_T("attempting to remove invalid-indexed item from list."));
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
			_T("attempting to alter invalid-indexed item in list."));
	}
	m_pi[i] = nNew;
}

#ifdef EASTER_EGGS
#include "midi.cpp"
#endif