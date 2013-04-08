/*
	Copyright (c) 2003 SIL International.

	Master installer logging file.
*/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "LogFile.h"
#include "UsefulStuff.h"
#include "dialogs.h"
#include "ErrorHandler.h"
#include "Resource.h"
#include "globals.h"

/*----------------------------------------------------------------------------------------------
	Constructor.
----------------------------------------------------------------------------------------------*/
LogFile::LogFile()
{
	m_fLocked = false;
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
void LogFile::SetActiveWriting(const _TCHAR * pszFilePath)
{
	bool fUsingTempFolder = false;

	if (!pszFilePath)
	{
		m_pszFilePath = NewTempFolderLogPath();
		fUsingTempFolder = true;
	}
	else if (pszFilePath[0] != 0)
		m_pszFilePath = my_strdup(pszFilePath);

	// Check that the log file is writable:
	if (m_pszFilePath)
	{
		FILE * file;
#ifdef UNICODE
		if (_tfopen_s(&file, m_pszFilePath, _T("a, ccs=UNICODE")) == 0)
#else
		if (_tfopen_s(&file, m_pszFilePath, _T("a")) == 0)
#endif
		{
			// We can open the file:
			fclose(file);
		}
		else if (fUsingTempFolder)
		{
			delete[] m_pszFilePath;
			m_pszFilePath = NULL;
		}
		else // Can't open file, and not using Temp folder:
			HandleError(kNonFatal, false, IDC_ERROR_LOG_FILE_INVALID, m_pszFilePath);
	}
}

/*----------------------------------------------------------------------------------------------
	Writes initial text to log file, based on how installation was started.
----------------------------------------------------------------------------------------------*/
void LogFile::Start()
{
	Write(_T("Installation started."));
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
void LogFile::Write(const _TCHAR * szText, ...)
{
	if (m_fLocked)
		return;

	// Always start with a time-stamp:
	SYSTEMTIME syst;
	GetLocalTime(&syst);

	_TCHAR * pszTotalMsg = NULL;

	// Add date/time stamp:
	new_sprintf_concat(pszTotalMsg, 0, _T("%04d-%02d-%02d %02d:%02d:%02d - "), syst.wYear,
		syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond);

	// Add current level of indentation:
	for (int i = 0; i < m_cIndent; i++)
		new_sprintf_concat(pszTotalMsg, 0, _T("  "));

	// Collect variable arguments:
	va_list arglist;
	va_start(arglist, szText);

	// Format text with variable arguments:
	new_vsprintf_concat(pszTotalMsg, 0, szText, arglist);
	new_sprintf_concat(pszTotalMsg, 1, _T("")); // Stick a newline on the end.

	if (m_pszFilePath)
	{
		FILE * file;
#ifdef UNICODE
		if (_tfopen_s(&file, m_pszFilePath, _T("a, ccs=UNICODE")) == 0)
#else
		if (_tfopen_s(&file, m_pszFilePath, _T("a")) == 0)
#endif
		{
			if (m_pszPendingMessages)
			{
				// We collected some messages before the message file was open:
				_fputts(m_pszPendingMessages, file);
				delete[] m_pszPendingMessages;
				m_pszPendingMessages = NULL;
			}
			// Check for newlines in the new message, and insert indentation:
			int ich = 0;
			while (ich < (int)_tcslen(pszTotalMsg))
			{
				if (pszTotalMsg[ich] == '\r' || pszTotalMsg[ich] == '\n')
				{
					// Find end of sequence of \r and \n _TCHARacters:
					do
					{
						ich++;
					} while (pszTotalMsg[ich] == '\r' || pszTotalMsg[ich] == '\n');
					// Put temporary terminator at current location:
					_TCHAR chOrig = pszTotalMsg[ich];
					if (chOrig != 0)
					{
						pszTotalMsg[ich] = 0;

						// Start to build replacement string:
						_TCHAR * pszNew = my_strdup(pszTotalMsg);
						pszTotalMsg[ich] = chOrig;

						// Insert current level of indentation, including indenting past time stamp:
						int NumSpaces = 22 + 2 * m_cIndent;
						for (int isp = 0; isp < NumSpaces; isp++)
							new_sprintf_concat(pszNew, 0, _T(" "));

						// Add in rest of original string:
						new_sprintf_concat(pszNew, 0, &pszTotalMsg[ich]);

						delete[] pszTotalMsg;
						pszTotalMsg = pszNew;
						pszNew = NULL;
					}
				}
				ich++;
			}

			_fputts(pszTotalMsg, file);
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
	if (m_fLocked)
		return false;
	m_fLocked = true;
	bool fRet = WriteClipboardText(m_pszLog);
	m_fLocked = false;
	return m_fLocked;
}


/*----------------------------------------------------------------------------------------------
	Closes down the log file, in case we wish to restart before object goes out of scope.
----------------------------------------------------------------------------------------------*/
void LogFile::Terminate()
{
	Write(_T("Installation ended.\n"));
	delete[] m_pszFilePath;
	m_pszFilePath = NULL;
}

/*----------------------------------------------------------------------------------------------
	Creates a path for the log file in the TEMP folder.
----------------------------------------------------------------------------------------------*/
_TCHAR * LogFile::NewTempFolderLogPath()
{
	// Get Temp folder:
	int cchTempFolder = 1 + GetTempPath(0, NULL);
	_TCHAR * pszTempFolder = new _TCHAR [cchTempFolder];
	GetTempPath(cchTempFolder, pszTempFolder);

	// Form file name root:
	_TCHAR * pszLogFileRoot = MakePath(pszTempFolder, g_pszTitle);
	delete[] pszTempFolder;
	pszTempFolder = NULL;

	// Append Title, current date and time:
	SYSTEMTIME syst;
	GetLocalTime(&syst);

	_TCHAR * pszLogPath = new_sprintf(_T("%s %04d-%02d-%02d %02d-%02d-%02d.log"),
		pszLogFileRoot, syst.wYear, syst.wMonth, syst.wDay, syst.wHour,
		syst.wMinute, syst.wSecond);

	delete[] pszLogFileRoot;
	pszLogFileRoot = NULL;

	return pszLogPath;
}

/*----------------------------------------------------------------------------------------------
	Returns path to log file.
----------------------------------------------------------------------------------------------*/
_TCHAR * LogFile::GetFilePath()
{
	if (!m_pszFilePath)
		return _T("[unknown]");

	return m_pszFilePath;
}
