/*
	Copyright (c) 2003 SIL International.

	Master installer logging file.
*/

#include <windows.h>
#include <stdio.h>
#include "LogFile.h"
#include "UsefulStuff.h"
#include "dialogs.h"
#include "ErrorHandler.h"
#include "Resource.h"


/*----------------------------------------------------------------------------------------------
	Constructor.
----------------------------------------------------------------------------------------------*/
LogFile::LogFile()
{
	m_pszFilePath = NULL;
	m_pszLog = NULL;
	m_pszPendingMessages = NULL; 
}

/*----------------------------------------------------------------------------------------------
	Destructor.
----------------------------------------------------------------------------------------------*/
LogFile::~LogFile()
{
	Terminate();
	delete[] m_pszLog;
	delete[] m_pszFilePath;
	delete[] m_pszPendingMessages;
}

/*----------------------------------------------------------------------------------------------
	Signals that all log messages must be flushed to a file as soon as they are sent.
----------------------------------------------------------------------------------------------*/
void LogFile::SetActiveWriting(const char * pszFilePath)
{
	if (!pszFilePath)
		pszFilePath = "C:\\SIL Installer.log";
	if (pszFilePath[0] != 0)
		m_pszFilePath = strdup(pszFilePath);

	// Check that the log file is writable:
	if (m_pszFilePath)
	{
		FILE * file = fopen(m_pszFilePath, "a");
		if (file)
			fclose(file);
		else
			HandleError(kNonFatal, false, IDC_ERROR_LOG_FILE_INVALID, m_pszFilePath);
	}
}

/*----------------------------------------------------------------------------------------------
	Writes initial text to log file, based on how installation was started.
----------------------------------------------------------------------------------------------*/
void LogFile::Start()
{
	Write("Installation started.");
}

/*----------------------------------------------------------------------------------------------
	Writes given text to log file.
	@param szText Text to be written
----------------------------------------------------------------------------------------------*/
void LogFile::Write(const char * szText, ...)
{
	// Always start with a time-stamp:
	SYSTEMTIME syst;
	GetLocalTime(&syst);

	char * pszTotalMsg = new_sprintf("%04d-%02d-%02d %02d:%02d:%02d - ", syst.wYear,
		syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);

	// Collect variable arguments:
	va_list arglist;
	va_start(arglist, szText);

	// Format text with variable arguments:
	new_vsprintf_concat(pszTotalMsg, 0, szText, arglist);
	new_sprintf_concat(pszTotalMsg, 1, ""); // Stick a newline on the end.

	if (m_pszFilePath)
	{
		FILE * file = fopen(m_pszFilePath, "a");
		if (file)
		{
			if (m_pszPendingMessages)
			{
				// We collected some messages before the message file was open:
				fputs(m_pszPendingMessages, file);
				delete[] m_pszPendingMessages;
				m_pszPendingMessages = NULL;
			}
			fputs(pszTotalMsg, file);
			fclose(file);
		}
	}
	else
	{
		// Collect in pending messages:
		new_sprintf_concat(m_pszPendingMessages, 0, pszTotalMsg);
	}
	new_sprintf_concat(m_pszLog, 0, pszTotalMsg);

	delete[] pszTotalMsg;
	pszTotalMsg = NULL;
}

/*----------------------------------------------------------------------------------------------
	Copies the contents of the log to the clipboard.
----------------------------------------------------------------------------------------------*/
bool LogFile::WriteClipboard()
{
	return WriteClipboardText(m_pszLog);
}


/*----------------------------------------------------------------------------------------------
	Closes down the log file, in case we wish to restart before object goes out of scope.
----------------------------------------------------------------------------------------------*/
void LogFile::Terminate()
{
	Write("Installation ended.\n");
	delete[] m_pszFilePath;
	m_pszFilePath = NULL;
}
