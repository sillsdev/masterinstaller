/*
	Copyright (c) 2003 SIL International.

	Master installer Log-file header file.
*/

#pragma once

class LogFile
{
public:
	LogFile();
	~LogFile();

	void SetActiveWriting(const _TCHAR * pszFilePath);
	void Start();
	void Indent();
	void Unindent();
	void Write(const _TCHAR * szText, ...);
	void Terminate();
	bool WriteClipboard();

protected:
	_TCHAR * m_pszFilePath;
	_TCHAR * m_pszLog;
	_TCHAR * m_pszPendingMessages;
	int m_cIndent;
};

extern LogFile g_Log;