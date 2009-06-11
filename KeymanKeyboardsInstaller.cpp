#include <tchar.h>

DWORD KeymanKeyboardsInstaller(bool /*fFlag*/, SoftwareProduct * Product)
{
	DWORD retVal = KeymanFunction(false, Product);
	if (retVal == 0)
		return ERROR_SUCCESS_REBOOT_REQUIRED; // Keyboards are not effective till after a reboot.
	return retVal;
}
