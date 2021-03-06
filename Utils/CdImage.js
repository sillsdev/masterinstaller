// JScript to control creation of a CD image (ISO file).
// Called with one parameter: The full path to a folder whose name will be used as the
// Volume Label for the CD, and whose contents will form the CD content.
// Subversion meta data folders (.svn) are automatically excluded.

// Works by building a batch file full of calls to msio.exe, adding one file at a time.

var fso = new ActiveXObject("Scripting.FileSystemObject");
var shellObj = new ActiveXObject("WScript.Shell");

// Check we have a valid folder argument:
if (WScript.Arguments.Length < 1)
{
	WScript.Echo("ERROR: must specify folder to be made into .iso image.");
	WScript.Quit();
}

var SourceFolder = WScript.Arguments.Item(0);
if (!fso.FolderExists(SourceFolder))
{
	WScript.Echo("ERROR - Folder '" + SourceFolder + "' does not exist.");
	WScript.Quit();
}
var IsoFile = SourceFolder + ".iso";
var BatchFile = SourceFolder + ".temp.bat";

var UDF = false;
if (WScript.Arguments.Length > 1)
{
	if (WScript.Arguments.Item(1).toUpperCase() == "-UDF")
		UDF = true;
}
// Check that we can access the MagicISO Console tool:
var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
if (MasterInstallerPath == "%MASTER_INSTALLER%")
{
	WScript.Echo("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
	WScript.Quit();
}
var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");
var MagicISOFile = fso.BuildPAth(UtilsPath, "miso.exe");
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
		WScript.Echo("ERROR - MagicISO Console tool does not exist in '" + UtilsPath + "' or in PATH.");
		WScript.Quit();
	}
}

// Deduce volume label:
var iLastBackslash = SourceFolder.lastIndexOf("\\");
var VolumeLabel = SourceFolder.slice(iLastBackslash + 1);

// Get list of all files to be used in CD image:
var fFoundSvn = false;
var FileList = GetFileList(SourceFolder, true);

// Initiate creation of .iso file (in batch file):
var tso = fso.OpenTextFile(BatchFile, 2, true);
tso.WriteLine("@echo off");
tso.Close();

var args = " -aj -l2 -k2";
if (UDF)
	args = " -aj -au -l2 -k2";

MagicISO("-l " + VolumeLabel + args);

// If we found any .svn folders, we have to set up a batch file adding individual
// files to the ISO one by one.
// If we didn't find and .svn folders, we can add the whole lot in one go, without
// actually using the list!
if (fFoundSvn)
{
	// Add files:
	for (i = 0; i < FileList.length; i++)
	{
		// See if there are any subfolders to be put in the CD image:
		var PathMinusSource = FileList[i].slice(SourceFolder.length + 1);
		var iLastBackslash = PathMinusSource.lastIndexOf("\\");
		var CmdLine = "";
		if (iLastBackslash != -1)
		{
			var LocalPath = PathMinusSource.slice(0, iLastBackslash);
			CmdLine += ' -c "' + LocalPath + '" -f "' + LocalPath + '"';
		}
		CmdLine += ' -a "' + FileList[i] + '"';
		MagicISO(CmdLine);
	}
}
else
{
	var CmdLine = ' -a "' + SourceFolder + '\\*"';
	MagicISO(CmdLine);
}

// Run the temporary batch file we have been building:
Cmd = 'cmd /D /C ""' + BatchFile + '""';
shellObj.Run(Cmd, 1, true);
// Delete the batch file:
fso.DeleteFile(BatchFile);



// Adds to the batch file a call to the MagicISO Console tool with the specified command line.
function MagicISO(CmdLine)
{
	//	Cmd = 'cmd /D /C ""' + MagicISOFile + '" "' + IsoFile + '" ' + CmdLine + '"';
	Cmd = '"' + MagicISOFile + '" "' + IsoFile + '" ' + CmdLine;
	//	shellObj.Run(Cmd, 0, true); // Put this line back if you remove the batch file mechanism.
	var tso = fso.OpenTextFile(BatchFile, 8, true);
	tso.WriteLine(Cmd);
	tso.Close();
}


// Generates a list of files using the given DOS file specification, which may include
// wildcards. Returns an array of strings listing full path to each file.
// Works by using the DOS dir command, redirecting output to a temp file, then
// reading in the file.
// Filters out any .svn folders (Subversion metadata).
function GetFileList(FileSpec, RecurseSubfolders)
{
	fFoundSvn = false;
	var Attributes = '-D';

	// Get the root folder at the base of the search:
	var RootFolder;
	if (fso.FolderExists(FileSpec))
		RootFolder = FileSpec;
	else
	{
		var iLastBackslash = FileSpec.lastIndexOf("\\");
		RootFolder = FileSpec.substr(0, iLastBackslash);
		if (!fso.FolderExists(RootFolder))
		{
			Exception = new Object();
			Exception.description = "Source specification '" + FileSpec + "' does not refer to a valid, accessible folder.";
			throw (Exception);
		}
	}
	// Build DOS dir command:
	Cmd = 'cmd /Q /D /C dir "' + FileSpec + '" /B';
	if (RecurseSubfolders)
		Cmd += ' /S';
	if (Attributes)
		Cmd += ' /A:' + Attributes;

	// Get path to temp file:
	var TempFolderName = fso.GetSpecialFolder(2);
	var TempFileName = fso.GetTempName();
	var TempFilePath = fso.BuildPath(TempFolderName, TempFileName);

	// Specify redirection to temp file in the DOS command:	
	Cmd += ' >"' + TempFilePath + '"';

	// Run DOS command:
	shellObj.Run(Cmd, 0, true);

	// Read resulting file:
	var File = fso.OpenTextFile(TempFilePath, 1);
	var FileList = new Array();
	var Index = 0;
	while (!File.AtEndOfStream)
	{
		var CurrentFile;
		// If we were recursing folders, the full path will be given already:
		if (RecurseSubfolders)
			CurrentFile = File.ReadLine();
		else // we have to add the root folder to the file name
			CurrentFile = fso.BuildPath(RootFolder, File.ReadLine());

		// Make sure there is nothing from Subversion in the path:
		if (CurrentFile.search(".svn") < 0)
		{
			FileList[Index] = CurrentFile;
			Index++;
		}
		else
			fFoundSvn = true;
	}
	File.Close();
	fso.DeleteFile(TempFilePath);

	return FileList;
}
