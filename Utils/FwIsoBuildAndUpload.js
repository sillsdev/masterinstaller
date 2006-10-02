// JScript to automatically upload an up-to-date FieldWorks and Encoding Converters
// ISO archive (self extracting exe)
// Prerequisites:
// 1) Must have MagicISO Console tool either in C:\Program Files\MagicISO or somewhere in PATH.
//    (MagicISO can be downloaded from www.magiciso.com, but costs $30 to run properly.)
// 2) Must have 7-zip compression utility installed in C:\Program Files\7zip
// 3) Must have BCopy tool in c:\fw\bin
// 4) Must have CdImage.js script in the same folder as this file.
//
// Call this script with the following parameters:
// 1) The path where up-to-date FieldWorks installer files are located
// 2) The path where up-to-date Encoding Converters installer files are located
// 3) The age limit, in hours, for the files to be accepted
// 4) The path where the ISO and self-extracting archive are to be built
// 5) The folder in the above path (3) where the CD files and folders are laid out
// 6 and beyond) Destination(s) where the self-extracting archive is to be uploaded
// Note that the paths in (1), (2), (4) and (6) can all be network paths. FTP sites are not allowed.
//
// Example parameters:
// 1) "\\cana\LSDev\Installers\FW4.0"
// 2) "\\cana\LSDev\Installers\EC2.5"
// 3) 24
// 4) "E:\CD Builder\CDs\FW 4.0"
// 5) LST_07-011
// 6) "\\jar-file\sillangsoft\FW_4_0_ISO"
// 7) "\\cana\LSDev\Installers\FW4.0"
//
// The program works as follows:
// 1) Check existence of files in path arguments (1) and (2).
// 2) If files are young enough (argument 3), copy them into CD file structure (arg 4+5)
// 3) Make ISO file from CD file structure. Subversion meta data folders (.svn) are automatically excluded.
// 4) Compress ISO file into self-extracting archive.
// 5) Upload archive to destination in arguments (6) and beyond.

// Check we have valid arguments:
if (WScript.Arguments.Length < 6)
{
	WScript.Echo("Usage: needs at least 6 arguments:\n" +
		"1) The path where up-to-date FW installer is located\n" +
		"2) The path where up-to-date EC installer is located\n" +
		"3) The age limit, in hours, for the msi files to be accepted\n" +
		"4) The path where the ISO and self-extracting archive are to be built\n" +
		"5) The folder in the above path (3) where the CD files and folders are laid out\n" +
		"6 and beyond) Destination(s) where the self-extracting archive is to be uploaded\n" +
		"Note that the paths in (1), (2), (4) and (6) can all be network paths. FTP sites are not allowed.");
	WScript.Quit();
}

var FWSourcePath = WScript.Arguments.Item(0);
var ECSourcePath = WScript.Arguments.Item(1);
var NumECCabFiles = 13;
var AgeLimitHours = WScript.Arguments.Item(2);
var WorkspacePath = WScript.Arguments.Item(3);
var CdFolder = WScript.Arguments.Item(4);
var DestinationPath = new Array();
for (i = 5; i < WScript.Arguments.length; i++)
	DestinationPath.push(WScript.Arguments.Item(i));

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

// Magic ISO command tool:
var MagicISOFile = "C:\\Program Files\\MagicISO\\miso.exe"
// 7 Zip command tool:
var ZipDir = "C:\\Program Files\\7zip";
// BCopy command tool:
var BCopy = "C:\\Fw\\Bin\\BCopy.exe";

// Get the drive (letter and colon) of the workspace path:
var WorkspaceDrive = WorkspacePath.slice(0, 2);
// Prepare file names based on CdFolder:
var IsoFile = CdFolder + ".iso";
var IsoPath = fso.BuildPath(WorkspacePath, IsoFile);
var ArchiveFile = IsoFile + ".exe";
var ArchivePath = fso.BuildPath(WorkspacePath, ArchiveFile);
var CdFullPath = fso.BuildPath(WorkspacePath, CdFolder);

// Check if installer exists on FWSourcePath:
var SetupFwMsiSource = fso.BuildPath(FWSourcePath, "SetupFW.msi");
var SetupFwMsi = fso.GetFile(SetupFwMsiSource);
if (SetupFwMsi == null)
	LogFatalError("SetupFW.msi not found in " + FWSourcePath);

var SetupFw5CabSource = fso.BuildPath(FWSourcePath, "SetupFW5.cab");
var SetupFw5Cab = fso.GetFile(SetupFw5CabSource);
if (SetupFw5Cab == null)
	LogFatalError("SetupFw5.Cab not found in " + FWSourcePath);

var SetupFw6CabSource = fso.BuildPath(FWSourcePath, "SetupFW6.cab");
var SetupFw6Cab = fso.GetFile(SetupFw6CabSource);
if (SetupFw6Cab == null)
	LogFatalError("SetupFw6.Cab not found in " + FWSourcePath);

// Check if installer exists on ECSourcePath:
var SetupECMsiSource = fso.BuildPath(ECSourcePath, "SetupEC.msi");
var SetupECMsi = fso.GetFile(SetupECMsiSource);
if (SetupECMsi == null)
	LogFatalError("SetupEC.msi not found in " + ECSourcePath);

