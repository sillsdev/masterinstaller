#include <windows.h>

// A little utility (probably also of little value) to remove the last product key
// stored in the user's registry. This is for when an installer relies on the last key used
// without asking the user for a new one, when the last key is not valid for that product.

int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/,
					 int /*nCmdShow*/)
{
	HKEY hKey = NULL;
	LONG lResult;

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SIL\\Installer\\ProductKeys", NULL,
		KEY_WRITE, &hKey);

	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegDeleteValue(hKey, "Last");
		RegCloseKey(hKey);
		hKey = NULL;

		if (lResult == ERROR_SUCCESS)
			MessageBox(NULL, "'Last' product key cleared. Try installation again.", "", 0);
		else
			MessageBox(NULL, "Error: there is no 'Last' product key.", "", 0);
		return 0;
	}
	MessageBox(NULL, "Error: cannot clear 'Last' product key.", "", 0);
	return 0;
}