// JScript to create self-extracting archive of a folder's contents, which will launch
// the setup.exe file after extracting the files.
// Called with one parameter: The full path to a folder whose name will be used as the
// file name for the 7-zip archive, and whose contents will form the archive content.
// (That folder must contain a setup.exe file.)
// Subversion meta data folders (.svn) are automatically excluded.
// If the parameter is missing, then this script merely sets up the registry,
// creating a shell extension, so that a folder can be right-clicked on to create a
// 7-zip archive from that folder's contents.
// Prerequisite: Must have 7-zip tool in same path as this script, or somewhere on PATH.
// 7-zip can be downloaded from http://www.7-zip.org.

// Works by building a batch file full of calls to 7za, adding one file at a time.

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

// Get path of this script:
var iLastBackslash = WScript.ScriptFullName.lastIndexOf("\\");
var ScriptPath = WScript.ScriptFullName.slice(0, iLastBackslash);

// Check we have a valid folder argument:
if (WScript.Arguments.Length < 1)
{
	// Assume user just double-clicked on this script. We will register it for context
	// submenu use.
	var Path = ScriptPath;
	
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
	var RegFile = fso.BuildPath(Path, "SilSfxSetup.reg");
	var tso = fso.OpenTextFile(RegFile, 2, true);
	tso.WriteLine('Windows Registry Editor Version 5.00');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\SilSfxSetup]');
	tso.WriteLine('"EditFlags"=hex:01,00,00,00');
	tso.WriteLine('@="Create SIL Software Package from this folder\'s contents"');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\SilSfxSetup\\command]');
	// OK, deep breath, now:
	tso.WriteLine('@="C:\\\\windows\\\\system32\\\\wscript.exe \\"' + bs2Path + '\\\\CreateSfxSetup.js\\" \\"%1\\"\"');
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

var iLastBackslash = SourceFolder.lastIndexOf("\\");
var RootFolder = SourceFolder.slice(iLastBackslash + 1);
var LocationFolder = SourceFolder.slice(0, iLastBackslash);
var ZipFile = RootFolder + ".7z";
var SfxFile = RootFolder + ".exe";
var iFirstBackslash = SourceFolder.indexOf("\\");
var LocationDrive = SourceFolder.slice(0, iFirstBackslash);

var BatchFile = SourceFolder + ".temp.bat";
var ConfigFile = SourceFolder + ".Config.txt";
var ListFile = SourceFolder + ".list.txt";

// Check that we can access the 7-zip tool:
var SevenZipExeFile = ScriptPath + "\\7za.exe"
if (!fso.FileExists(SevenZipExeFile))
{
	// Try to run 7za, to see if file exists on path:
	SevenZipExeFile = "7za.exe"
	try
	{
		var Cmd = SevenZipExeFile;
		shellObj.Run(Cmd, 0, true);
	}
	catch (err)
	{
		WScript.Echo("ERROR - 7-zip Console tool '7za.exe' does not exist in in " + ScriptPath + " or on PATH.");
		WScript.Quit();
	}
}

// Get list of all files to be put in archive:
var fFoundSvn = false;
var FileList = GetFileList(SourceFolder, true);

// Initiate creation of 7-zip batch file:
var tso = fso.OpenTextFile(BatchFile, 2, true);
tso.WriteLine("@echo off");
tso.WriteLine(LocationDrive);
tso.WriteLine('cd "' + LocationFolder + '"');

// If we found any .svn folders, we have to use a list file, adding individual
// files to the list one by one.
// If we didn't find and .svn folders, we can add the whole lot in one go, without
// using a list file:
if (fFoundSvn)
{
	// Add files:
	for (i = 0; i < FileList.length; i++)
		AddFile(FileList[i]);
	tso.WriteLine('"' + SevenZipExeFile + '" a "' + ZipFile + '" + @"' + ListFile + '" -mx=9 -mmt=on');
}
else
{
	tso.WriteLine('"' + SevenZipExeFile + '" a "' + ZipFile + '" + "' + RootFolder + '\\*" -r -mx=9 -mmt=on');
}

