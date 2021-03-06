// This script takes a mater installer XML file, and rebuilds the web download package,
// updating files where necessary (or gathering them from scratch, if no package subfolder
// with the same name as the XML file is found in the same folder).
// It relies on an SFX config file being present in the same folder as the master installer XML file.
// This is generally the case when the ConfigureWebDownloads.xsl was previously used to build the download package.

var fso = new ActiveXObject("Scripting.FileSystemObject");
var shellObj = new ActiveXObject("WScript.Shell");

if (WScript.Arguments.Length < 1)
{
	WScript.Echo("ERROR: must specify a master installer XML.");
	WScript.Quit();
}

var XmlFileName = WScript.Arguments.Item(0);

// Check that the input file is an XML file:
if (XmlFileName.slice(-4).toLowerCase() != ".xml")
{
	WScript.Echo("ERROR in RebuildWebDownload.js - XML file must be specified: " + XmlFileName);
	WScript.Quit();
}

// Check that the XML file has a <MasterInstaller> node:
var xmlDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
xmlDoc.async = false;
xmlDoc.load(XmlFileName);
if (xmlDoc.parseError.errorCode != 0)
{
	var myErr = xmlDoc.parseError;
	WScript.Echo("You have an XML error in " + XmlFileName + ": " + myErr.reason + " on line " + myErr.line + " at position " + myErr.linepos);
	WScript.Quit();
}
if (xmlDoc.selectSingleNode("/MasterInstaller") == null)
{
	WScript.Echo("ERROR - " + XmlFileName + " is not a Master Installer configuration file." + xmlDoc.xml);
	WScript.Quit();
}

// Sort out which activities we should avoid, based on command line arguments:
var fBuildSetupExe = true;
var fGatherFiles = true;
var fBuildSfx = true;
var fShowLog = true;
var CertificatePassword = "";

for (i = 1; i < WScript.Arguments.Length; i++)
{
	var arg = WScript.Arguments.Item(i);

	if (arg.slice(0, 21) == "-certificatepassword:") // See if a certificate signing password was supplied
		CertificatePassword = arg.slice(21);
	else
	{
		switch (arg)
		{
			case "-NoBuildSetupExe":
				fBuildSetupExe = false;
				break;
			case "-NoGatherFiles":
				fGatherFiles = false;
				break;
			case "-NoBuildSfx":
				fBuildSfx = false;
				break;
			case "-NoShowLog":
				fShowLog = false;
				break;
		}
	}
}

var iLastBackslash = XmlFileName.lastIndexOf("\\");
var RootFolder = XmlFileName.substr(0, iLastBackslash);
// The package contents go in a folder with the same name as the master installer XML
// file (minus the .xml extension):
var FlavorFolder = XmlFileName.slice(0, -4);
var FlavorName = FlavorFolder.slice(iLastBackslash + 1);
var SfxFile = FlavorFolder + ".exe";

var ReportFile = fso.BuildPath(RootFolder, "RebuildWebDownload." + FlavorName + ".log");
var tsoLog = fso.OpenTextFile(ReportFile, 2, true);

tsoLog.WriteLine("Rebuild of " + FlavorFolder);
Report("Started.");

// Get list of existing files within the destination folder tree, so we can compare files
// before deciding which to copy over:
var OriginalFiles = GetFileList(FlavorFolder, true);
OriginalFileList = OriginalFiles.FileList;

Report("Starting off with " + OriginalFileList.length + " files already in " + FlavorFolder + ".");

var NumProducts;
var SourceFileLists;
var FileTotal;
var ExternalHelpFileData = null;
var TermsOfUseFileData = null;

var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
var ProductsPath = shellObj.ExpandEnvironmentStrings("%PACKAGE_PRODUCTS%");

