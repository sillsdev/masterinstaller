#include <windows.h>
#include <stdio.h>

// This program writes text files, each of which  which defines the byte content of one of the
// master installer's help files, in valid C syntax.

void CrunchFile(const char * pszInputFilePath, const char * pszOutputFilePath)
{
    // Check existence of input file:
	FILE * fInput = fopen(pszInputFilePath, "rb");
	if (!fInput)
	{
		printf("ERROR: Cannot open input file \"%s\"\n", pszInputFilePath);
		return;
	}
	
    // Create output file:
	FILE * fOutput = fopen(pszOutputFilePath, "wt");
	if (!fOutput)
	{
		printf("ERROR: Cannot open output file \"%s\"\n", pszOutputFilePath);
		return;
	}

	// Get input file name without path:
	char rgchVarName[MAX_PATH];
	char * pszDupInputFilePath = strdup(pszInputFilePath);
	char * pszExtn = NULL;
	char * ch = strrchr(pszDupInputFilePath, '.');
	if (ch)
	{
		*ch = 0;
		pszExtn = ++ch;
	}
	ch = strrchr(pszDupInputFilePath, '\\');
	if (ch)
		sprintf(rgchVarName, "rgb%s", ++ch);
	else
		sprintf(rgchVarName, "rgbFile");

	// Replace any spaces with underscores:
	int cch = (int)strlen(rgchVarName);
	for (int i = 0; i < cch; i++)
		if (rgchVarName[i] == ' ')
			rgchVarName[i] = '_';

	fprintf(fOutput, "unsigned char %s[] =\n{", rgchVarName);

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

			char psz[10];
			if (ch < 16)
				sprintf(psz, "0x0%x, ", ch);
			else
				sprintf(psz, "0x%x, ", ch);
			fputs(psz, fOutput);
			nByteCount++;
		}
	}
	fputs("\n};\n", fOutput);

	fclose(fOutput);
	fclose(fInput);
}


int main(int argc, char * argv[])
{
	if (argc < 3)
	{
		MessageBox(NULL, "Usage: CrunchHelpFiles <input-file> <output-file>", "Usage error", 0);
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