// Create configuration file to bind to self-extracting archive:
var tsoConfig = fso.OpenTextFile(ConfigFile, 2, true, 0); // Must be UTF-8; ASCII will do for us
tsoConfig.WriteLine(';!@Install@!UTF-8!');
tsoConfig.WriteLine('Title="SIL \'' + RootFolder + '\' Software Package"');
tsoConfig.WriteLine('HelpText="Double-click the file \'' + RootFolder + '.exe\' to extract the installation files and run the installer.\nThe extracted files will be deleted when the installer has finished.\n\nRun the file with the -nr option to simply extract the files and leave them.\n\nFile extraction will be to the same folder where this file is, in both cases."');
tsoConfig.WriteLine('InstallPath="%%S"');
tsoConfig.WriteLine('Delete="%%S\\' + RootFolder + '"');
tsoConfig.WriteLine('ExtractTitle="Extracting installation files"');
tsoConfig.WriteLine('ExtractDialogText="Preparing the \'' + RootFolder + '\' files for installation"');
tsoConfig.WriteLine('RunProgram="fm0:\\"' + RootFolder + '\\setup.exe\\""');
tsoConfig.WriteLine(';!@InstallEnd@!');
tsoConfig.Close();

// Add self-extracting module and configuration to launch setup.exe:
tso.WriteLine('copy /b "' + ScriptPath + '\\7zSD_new.sfx" + "' + ConfigFile + '" + "' + ZipFile + '" "' + SfxFile + '"');
tso.Close();

// Run the temporary batch file we have been building:
Cmd = 'cmd /D /C ""' + BatchFile + '""';
shellObj.Run(Cmd, 1, true);

// Delete the temporary working files:
fso.DeleteFile(BatchFile);
fso.DeleteFile(ConfigFile);
fso.DeleteFile(ZipFile);
if (fso.FileExists(ListFile))
	fso.DeleteFile(ListFile);

// If the md5sums utility exists in the same folder as this script, use it to generate an md5 hash
// of the new archive file:
var Md5SumsPath = ScriptPath + "\\md5sums.exe";
if (fso.FileExists(Md5SumsPath))
{
	var md5File = LocationFolder + '\\' + RootFolder + '.md5.txt';
	// Make new batch file:
	var tso = fso.OpenTextFile(BatchFile, 2, true);
	tso.WriteLine("@echo off");
	tso.WriteLine(LocationDrive);
	tso.WriteLine('cd "' + LocationFolder + '"');
	tso.WriteLine('"' + Md5SumsPath + '" -u "' + LocationFolder + '\\' + SfxFile + '" >"' + md5File + '"');
	tso.Close();

	// Run the temporary batch file we have been building:
	Cmd = 'cmd /D /C ""' + BatchFile + '""';
	shellObj.Run(Cmd, 1, true);
	// Delete the batch and list files:
	fso.DeleteFile(BatchFile);
	
	// Get the raw md5 value from among the other junk in the output of md5sums:
	var tso = fso.OpenTextFile(md5File, 1, true);
	var Line = tso.ReadLine();
	tso.Close();
	var IndexOfSpace = Line.indexOf(' ');
	var MD5 = Line.slice(0, IndexOfSpace);

	// Write the raw md5 value into the outptut file:
	var tso = fso.OpenTextFile(md5File, 2, true);
	tso.WriteLine(MD5);
	tso.Close();
}

// Adds to the batch file a call to the 7za tool with the specified file.
function AddFile(NewFile)
{
	// Remove SourceFolder from NewFile:
	if (NewFile.indexOf(SourceFolder) == 0)
		NewFile = RootFolder + NewFile.slice(SourceFolder.length);
	
	var tso = fso.OpenTextFile(ListFile, 8, true);
	tso.WriteLine('"' + NewFile + '"');
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
