// JScript to control creation of a CD image (ISO file).
// Called with one parameter: The full path to a folder whose name will be used as the
// Volume Label for the CD, and whose contents will form the CD content.
// If the parameter is "-register", then this script merely sets up the registry
// creating a shell extension, so that a folder can be right-clicked on to create a
// CD image from that folder's contents.
// Prerequisite: Must have MagicISO Console tool either in C:\Program Files\MagicISO or
// somewhere in PATH.
// MagicISO can be downloaded from www.magiciso.com, but costs $30 to run properly.

// Works by building a batch file full of calls to msio.exe, adding one file at a time.

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

// Check we have a valid folder argument:
if (WScript.Arguments.Length < 1)
{
	// Assume user just double-clicked on this script. We will register it for context
	// submenu use.
	// Get path of this script:
	var iLastBackslash = WScript.ScriptFullName.lastIndexOf("\\");
	var Path = WScript.ScriptFullName.slice(0, iLastBackslash);
	
	// Build a version of the Path with doubled up backslashes:
	var aFolder = Path.split("\\");
	var bs2Path = "";
	for (i = 0; i < aFolder.length; i++)
	{
		if (i > 0)
			bs2Path += "\\\\";
		bs2Path += aFolder[i];
	}
	
	// Write registry settings:
	var RegFile = fso.BuildPath(Path, "CdImage.reg");
	var tso = fso.OpenTextFile(RegFile, 2, true);
	tso.WriteLine('Windows Registry Editor Version 5.00');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\CD_Image]');
	tso.WriteLine('"EditFlags"=hex:01,00,00,00');
	tso.WriteLine('@="Make CD image with this label"');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\CD_Image\\command]');
	// OK, deep breath, now:
	tso.WriteLine('@="C:\\\\windows\\\\system32\\\\wscript.exe \\"' + bs2Path + '\\\\CdImage.js\\" \\"%1\\"\"');
	tso.Close();
	
	// Run Regedit with the new file:
	var Cmd = 'Regedit.exe "' + RegFile + '"';
	shellObj.Run(Cmd, 0, true);
	
	// Delete RegFile:
	fso.DeleteFile(RegFile);

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

// Check that we can access the MagicISO Console tool:
var MagicISOFile = "C:\\Program Files\\MagicISO\\miso.exe"
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
		WScript.Echo("ERROR - MagicISO Console tool does not exist in C:\Program Files\MagicISO or in PATH.");
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
MagicISO("-l " + VolumeLabel + " -aj -l2 -k2");

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
			throw(Exception);
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
