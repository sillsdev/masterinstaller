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
	m_cIndent = 0;
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
		m_pszFilePath = my_strdup(pszFilePath);

	// Check that the log file is writable:
	if (m_pszFilePath)
	{
		FILE * file;
		if (fopen_s(&file, m_pszFilePath, "a") == 0)
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
	Adds a new level of indentation.
----------------------------------------------------------------------------------------------*/
void LogFile::Indent()
{
	m_cIndent += 1;
}

/*----------------------------------------------------------------------------------------------
	Removes a level of indentation.
----------------------------------------------------------------------------------------------*/
void LogFile::Unindent()
{
	m_cIndent -= 1;
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

	char * pszTotalMsg = NULL;

	// Add date/time stamp:
	new_sprintf_concat(pszTotalMsg, 0, "%04d-%02d-%02d %02d:%02d:%02d - ", syst.wYear,
		syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);

	// Add current level of indentation:
	for (int i = 0; i < m_cIndent; i++)
		new_sprintf_concat(pszTotalMsg, 0, "  ");

	// Collect variable arguments:
	va_list arglist;
	va_start(arglist, szText);

	// Format text with variable arguments:
	new_vsprintf_concat(pszTotalMsg, 0, szText, arglist);
	new_sprintf_concat(pszTotalMsg, 1, ""); // Stick a newline on the end.

	if (m_pszFilePath)
	{
		FILE * file;
		if (fopen_s(&file, m_pszFilePath, "a") == 0)
		{
			if (m_pszPendingMessages)
			{
				// We collected some messages before the message file was open:
				fputs(m_pszPendingMessages, file);
				delete[] m_pszPendingMessages;
				m_pszPendingMessages = NULL;
			}
			// Check for newlines in the new message, and insert indentation:
			int ich = 0;
			while (ich < (int)strlen(pszTotalMsg))
			{
				if (pszTotalMsg[ich] == '\r' || pszTotalMsg[ich] == '\n')
				{
					// Find end of sequence of \r and \n characters:
					do
					{
						ich++;
					} while (pszTotalMsg[ich] == '\r' || pszTotalMsg[ich] == '\n');
					// Put temporary terminator at current location:
					char chOrig = pszTotalMsg[ich];
					if (chOrig != 0)
					{
						pszTotalMsg[ich] = 0;

						// Start to build replacement string:
						char * pszNew = my_strdup(pszTotalMsg);
						pszTotalMsg[ich] = chOrig;

						// Insert current level of indentation, including indenting past time stamp:
						int NumSpaces = 22 + 2 * m_cIndent;
						for (int isp = 0; isp < NumSpaces; isp++)
							new_sprintf_concat(pszNew, 0, " ");

						// Add in rest of original string:
						new_sprintf_concat(pszNew, 0, &pszTotalMsg[ich]);

						delete[] pszTotalMsg;
						pszTotalMsg = pszNew;
						pszNew = NULL;
					}
				}
				ich++;
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
