/*
	Copyright (c) 2003 SIL International.

	Master installer file for writing current activity to the registry, and reading it back on
	restart.
*/

#include <windows.h>
#include <tchar.h>

#include "PersistantProgress.h"
#include "resource.h"
#include "ErrorHandler.h"
#include "LogFile.h"
#include "UsefulStuff.h"


// Global instantiation:
PersistantProgress g_ProgRecord;


const _TCHAR * PersistantProgress::m_kpszRegKey = _T("SOFTWARE\\SIL\\Installer\\MasterInstaller");
const _TCHAR * PersistantProgress::m_kpszRegValuePhase = _T("CurrentPhase");
const _TCHAR * PersistantProgress::m_kpszRegValueMainList = _T("MainSelections");
const _TCHAR * PersistantProgress::m_kpszRegValueMainListCount = _T("NumMainSelections");
const _TCHAR * PersistantProgress::m_kpszRegValueProduct = _T("Product_");
const _TCHAR * PersistantProgress::m_kpszRegValueRunOnce = _T("SIL Software Installation");

PersistantProgress::PersistantProgress()
{
	m_hKey = NULL;
	m_pszCmdLine = _T("");
}

PersistantProgress::~PersistantProgress()
{
	Close();
}

LONG PersistantProgress::OpenForRead()
{
	return RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_kpszRegKey, 0, KEY_READ, &m_hKey);
}

void PersistantProgress::OpenForWrite()
{
	LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, m_kpszRegKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, NULL);

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);
}

void PersistantProgress::Close()
{
	if (m_hKey)
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}

bool PersistantProgress::RecordExists()
{
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_kpszRegKey, 0, KEY_READ, &m_hKey);
	
	return (ERROR_SUCCESS == lResult);
}

void PersistantProgress::SetCmdLine(const _TCHAR * pszCmdLine)
{
	m_pszCmdLine = pszCmdLine;
}

void PersistantProgress::WriteRunOnce()
{
	HKEY hKey;

	g_Log.Write(_T("Writing RunOnce registry data."));

	LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);

	const int knLen = MAX_PATH + 20;
	_TCHAR szPath[knLen];
	GetModuleFileName(NULL, szPath, knLen);
	_TCHAR * pszRunOnce = new_sprintf(_T("\"%s\" %s"), szPath, m_pszCmdLine);
	lResult = RegSetValueEx(hKey, m_kpszRegValueRunOnce, 0, REG_SZ, (LPBYTE)pszRunOnce,
		(DWORD)_tcslen(pszRunOnce));
	delete[] pszRunOnce;
	pszRunOnce = NULL;

	RegCloseKey(hKey);

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);
}

int PersistantProgress::ReadPhase()
{
	int nPhase;
	DWORD cbData = sizeof(nPhase);

	LONG lResult = OpenForRead();
	if (ERROR_SUCCESS == lResult)
	{
		lResult = RegQueryValueEx(m_hKey, m_kpszRegValuePhase, NULL, NULL, (LPBYTE)&nPhase,
			&cbData);
		Close();
	}

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_READ_REG);

	return nPhase;
}

void PersistantProgress::WritePhase(int nPhase)
{
	OpenForWrite();
	LONG lResult = RegSetValueEx(m_hKey, m_kpszRegValuePhase, 0, REG_DWORD, (LPBYTE)&nPhase,
		sizeof(nPhase));
	Close();

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);
}

bool PersistantProgress::ReadList(const _TCHAR * pszRegListName, const _TCHAR * pszRegCountName,
								  IndexList_t & rgi)
{
	rgi.Flush();

	DWORD cbData = sizeof(rgi.m_ct);

	LONG lResult = OpenForRead();
	if (ERROR_SUCCESS == lResult)
	{
		lResult = RegQueryValueEx(m_hKey, pszRegCountName, NULL, NULL, (LPBYTE)&rgi.m_ct,
			&cbData);
		Close();
	}

	if (ERROR_SUCCESS != lResult)
		return false;

	rgi.m_pi = new int [rgi.m_ct];
	cbData = rgi.m_ct * sizeof(int);

	lResult = OpenForRead();
	if (ERROR_SUCCESS == lResult)
	{
		lResult = RegQueryValueEx(m_hKey, pszRegListName, NULL, NULL, (LPBYTE)rgi.m_pi,
			&cbData);
		Close();
	}

	if (ERROR_SUCCESS != lResult)
	{
		rgi.Flush();
		return false;
	}
	return true;
}

void PersistantProgress::WriteList(const _TCHAR * pszRegListName, const _TCHAR * pszRegCountName,
								   IndexList_t & rgi)
{
	OpenForWrite();
	LONG lResult = RegSetValueEx(m_hKey, pszRegListName, 0, REG_BINARY, (LPBYTE)rgi.m_pi,
		rgi.m_ct * sizeof(int));
	Close();

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);

	// Save count:
	OpenForWrite();
	lResult = RegSetValueEx(m_hKey, pszRegCountName, 0, REG_DWORD, (LPBYTE)&rgi.m_ct,
		sizeof(rgi.m_ct));
	Close();

	if (ERROR_SUCCESS != lResult)
	{
		RegCloseKey(m_hKey);
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);
	}
}

void PersistantProgress::ReadMainSelectionList(IndexList_t & rgi)
{
	if (!ReadList(m_kpszRegValueMainList, m_kpszRegValueMainListCount, rgi))
		HandleError(kFatal, true, IDC_ERROR_READ_REG);
}

void PersistantProgress::WriteMainSelectionList(IndexList_t & rgi)
{
	WriteList(m_kpszRegValueMainList, m_kpszRegValueMainListCount, rgi);
}

int PersistantProgress::ReadInstallStatus(int iProduct)
{
	_TCHAR * pszRegValue = new_sprintf(_T("%s%d"), m_kpszRegValueProduct, iProduct);

	int nStatus = 0;
	DWORD cbData = sizeof(nStatus);

	LONG lResult = OpenForRead();
	if (ERROR_SUCCESS == lResult)
	{
		lResult = RegQueryValueEx(m_hKey, pszRegValue, NULL, NULL, (LPBYTE)&nStatus, &cbData);
		Close();
	}
	// Don't bother with handling an error - a return value of zero will do.
	delete[] pszRegValue;
	return nStatus;
}

void PersistantProgress::WriteInstallStatus(int iProduct, int nStatus)
{
	_TCHAR * pszRegValue = new_sprintf(_T("%s%d"), m_kpszRegValueProduct, iProduct);

	OpenForWrite();
	LONG lResult = RegSetValueEx(m_hKey, pszRegValue, 0, REG_DWORD, (LPBYTE)&nStatus,
		sizeof(nStatus));
	Close();

	if (ERROR_SUCCESS != lResult)
		HandleError(kFatal, true, IDC_ERROR_WRITE_REG);

	delete[] pszRegValue;
}

void PersistantProgress::RemoveData(bool fReportError)
{
	LONG lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, m_kpszRegKey);

	if (ERROR_SUCCESS != lResult && fReportError)
	{
		RegCloseKey(m_hKey);
		HandleError(kNonFatal, false, IDC_ERROR_REMOVE_REG);
	}

	HKEY hKey;
	lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lResult)
		HandleError(kNonFatal, false, IDC_ERROR_REMOVE_REG);

	::RegDeleteValue(hKey, m_kpszRegValueRunOnce);
	RegCloseKey(hKey);
}
