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
	void Write(const char * szText, ...);
	void Terminate();
	bool WriteClipboard();

protected:
	char * m_pszFilePath;
	char * m_pszLog;
	char * m_pszPendingMessages;
};

extern LogFile g_Log;