#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <tchar.h>

// This program writes out valid C++ files, each of which defines an array of bytes representing
// one of the master installer's help files.
// You can either run it with command line arguments (input help file path and output C++ file
// path), or you can uncomment out some of the examples in the main() function at the bottom
// of this file.

void CrunchFile(const _TCHAR * pszInputFilePath, const _TCHAR * pszOutputFilePath)
{
    // Check existence of input file:
	FILE * fInput;
	if (fopen_s(&fInput, pszInputFilePath, "rb") != 0)
	{
		printf("ERROR: Cannot open input file \"%s\"\n", pszInputFilePath);
		return;
	}
	
    // Create output file:
	FILE * fOutput;
	if (fopen_s(&fOutput, pszOutputFilePath, "wt") != 0)
	{
		printf("ERROR: Cannot open output file \"%s\"\n", pszOutputFilePath);
		return;
	}

	// Get input file name without path:
	_TCHAR rgchVarName[MAX_PATH];
	_TCHAR * pszDupInputFilePath = _tcsdup(pszInputFilePath);
	_TCHAR * pszExtn = NULL;
	_TCHAR * ch = _tcsrchr(pszDupInputFilePath, '.');
	if (ch)
	{
		*ch = 0;
		pszExtn = ++ch;
	}
	ch = _tcsrchr(pszDupInputFilePath, '\\');
	if (ch)
		sprintf_s(rgchVarName, MAX_PATH, "rgb%s", ++ch);
	else
		sprintf_s(rgchVarName, MAX_PATH, "rgbFile");

	// Replace any spaces with underscores:
	int cch = (int)_tcslen(rgchVarName);
	for (int i = 0; i < cch; i++)
		if (rgchVarName[i] == ' ')
			rgchVarName[i] = '_';

	fprintf(fOutput, "BYTE %s[] =\n{", rgchVarName);

	delete[] pszDupInputFilePath;
	pszDupInputFilePath = NULL;

	int nByteCount = 0;
	while (!feof(fInput))
	{
		int ch = getc(fInput);
		if (!feof(fInput))
		{
			if (nByteCount % 8 == 0)
				fputs("\n\t", fOutput);

			// Apply minimal encryption, just so anyone looking at the file contents won't see
			// any words that arouse suspicion:
			ch ^= 0xAA;

			const int kcch = 10;
			_TCHAR psz[kcch];
			if (ch < 16)
				sprintf_s(psz, kcch, "0x0%x, ", ch);
			else
				sprintf_s(psz, kcch, "0x%x, ", ch);
			fputs(psz, fOutput);
			nByteCount++;
		}
	}
	fputs("\n};\n", fOutput);

	fclose(fOutput);
	fclose(fInput);
}


int main(int argc, _TCHAR * argv[])
{
	if (argc < 3)
	{
		MessageBox(NULL, "Usage: Bin2Cpp <input-file> <output-file>", "Usage error", 0);
		return 0;
	}
	CrunchFile(argv[1], argv[2]);

/*	CrunchFile("C:\\Work\\Master Installer\\SetupFW.hlp",
		"C:\\Work\\Master Installer\\InstallerHelp2\\SetupFW_hlp.cpp");

	CrunchFile("C:\\Work\\Master Installer\\SetupST.hlp",
		"C:\\Work\\Master Installer\\InstallerHelp2\\SetupST_hlp.cpp");

	CrunchFile("C:\\Work\\Master Installer\\SetupWS.hlp",
		"C:\\Work\\Master Installer\\InstallerHelp2\\SetupWS_hlp.cpp");

	CrunchFile("C:\\Work\\Master Installer\\SetupBART.hlp",
		"C:\\Work\\Master Installer\\InstallerHelp2\\SetupBART_hlp.cpp");

	CrunchFile("C:\\Work\\Master Installer\\SetupTW.hlp",
		"C:\\Work\\Master Installer\\InstallerHelp2\\SetupTW_hlp.cpp");
*/
	return 0;
}