var SetupECCabSource = new Array();
var SetupECCab = new Array();
for (i = 0; i < NumECCabFiles; i++)
{
	SetupECCabSource.push(fso.BuildPath(ECSourcePath, "SetupEC" + (i + 1) + ".cab"));
	SetupECCab.push(fso.GetFile(SetupECCabSource[i]));
	if (SetupECCab[i] == null)
		LogFatalError("SetupEC" + (i + 1) + ".cab not found in " + ECSourcePath);
}

// Get date of installer files:
var SetupFwMsiDateMs = Date.parse(SetupFwMsi.DateLastModified);
var SetupECMsiDateMs = Date.parse(SetupECMsi.DateLastModified);

// See if installer date is within AgeLimitHours hours of now:
var Now = new Date();
var SetupFwMsiAgeMs = Now.getTime() - SetupFwMsiDateMs;
var SetupFwMsiAgeHours = SetupFwMsiAgeMs / (1000 * 60 * 60);
if (SetupFwMsiAgeHours > AgeLimitHours)
	LogFatalError("SetupFW.msi more than " + AgeLimitHours + " hours old (" + SetupFwMsiAgeHours + " hours)");

var SetupECMsiAgeMs = Now.getTime() - SetupECMsiDateMs;
var SetupECMsiAgeHours = SetupECMsiAgeMs / (1000 * 60 * 60);
if (SetupECMsiAgeHours > AgeLimitHours)
	LogFatalError("SetupEC.msi more than " + AgeLimitHours + " hours old (" + SetupECMsiAgeHours + " hours)");

// Copy FW installer files from FWSourcePath to CdFolder:
var LocalFwPath = fso.BuildPath(CdFullPath, "FieldWorks");
var Cmd = '"' + BCopy + '" "' + SetupFwMsiSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

Cmd = '"' + BCopy + '" "' + SetupFw5CabSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

Cmd = '"' + BCopy + '" "' + SetupFw6CabSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

// Copy EC installer files from ECSourcePath to CdFolder:
var LocalECPath = fso.BuildPath(CdFullPath, "Encoding Converters");
Cmd = '"' + BCopy + '" "' + SetupECMsiSource + '" "' + LocalECPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);
for (i = 0; i < NumECCabFiles; i++)
{
	Cmd = '"' + BCopy + '" "' + SetupECCabSource[i] + '" "' + LocalECPath + '" /b:500 /r:40 /f';
	shellObj.Run(Cmd, 1, true);
}

// Remove existing ISO file:
if (fso.FileExists(IsoPath))
	fso.DeleteFile(IsoPath);

// Get path of this script:
var iLastBackslash = WScript.ScriptFullName.lastIndexOf("\\");
var ScriptPath = WScript.ScriptFullName.slice(0, iLastBackslash);

// Call CdImage.js script to create new ISO file:
Cmd = 'wscript.exe "' + fso.BuildPath(ScriptPath, "CdImage.js") + '" "' + CdFullPath + '"';
shellObj.Run(Cmd, 1, true);

// Compress ISO file:
var BatchFile = fso.BuildPath(WorkspacePath, "compress.bat");
var tso = fso.OpenTextFile(BatchFile, 2, true);
tso.WriteLine('cd "' + WorkspacePath + '"');
tso.WriteLine(WorkspaceDrive);
var SevenZaExe = '"' + fso.BuildPath(ZipDir, "7za.exe") + '"';
var SelfExtract = ' a "-sfx' + fso.BuildPath(ZipDir, "7zC.sfx") + '"';
var OutputFile = ' "' + ArchiveFile + '"';
var InputFile = ' "' + IsoFile + '"';
var Compression = ' -mx=9 -mmt=on""';
tso.WriteLine(SevenZaExe + SelfExtract + OutputFile + InputFile + Compression);
tso.Close();
Cmd = 'cmd /D /C ""' + BatchFile + '""';
shellObj.Run(Cmd, 1, true);
fso.DeleteFile(BatchFile);

// Copy file to DestinationPath(s):
for (i = 0; i < DestinationPath.length; i++)
{
	Cmd = '"' + BCopy + '" "' + ArchivePath + '" "' + fso.BuildPath(DestinationPath[i], ArchiveFile) + '" /b:500 /r:40 /f';
	shellObj.Run(Cmd, 1, true);
}


function LogFatalError(Msg)
{
	// Create new log file:
	var tso = fso.OpenTextFile(fso.BuildPath(WorkspacePath, "Error.log"), 2, true);
	tso.WriteLine('Error: ' + Msg);
	tso.Close();

	WScript.Quit();
}

// Check that we can access the MagicISO Console tool:
function VerifyMagicIso()
{
	if (!fso.FileExists(MagicISOFile))
	{
		// Try to run miso, to see if file exists on path:
		MagicISOFile = "miso.exe"
		try
		{
			var Cmd = MagicISOFile + ' -py';
			shellObj.Run(Cmd, 0, true);
		}
		catch (err)
		{
			LogFatalError("ERROR - MagicISO Console tool does not exist in C:\Program Files\MagicISO or in PATH.");
		}
	}
}

// Runs a call to the MagicISO Console tool with the specified command line.
function MagicISO(CmdLine)
{
//	Cmd = 'cmd /D /C ""' + MagicISOFile + '" "' + fso.BuildPath(WorkspacePath, IsoFile) + '" ' + CmdLine + '"';
	Cmd = '"' + MagicISOFile + '" "' + fso.BuildPath(WorkspacePath, IsoFile) + '" ' + CmdLine;
	shellObj.Run(Cmd, 0, true);
}