if (MasterInstallerPath == "%MASTER_INSTALLER%")
	WScript.Echo("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
else if (ProductsPath == "%PACKAGE_PRODUCTS%")
	WScript.Echo("ERROR: the PACKAGE_PRODUCTS environment variable has not been defined. You cannot create web download packages without specifying where the products and documents are stored.");

var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");
var SevenZipExeFile = fso.BuildPath(UtilsPath, "7za.exe");
var BitmapsPath = fso.BuildPath(MasterInstallerPath, "Code and Projects\\Bitmaps");

var f7ZipFoundSvn = false;

// Do a test for legacy data that specified a full path to the background bitmap:
var bmp = xmlDoc.selectSingleNode("/MasterInstaller/General/ListBackground").text;
if (bmp.indexOf("\\") >= 0)
{
	if (!fso.FileExists(bmp))
	{
		WScript.Echo("ERROR: Bitmap path '" + bmp + "' specified in XML node /MasterInstaller/General/ListBackground does not exist. This may be legacy data. Bitmaps should be stored in the '" + BitmapsPath + "' folder and referenced by file name only.");
		WScript.Quit();
	}
}

var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
NumProducts = ProductNodeList.length;
FileTotal = 0;
GenerateSourceFileLists(); // Must come after assigning NumProducts and before calling FindDuplicateSets().
GenerateExtHelpAndTermsFileLists();
BuildPackage();

Report("Ended.");
tsoLog.Close();

if (fShowLog)
	shellObj.Run('Notepad "' + ReportFile + '"');

// Builds an array of file lists for each product, where the files in the list are interpretted
// from the AutoConfigure/Source nodes of each product. (More than one source node per product
// is allowed.)
function GenerateSourceFileLists()
{
	SourceFileLists = new Array();

	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
	
	// Iterate through all products' sources:
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		var FileListData = new Array();
		var AllOk = true;
		var ProductNode = ProductNodeList[iProduct];
		var ProductSourceList = ProductNode.selectNodes('AutoConfigure/Source');
		for (iSource = 0; iSource < ProductSourceList.length; iSource++)
		{
			var ProductSource = ProductSourceList[iSource];
			var NameWhenLocked = ProductSource.getAttribute("NameWhenLocked");
			var Attributes = ProductSource.getAttribute("Attributes");
			var DestPath = ProductSource.getAttribute("DestPath");
			var SignAs = ProductSource.getAttribute("SignAs");

			// Check if source path is relative:
			var SourcePath = CheckProductRelativePath(ProductSource.text, ProductsPath);

			// Whether or not we recurse depends on the following rules, in order
			// of decreasing priority:
			// 1) If there is a Recurse attribute defined, we use its value
			// 2) Otherwise, if the source text is an existing folder, we recurse
			// 3) Otherwise, we do not recurse.
			var Recurse = false; // Value if all other tests fail
			var RecurseAttribute = ProductSource.getAttribute("Recurse");
			if (RecurseAttribute)
				Recurse = (RecurseAttribute == "true");
			else if (fso.FolderExists(SourcePath))
				Recurse = true;

			// The SignAs value is only valid if the Source is a file (not a directory):
			if (!fso.FileExists(SourcePath))
				SignAs = null;

			var NewListData = GetFileList(SourcePath, Recurse, Attributes);
			if (NameWhenLocked && NewListData.FileList.length > 1)
			{
				Report("Error - Product " + ProductNode.selectSingleNode('Title').text + " contains an AutoConfigure Source node with a NameWhenLocked attribute (" + NameWhenLocked + ") but multiple files matching.");
				WScript.Quit();
			}
			if (NameWhenLocked)
				NewListData.NameWhenLocked = NameWhenLocked;
			NewListData.DestPath = DestPath;
			NewListData.SignAs = SignAs;
			FileTotal += NewListData.FileList.length;

			FileListData[iSource] = NewListData;
		}
		SourceFileLists[iProduct] = new Object();
		SourceFileLists[iProduct].ListData = FileListData;
		SourceFileLists[iProduct].AllFilesFound = AllOk;
	} // Next Product
}

