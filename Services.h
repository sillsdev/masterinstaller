#pragma once

#include <tchar.h>
#include <windows.h>

class ServiceManager_t
{
public:
	ServiceManager_t(void);

	bool StartService(_TCHAR * pszServiceName);
	bool StopService(_TCHAR * pszServiceName, bool fStopDependencies, DWORD dwTimeout);
	bool RestartService(_TCHAR * pszServiceName, bool fStopDependencies, DWORD dwStopTimeout);

protected:
	SC_HANDLE schSCManager;
};
