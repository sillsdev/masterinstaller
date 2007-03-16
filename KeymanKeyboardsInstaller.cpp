#include <tchar.h>

DWORD KeymanKeyboardsInstaller(bool /*fFlag*/, const TCHAR * pszCriticalFile)
{
	DWORD retVal = KeymanFunction(false, pszCriticalFile);
	if (retVal == 0)
		return ERROR_SUCCESS_REBOOT_REQUIRED; // Keyboards are not effective till after a reboot.
	return retVal;
}
