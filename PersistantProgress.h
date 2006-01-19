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
	void SetCmdLine(const char * pszCmdLine);
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
	const char * m_pszCmdLine;

	bool ReadList(const char * pszRegListName, const char * pszRegCountName, IndexList_t & rgi);
	void WriteList(const char * pszRegListName, const char * pszRegCountName,
		IndexList_t & rgi);

	static const char * m_kpszRegKey;
	static const char * m_kpszRegValueActivity;
	static const char * m_kpszRegValuePhase;
	static const char * m_kpszRegValueStep;
	static const char * m_kpszRegValueMainList;
	static const char * m_kpszRegValueMainListCount;
	static const char * m_kpszRegValueProduct;
	static const char * m_kpszRegValueRunOnce;
};

// Global declaration:
extern PersistantProgress g_ProgRecord;