// Builds file list each for the External Help file(s) and the Terms of Use file(s).
function GenerateExtHelpAndTermsFileLists()
{
	// External Help file(s):
	var ExternalHelpSourceNode = xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpSource");
	var Found = false;
	if (ExternalHelpSourceNode)
	{
		var ExternalHelpSource = ExternalHelpSourceNode.text;
		if (ExternalHelpSource.length > 0)
		{
			var Recurse = false;
			if (fso.FolderExists(ExternalHelpSource))
			Recurse = true;
			ExternalHelpSource = CheckProductRelativePath(ExternalHelpSource, ProductsPath);
			ExternalHelpFileData = GetFileList(ExternalHelpSource, Recurse);
			FileTotal += ExternalHelpFileData.FileList.length;
			Found = true;
		}
	}
	if (!Found)
	{
		// Set up empty values:
		ExternalHelpFileData = new Object();
		ExternalHelpFileData.FileList = new Array();
		ExternalHelpFileData.RootFolder = "";
	}

	// Do the same for the Terms of Use file(s):
	Found = false;
	var TermsOfUseSourceNode = xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/TermsOfUseSource");
	if (TermsOfUseSourceNode)
	{
		var TermsOfUseSource = TermsOfUseSourceNode.text;
		if (TermsOfUseSource.length > 0)
		{
			Recurse = false;
			if (fso.FolderExists(TermsOfUseSource))
			Recurse = true;
			TermsOfUseSource = CheckProductRelativePath(TermsOfUseSource, ProductsPath);
			TermsOfUseFileData = GetFileList(TermsOfUseSource, Recurse);
			FileTotal += TermsOfUseFileData.FileList.length;
			Found = true;
		}
	}
	if (!Found)
	{
		// Set up empty values:
		TermsOfUseFileData = new Object();
		TermsOfUseFileData.FileList = new Array();
		TermsOfUseFileData.RootFolder = "";
	}
}

// Analyzes whether the given path is a relative path or a full path specification.
// If relative, the RelativePathPrepend is prepended.
function CheckProductRelativePath(Path, RelativePathPrepend)
{
	var IsRelative = true;
	// Check if Path starts with a backslash:
	if (Path.substr(0, 1) == "\\")
	IsRelative = false;
	// Check if Path starts with a drive letter and colon:
	if (Path.substr(1, 2) == ":\\")
	IsRelative = false;

	// If Path was found to be relative, find the path of the source XML file:
	if (IsRelative)
	Path = fso.BuildPath(RelativePathPrepend, Path);

	return Path;
}

// Generates a list of files using the given DOS file specification, which may include
// wildcards. Returns an object containing an array of strings listing full path to each file,
// and a string giving the original root folder of the list.
// DOS file attributes also can be specified.
// Works by using the DOS dir command, redirecting output to a temp file, then
// reading in the file.
// Filters out any .svn folders (Subversion metadata).
function GetFileList(FileSpec, RecurseSubfolders, Attributes)
{
	if (Attributes)
		Attributes += '-D'; // Force removal of folders from list
	else
		Attributes = '-D';

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
			Report("Source specification '" + FileSpec + "' does not refer to a valid, accessible folder.");
			WScript.Quit();
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
	Report("Running DOS command: " + Cmd);
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
			f7ZipFoundSvn = true;
	}
	File.Close();
	fso.DeleteFile(TempFilePath);

	ReturnObject = new Object();
	ReturnObject.FileList = FileList;
	ReturnObject.RootFolder = RootFolder;
	return ReturnObject;
}

function BuildPackage()
{
	if (!fso.FolderExists(FlavorFolder))
		Report(FlavorFolder + " does not yet exist; building from scratch.");
	
	MakeSureFolderExists(FlavorFolder);

	var FinalSetupExePath = fso.BuildPath(FlavorFolder, "setup.exe");
	var SetupExeBuilt = false;

	if (fBuildSetupExe && !fso.FileExists(FinalSetupExePath))
	{
		Report("Building master installer.");

		// Compile master installer:
		var SetupExePath = CompileMasterInstaller(XmlFileName);

		// Move master installer into its proper folder:
		fso.MoveFile(SetupExePath, FinalSetupExePath);

		SetupExeBuilt = true;
	}
	// Remove from OriginalFileList:
	RemoveFileFromList(FinalSetupExePath);

	if (fGatherFiles)
	{
		Report("Gathering Files.");
		var NumGatheredFiles = GatherFiles(FlavorFolder);
		if (fBuildSfx && (NumGatheredFiles > 0 || SetupExeBuilt || !fso.FileExists(SfxFile)))
		{
			Report("Building self-extracting archive.");
			Compress();
		}
		else
			Report("Did not build self-extracting archive.");
	}
	else
		Report("Did not gather files.");
}

