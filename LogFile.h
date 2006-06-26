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

	void SetActiveWriting(const char * pszFilePath);
	void Start();
	void Indent();
	void Unindent();
	void Write(const char * szText, ...);
	void Terminate();
	bool WriteClipboard();

protected:
	char * m_pszFilePath;
	char * m_pszLog;
	char * m_pszPendingMessages;
	int m_cIndent;
};

extern LogFile g_Log;