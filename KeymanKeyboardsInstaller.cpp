#include <tchar.h>

DWORD KeymanKeyboardsInstaller(bool /*fFlag*/, const TCHAR * pszCriticalFile)
{
	return KeymanFunction(false, pszCriticalFile);
}