// Create the specified folder path, if it doesn't already exist.
// Thanks, Jeff!
//	Note - Does not handle \\LS-ELMER\ type directory creation.
function MakeSureFolderExists(strDir)
{
	// See if the dir exists.
	if (!fso.FolderExists(strDir))
	{
		var aFolder = new Array();
		aFolder = strDir.split("\\");
		var strNewFolder = fso.BuildPath(aFolder[0], "\\");

		// Loop through each folder name and create if not already created
		var	iFolder;
		for (iFolder = 1; iFolder < aFolder.length; iFolder++)
		{
			strNewFolder = fso.BuildPath(strNewFolder, aFolder[iFolder]);
			if (!fso.FolderExists(strNewFolder))
				fso.CreateFolder(strNewFolder);
		}
	}
}

function CompileMasterInstaller(XmlFileName)
{
	var Cmd = 'wscript.exe "' + fso.BuildPath(UtilsPath, "CompileXmlMasterInstaller.js") + '" "' + XmlFileName + '" -E "-certificatepassword:' + CertificatePassword + '"';
	Report("Running DOS command: " + Cmd);
	shellObj.Run(Cmd, 0, true);
	var SetupExePath = RootFolder + "\\setup.exe";
	return SetupExePath;
}

// Copies files from their various sources to the download package folder.
function GatherFiles(DestinationPath)
{
	MakeSureFolderExists(DestinationPath);
	var FilesCopied = 0;

	// See if we need InstallerHelp.dll or InstallerHelp2.dll:
	if (IsAnyProductLocked())
	{
		var InstallerHelpDll = "InstallerHelp.dll";
		var InstallerHelpDllSourcePath = fso.BuildPath(MasterInstallerPath, InstallerHelpDll);
		var InstallerHelpDllDestPath = fso.BuildPath(DestinationPath, InstallerHelpDll);
		if (TestCopy(InstallerHelpDllSourcePath, InstallerHelpDllDestPath))
			FilesCopied++;
		Report("Warning - InstallerHelp2.dll cannot be generated by this tool");
	}

	// See if we need External Help file(s):
	if (ExternalHelpFileData.FileList.length > 0)
	{
		var FileList = ExternalHelpFileData.FileList;
		var RootFolder = ExternalHelpFileData.RootFolder;
		var Destination = DestinationPath;
		var ExternalHelpDestNode = xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpDestination");
		if (ExternalHelpDestNode)
			Destination = fso.BuildPath(DestinationPath, ExternalHelpDestNode.text);
		for (i = 0; i < FileList.length; i++)
		{
			var SourcePath = FileList[i];
			var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, Destination);
			MakeSureFolderExists(TargetFullPath);
			TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

			if (TestCopy(FileList[i], TargetFullPath))
				FilesCopied++;
		} // Next file
	}

	// See if we need Terms of Use file(s):
	if (TermsOfUseFileData.FileList.length > 0)
	{
		var FileList = TermsOfUseFileData.FileList;
		var RootFolder = TermsOfUseFileData.RootFolder;
		var Destination = DestinationPath;
		var TermsOfUseDestNode = xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/TermsOfUseDestination");
		if (TermsOfUseDestNode)
			Destination = fso.BuildPath(DestinationPath, TermsOfUseDestNode.text);
		for (i = 0; i < FileList.length; i++)
		{
			var SourcePath = FileList[i];
			var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, DestinationPath);
			MakeSureFolderExists(TargetFullPath);
			TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

			if (TestCopy(FileList[i], TargetFullPath))
				FilesCopied++;
		} // Next file
	}

	// Copy all other files:
	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (iProduct = 0; iProduct < ProductNodeList.length; iProduct++)
	{
		var ProductNode = ProductNodeList[iProduct];
		if (IsProductNeeded(ProductNode))
		{
			var ProductTitle = ProductNode.selectSingleNode("Title").text;
			var Destination = fso.BuildPath(DestinationPath, ProductNode.selectSingleNode('AutoConfigure/Destination').text);
			var FileListData = SourceFileLists[iProduct].ListData;
			for (iData = 0; iData < FileListData.length; iData++)
			{
				var FileList = FileListData[iData].FileList;
				var RootFolder = FileListData[iData].RootFolder;
				var Substitution = FileListData[iData].NameWhenLocked;
				var DestPath = FileListData[iData].DestPath;
				var SignAs = FileListData[iData].SignAs;

				for (i = 0; i < FileList.length; i++)
				{
					var SourcePath = FileList[i];
					var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, Destination);
					if (DestPath)
						TargetFullPath = fso.BuildPath(TargetFullPath, DestPath);
					MakeSureFolderExists(TargetFullPath);

					var UsingSub = false;
					if (IsProductLocked(ProductNode))
						if (Substitution)
							UsingSub = true;

					if (UsingSub)
						TargetFullPath = fso.BuildPath(TargetFullPath, Substitution);
					else
						TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

					if (TestCopy(FileList[i], TargetFullPath))
					{
						FilesCopied++;

						if (SignAs != null)
						{
							// Sign the current file:
							var Cmd = '"' + fso.BuildPath(UtilsPath, "SignMaster.exe") + '" "' + TargetFullPath + '" -d "' + SignAs + '" -p "' + CertificatePassword + '"';
							Report("Running command: " + Cmd.replace(CertificatePassword, "********"));
							shellObj.Run(Cmd, 1, true);
						}
					}
				} // Next file
			} // Next source node
			
			// If the Product is not locked, copy the setup help file:
			if (!IsProductLocked(ProductNode))
			{
				var SetupHelpNode = ProductNode.selectSingleNode("AutoConfigure/SetupHelp");
				var SetupHelpTargetNode = ProductNode.selectSingleNode('Help');
				if (SetupHelpNode && SetupHelpTargetNode)
				{
					// Check if source path is relative:
					var HelpSource = CheckProductRelativePath(SetupHelpNode.text, ProductsPath);
					if (TestCopy(HelpSource, fso.BuildPath(DestinationPath, SetupHelpTargetNode.text)))
						FilesCopied++;
				}
			}
		}
	} // Next product
	Report(FilesCopied + " files gathered.");

	if (OriginalFileList.length > 0)
	{
		Report("The following " + OriginalFileList.length + " were already in the package folder, but appear to be no longer needed:");
		var i;
		for (i = 0; i < OriginalFileList.length; i++)
			Report(OriginalFileList[i]);
	}

	return FilesCopied;
}

