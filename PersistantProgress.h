/*
	Copyright (c) 2003 SIL International.

	Master installer header file for writing current activity to the registry, and reading it
	back on restart.
*/

#pragma once

#include "ProductManager.h"

class PersistantProgress
{
public:
	PersistantProgress();
	~PersistantProgress();

	bool RecordExists();
	void SetCmdLine(const _TCHAR * pszCmdLine);
	void WriteRunOnce();
	int ReadPhase();
	void WritePhase(int nPhase);
	void ReadMainSelectionList(IndexList_t & rgi);
	void WriteMainSelectionList(IndexList_t & rgi);
	int ReadInstallStatus(int iProduct);
	void WriteInstallStatus(int iProduct, int nStatus);
	void RemoveData(bool fReportError = true);

protected:
	LONG OpenForRead();
	void OpenForWrite();
	void Close();

	HKEY m_hKey;
	const _TCHAR * m_pszCmdLine;

	bool ReadList(const _TCHAR * pszRegListName, const _TCHAR * pszRegCountName, IndexList_t & rgi);
	void WriteList(const _TCHAR * pszRegListName, const _TCHAR * pszRegCountName,
		IndexList_t & rgi);

	static const _TCHAR * m_kpszRegKey;
	static const _TCHAR * m_kpszRegValueActivity;
	static const _TCHAR * m_kpszRegValuePhase;
	static const _TCHAR * m_kpszRegValueStep;
	static const _TCHAR * m_kpszRegValueMainList;
	static const _TCHAR * m_kpszRegValueMainListCount;
	static const _TCHAR * m_kpszRegValueProduct;
	static const _TCHAR * m_kpszRegValueRunOnce;
};

// Global declaration:
extern PersistantProgress g_ProgRecord;