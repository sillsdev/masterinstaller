// JScript to automatically upload an up-to-date FieldWorks ISO archive (self extracting exe)
// Prerequisites:
// 1) Must have MagicISO Console tool either in C:\Program Files\MagicISO or somewhere in PATH.
//    (MagicISO can be downloaded from www.magiciso.com, but costs $30 to run properly.)
// 2) Must have 7-zip compression utility installed in C:\Program Files\7zip
// 3) Must have BCopy tool in c:\fw\bin
// 4) Must have CdImage.js script in the same folder as this file.
//
// Call this script with the following parameters:
// 1) The path where up-to-date SetupFW.msi is located
// 2) The age limit, in hours, for the msi file to be accepted
// 3) The path where the ISO and self-extracting archive are to be built
// 4) The folder in the above path (3) where the CD files and folders are laid out
// 5) The destination where the self-extracting archive is to be uploaded
// Note that the paths in (1), (3) and (5) can all be network paths. FTP sites are not allowed.
//
// The program works as follows:
// 1) Check existence of SetupFW.msi in path argument (1).
// 2) If msi file is young enough (argument 2), copy it into CD file structure (arg 3+4)
// 3) Make ISO file from CD file structure.
// 4) Compress ISO file into self-extracting archive.
// 5) Upload archive to destination in argument (5).

// Check we have valid arguments:
if (WScript.Arguments.Length != 5)
{
	WScript.Echo("Usage: needs 5 arguments:\n" +
		"1) The path where up-to-date SetupFW.msi is located\n" +
		"2) The age limit, in hours, for the msi files to be accepted\n" +
		"3) The path where the ISO and self-extracting archive are to be built\n" +
		"4) The folder in the above path (3) where the CD files and folders are laid out\n" +
		"5) The destination where the self-extracting archive is to be uploaded\n" +
		"Note that the paths in (1), (3) and (5) can all be network paths. FTP sites are not allowed.");
	WScript.Quit();
}

var MsiSourcePath = WScript.Arguments.Item(0);
var AgeLimitHours = WScript.Arguments.Item(1);
var WorkspacePath = WScript.Arguments.Item(2);
var CdFolder = WScript.Arguments.Item(3);
var DestinationPath = WScript.Arguments.Item(4);

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

// Check if installer exists on MsiSourcePath:
var SetupFwMsiSource = fso.BuildPath(MsiSourcePath, "SetupFW.msi");
var SetupFwMsi = fso.GetFile(SetupFwMsiSource);
if (SetupFwMsi == null)
	LogFatalError("SetupFW.msi not found in " + MsiSourcePath);

var SetupFw5CabSource = fso.BuildPath(MsiSourcePath, "SetupFW5.cab");
var SetupFw5Cab = fso.GetFile(SetupFw5CabSource);
if (SetupFw5Cab == null)
	LogFatalError("SetupFw5.Cab not found in " + MsiSourcePath);

var SetupFw6CabSource = fso.BuildPath(MsiSourcePath, "SetupFW6.cab");
var SetupFw6Cab = fso.GetFile(SetupFw6CabSource);
if (SetupFw6Cab == null)
	LogFatalError("SetupFw6.Cab not found in " + MsiSourcePath);

// Get date of installer:
var SetupFwMsiDateMs = Date.parse(SetupFwMsi.DateLastModified);

// See if installer date is within AgeLimitHours hours of now:
var Now = new Date();
var SetupFwMsiAgeMs = Now.getTime() - SetupFwMsiDateMs;
var SetupFwMsiAgeHours = SetupFwMsiAgeMs / (1000 * 60 * 60);
if (SetupFwMsiAgeHours > AgeLimitHours)
	LogFatalError("SetupFW.msi more than " + AgeLimitHours + " hours old (" + SetupFwMsiAgeHours + " hours)");

// Copy installer from MsiSourcePath to CdFolder:
var LocalFwPath = fso.BuildPath(CdFullPath, "FieldWorks");
var Cmd = '"' + BCopy + '" "' + SetupFwMsiSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

Cmd = '"' + BCopy + '" "' + SetupFw5CabSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

Cmd = '"' + BCopy + '" "' + SetupFw6CabSource + '" "' + LocalFwPath + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);

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

// Copy file to DestinationPath:
Cmd = '"' + BCopy + '" "' + ArchivePath + '" "' + fso.BuildPath(DestinationPath, ArchiveFile) + '" /b:500 /r:40 /f';
shellObj.Run(Cmd, 1, true);


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