// Returns true if the specified product is needed in the specified flavor.
function IsProductNeeded(ProductNode)
{
	var CdNode = ProductNode.selectSingleNode("CD");
	if (CdNode == null)
		return false;
	if (CdNode.text == "-1")
		return false;

	return true;
}

// Returns true if any needed product is locked.
function IsAnyProductLocked()
{
	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');

	for (i = 0; i < ProductNodeList.length; i++)
		if (ProductNodeList[i].getAttribute("KeyId") != null && ProductNodeList[i].getAttribute("KeyId") != "")
			return true;
	return false;
}

// Returns true if any needed product is locked.
function IsProductLocked(ProductNode)
{
	if (ProductNode.getAttribute("KeyId") != null && ProductNode.getAttribute("KeyId") != "")
		return true;
	return false;
}

// Returns a full path to a file's destination, based on the file's source,
// the root folder at the time the source was found, and the intended root folder
// of the destination.
function GetDestinationFolder(SourcePath, RootFolder, TargetRoot)
{
	// Make sure the target path reflects any folders that may have been recursed:
	var TargetFullPath = TargetRoot;
	// Add to TargetFullPath any folder path between the RootFolder and the actual file name:
	if (SourcePath.slice(0, RootFolder.length) != RootFolder)
		Report("Error: SourcePath " + SourcePath + " does not contain expected root " + RootFolder);
	var SectionStart = RootFolder.length;
	var SectionEnd = SourcePath.lastIndexOf("\\")
	var ExtraFolders = SourcePath.slice(SectionStart, SectionEnd);
	TargetFullPath = fso.BuildPath(TargetFullPath, ExtraFolders);

	return TargetFullPath;
}

// Copies over the Source file to the Dest path, unless the file is already
// there (with the same size and modification date).
// Returns true if the copy was at least attempted because it was needed.
function TestCopy(Source, Dest)
{
	if (!fso.FileExists(Source))
	{
		Report("Error: file " + Source + " does not exist.");
		return false;
	}
	if (fso.FileExists(Dest))
	{
		// Remove from OriginalFileList:
		RemoveFileFromList(Dest);

		var fileDest = fso.GetFile(Dest);
		var fileSource = fso.GetFile(Source);

		var fileDestDateLastModified = fileDest.DateLastModified;
		var fileSourceDateLastModified = fileSource.DateLastModified;

		// The following contains a rather awkward <= and >= comparison, because the == comparison did not work.
		// In fact, the comparison "fileDestDateLastModified == fileDestDateLastModified" yields "false", a truly remarkable feat of logic!
		if (fileDest.Size == fileSource.Size && fileDestDateLastModified <= fileSourceDateLastModified && fileDestDateLastModified >= fileSourceDateLastModified)
			return false;

		Report(Dest + " has changed - copying from " + Source + " (size is " + fileDest.Size + " [" + fileSource.Size + "], date is " + fileDestDateLastModified + " [" + fileSourceDateLastModified + "])");
	}
	else
		Report(Dest + " does not exist - copying from " + Source);

	// For some unknown reason, the fso.CopyFile() method sometimes fails
	// with a "permission denied" error. This method uses DOS to do the file
	// copy instead:
	var Cmd = 'cmd /Q /D /C copy "' + Source + '" "' + Dest + '"';
	var shellObj = new ActiveXObject("WScript.Shell");
	Report("Running DOS command: " + Cmd);
	shellObj.Run(Cmd, 0, true);

	if (!fso.FileExists(Dest))
		Report("Error - copy has failed.");

	return true;
}

// Removes the given file path from the list of original files.
function RemoveFileFromList(path)
{
	var i;
	for (i = 0; i < OriginalFileList.length; i++)
	{
		if (path == OriginalFileList[i])
		{
			OriginalFileList.splice(i, 1);
			break;
		}
	}
}

// Uses the 7-zip tool to make a self-extracting archive of the specified folder.
function Compress()
{
	if (SevenZipExeFile == null)
	{
		Report("Error: 7-zip utility not found...");
		return;
	}

	var ZipFile = FlavorFolder + ".7z";
	var iFirstBackslash = FlavorFolder.indexOf("\\");
	var LocationDrive = FlavorFolder.slice(0, iFirstBackslash);

	var BatchFile = FlavorFolder + ".temp.bat";
	var ConfigFile = FlavorFolder + ".Config.txt";
	var ListFile = FlavorFolder + ".list.txt";

	f7ZipFoundSvn = false;
	// Get list of all files to be used in archive:
	var FileListObject = GetFileList(FlavorFolder, true);
	var FileList = FileListObject.FileList;

	// Initiate creation of 7-zip batch file:
	Report("Building batch file " + BatchFile);
	var tso = fso.OpenTextFile(BatchFile, 2, true);
	var BatchLine = "@echo off"
	tso.WriteLine(BatchLine);
	Report(BatchLine);
	tso.WriteLine(LocationDrive);
	Report(LocationDrive);
	BatchLine = 'cd "' + RootFolder + '"'
	tso.WriteLine(BatchLine);
	Report(BatchLine);

	// If we found any .svn folders, we have to use a list file, adding individual
	// files to the list one by one.
	// If we didn't find and .svn folders, we can add the whole lot in one go, without
	// using a list file:
	if (f7ZipFoundSvn)
	{
		Report("    Building list file " + ListFile + ":");
		// Add files:
		for (i = 0; i < FileList.length; i++)
		{
			// Check that current file path begins with RootFolder and FlavorName, then remove path before FlavorName:
			var path = FileList[i];
			var rLen = RootFolder.length;
			if (path.slice(0, rLen) == RootFolder && path.slice(rLen, rLen + 1 + FlavorName.length) == "\\" + FlavorName)
				path = path.slice(rLen + 1);

			var ListFileLine = '"' + path + '"';
			Report("    " + ListFileLine);
			var tsoList = fso.OpenTextFile(ListFile, 8, true);
			tsoList.WriteLine(ListFileLine);
			tsoList.Close();
		}
		BatchLine = '"' + SevenZipExeFile + '" a "' + ZipFile + '" + @"' + ListFile + '" -mx=9 -mmt=on'
		tso.WriteLine(BatchLine);
		Report(BatchLine);
	}
	else
	{
		BatchLine = '"' + SevenZipExeFile + '" a "' + ZipFile + '" + "' + FlavorName + '\\*" -r -mx=9 -mmt=on'
		tso.WriteLine(BatchLine);
		Report(BatchLine);
	}

	// Add self-extracting module and configuration to launch setup.exe:
	BatchLine = 'copy /b "' + fso.BuildPath(UtilsPath, "7zSD_new.sfx") + '" + "' + ConfigFile + '" + "' + ZipFile + '" "' + SfxFile + '"'
	tso.WriteLine(BatchLine);
	Report(BatchLine);
	tso.Close();

	// Run the temporary batch file we have been building:
	Cmd = 'cmd /D /C ""' + BatchFile + '""';
	Report("Running DOS command: " + Cmd);
	shellObj.Run(Cmd, 1, true);
	// Delete the batch and list files:
	fso.DeleteFile(BatchFile);
	if (fso.FileExists(ListFile))
		fso.DeleteFile(ListFile);

	// Attempt to sign the SfxFile file:
	var Cmd = '"' + fso.BuildPath(UtilsPath, "SignMaster.exe") + '" "' + SfxFile + '" -d "SIL Software Package" -p "' + CertificatePassword + '"';
	Report("Running command: " + Cmd.replace(CertificatePassword, "********"));
	shellObj.Run(Cmd, 1, true);

	// If the md5sums utility exists where we expect, use it to generate an md5 hash
	// of the new archive file:
	var Md5SumsPath = fso.BuildPath(UtilsPath, "md5sums.exe");
	if (fso.FileExists(Md5SumsPath))
	{
		var md5FilePath = FlavorFolder + '.md5.txt';

		// Make new batch file:
		var tso = fso.OpenTextFile(BatchFile, 2, true);
		tso.WriteLine("@echo off");
		tso.WriteLine(LocationDrive);
		tso.WriteLine('cd "' + RootFolder + '"');
		tso.WriteLine('"' + Md5SumsPath + '" -u "' + SfxFile + '" >"' + md5FilePath + '"');
		tso.Close();

		// Run the temporary batch file we have been building:
		Cmd = 'cmd /D /C ""' + BatchFile + '""';
		Report("Running DOS command: " + Cmd);
		shellObj.Run(Cmd, 1, true);
		// Delete the batch and list files:
		fso.DeleteFile(BatchFile);

		// Get the raw md5 value from among the other junk in the output of md5sums:
		var tso = fso.OpenTextFile(md5FilePath, 1, true);
		var Line = tso.ReadLine();
		tso.Close();
		var IndexOfSpace = Line.indexOf(' ');
		var MD5 = Line.slice(0, IndexOfSpace);

		// Write the raw md5 value into the outptut file:
		var tso = fso.OpenTextFile(md5FilePath, 2, true);
		tso.WriteLine(MD5);
		tso.Close();
	}
}

function Report(msg)
{
	var d = new Date();
	tsoLog.WriteLine(d.toString() + " - " + msg);
}