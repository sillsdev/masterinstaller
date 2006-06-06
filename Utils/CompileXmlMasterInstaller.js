// This script takes an XML file, and compiles/links it into a setup.exe Master Installer file.

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

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
	var RegFile = fso.BuildPath(Path, "CompileXmlMasterInstaller.reg");
	var tso = fso.OpenTextFile(RegFile, 2, true);
	tso.WriteLine('Windows Registry Editor Version 5.00');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\xmlfile\\shell\\CompileMasterInstaller]');
	tso.WriteLine('@="Compile Master Installer from this XML file"');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\xmlfile\\shell\\CompileMasterInstaller\\command]');
	// OK, deep breath, now:
	tso.WriteLine('@="C:\\\\windows\\\\system32\\\\wscript.exe \\"' + bs2Path + '\\\\CompileXmlMasterInstaller.js\\" \\"%1\\"\"');
	tso.Close();
	
	// Run Regedit with the new file:
	var Cmd = 'Regedit.exe "' + RegFile + '"';
	shellObj.Run(Cmd, 0, true);
	
	// Delete RegFile:
	fso.DeleteFile(RegFile);

	WScript.Quit();
}

var XmlFileName = WScript.Arguments.Item(0);
var CppFilePath = "E:\\CD Builder\\Master Installer";

// Check that the inout file is an XML file:
if (XmlFileName.slice(-4).toLowerCase() != ".xml")
{
	WScript.Echo("ERROR - XML file must be specified.");
	WScript.Quit();
}

// Check that the XML file has a <MasterInstaller> node:
var xmlDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
xmlDoc.async = false;
xmlDoc.load(XmlFileName);
if (xmlDoc.parseError.errorCode != 0)
{
	var myErr = xmlDoc.parseError;
	WScript.Echo("You have an XML error " + myErr.reason + " on line " + myErr.line + " at position " + myErr.linepos);
	WScript.Quit();
}
if (xmlDoc.selectSingleNode("/MasterInstaller") == null)
{
	WScript.Echo("ERROR - " + XmlFileName + " is not a Master Installer configuration file." + xmlDoc.xml);
	WScript.Quit();
}

iLastBackslash = XmlFileName.lastIndexOf("\\");
var InputFolder = XmlFileName.substr(0, iLastBackslash);
var NewCompilationFolder = fso.BuildPath(InputFolder, "setup.exe coming soon");
MakeSureFolderExists(NewCompilationFolder);

// Write new C++ files to reflect this new configuration:
PrepareCppFiles(CppFilePath, xmlDoc);

// Prepare the file containing all the compilation settings:
var CppRspFilePath = NewCompilationFolder + "\\" + "Cpp.rsp";
PrepareCppRspFile(CppRspFilePath, CppFilePath, NewCompilationFolder);

// Compile all C++ files:
var CompileCmd = 'cl.exe @"' + CppRspFilePath + '" /nologo';
shellObj.Run(CompileCmd, 7, true);

// Compile resource file:
var ResourceCmd = 'rc.exe /fo"' + NewCompilationFolder + '/resources.res" "' + CppFilePath + '\\resources.rc"';
shellObj.Run(ResourceCmd, 7, true);

// Prepare the file containing all the linker settings:
var ObjRspFilePath = NewCompilationFolder + "\\" + "Obj.rsp";
var SetupExePath = InputFolder + "\\setup.exe";
PrepareObjRspFile(ObjRspFilePath, NewCompilationFolder);

// Link the obj files to produce the master installer:
var LinkCmd = 'link.exe @"' + ObjRspFilePath + '"'; 
shellObj.Run(LinkCmd, 7, true);

// Test that setup.exe exists:
if (!fso.FileExists(SetupExePath))
{
	WScript.Echo("ERROR - Master installer [setup.exe] failed to compile and link.");
	WScript.Quit();
}

// Remove junk from the NewCompilationFolder:			
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.obj"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.res"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.rsp"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.pdb"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.idb"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.lib"));
DeleteIfExists(fso.BuildPath(NewCompilationFolder, "*.exp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "ConfigProducts.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "ConfigGeneral.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "ConfigFunctions.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "ConfigDisks.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "AutoResources.rc"));
DeleteIfExists(fso.BuildPath(CppFilePath, "AutoGlobals.h"));
DeleteIfExists(fso.BuildPath(CppFilePath, "Helps.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "FileList.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "Product?.cpp"));
DeleteIfExists(fso.BuildPath(CppFilePath, "Product??.cpp"));
fso.DeleteFolder(NewCompilationFolder);


// Create the specified folder path, if it doesn't already exist.
// Thanks, Jeff!
//	Note - Does not handle \\LS-ELMER\ type directory creation.
function MakeSureFolderExists(strDir)
{
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	
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

// Processes the given xmlDoc to generate the C++ files needed to configure the Setup.exe program.
function PrepareCppFiles(CppFilePath, xmlDoc)
{
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGeneral.xsl", CppFilePath + "\\ConfigGeneral.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigDisks.xsl", CppFilePath + "\\ConfigDisks.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigProducts.xsl", CppFilePath + "\\ConfigProducts.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigFunctions.xsl", CppFilePath + "\\ConfigFunctions.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGlobals.xsl", CppFilePath + "\\AutoGlobals.h");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigResources.xsl", CppFilePath + "\\AutoResources.rc");
}

// Uses the given XSL file (path) to process the given xmlDoc, outputting to strOutputFile (path).
function ProcessConfigFile(xmlDoc, strInputXsl, strOutputFile)
{
	var xslt = new ActiveXObject("Msxml2.XSLTemplate.3.0");
	var xslDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
	var xslProc;
	xslDoc.async = false;
	xslDoc.load(strInputXsl);
	if (xslDoc.parseError.errorCode != 0)
	{
		var myErr = xslDoc.parseError;
		Exception = new Object();
		Exception.description = "XSL error in " + strInputXsl + ": " + myErr.reason + "\non line " + myErr.line + " at position " + myErr.linepos;
		throw(Exception);
	}
	xslt.stylesheet = xslDoc;
	xslProc = xslt.createProcessor();
	xslProc.input = xmlDoc;
	xslProc.transform();
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var tso = fso.OpenTextFile(strOutputFile, 2, true);
	tso.Write(xslProc.output);
	tso.Close();
}

// Prepares a file of Setup.exe configuration settings for the C++ compiler.
function PrepareCppRspFile(RspFilePath, CppFilePath, CompilationFolder)
{
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var tso = fso.OpenTextFile(RspFilePath, 2, true);
	tso.WriteLine('/O1 /Ob1 /Os /Oy /GL /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /GF /EHsc /MT /GS- /Gy /Fo"' + CompilationFolder + '\\\\" /Fd"' + CompilationFolder + '\\vc80.pdb" /W3 /c /Wp64 /Zi /TP');
	tso.WriteLine('"' + CppFilePath + '\\WIWrapper.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\UsefulStuff.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\ThirdPartySoftware.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\ProductManager.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\ProductKeys.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\PersistantProgress.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\main.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\LogFile.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\Globals.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\ErrorHandler.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\DiskManager.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\Dialogs.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\Control.cpp"');
	tso.Close();
}

// Prepares a file of Setup.exe configuration settings for the object file linker.
function PrepareObjRspFile(RspFilePath, CompilationFolder)
{
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var tso = fso.OpenTextFile(RspFilePath, 2, true);
	tso.WriteLine('/OUT:"' + SetupExePath + '" /INCREMENTAL:NO /NOLOGO /LIBPATH:"Msi.lib" /MANIFEST /MANIFESTFILE:"' + CompilationFolder + '\\setup.exe.intermediate.manifest" /SUBSYSTEM:WINDOWS /SWAPRUN:CD /OPT:REF /OPT:ICF /OPT:NOWIN98 /LTCG /MACHINE:X86 version.lib shlwapi.lib C:\\Work\\MsiIntel.SDK\\Lib\\Msi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib');
	tso.WriteLine('"' + CompilationFolder + '\\Control.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\Dialogs.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\DiskManager.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\ErrorHandler.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\Globals.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\LogFile.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\main.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\PersistantProgress.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\ProductKeys.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\ProductManager.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\ThirdPartySoftware.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\UsefulStuff.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\WIWrapper.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\resources.res"');
	tso.Close();
}

// Deletes the specified file(s), if they exist. Doesn't complain if they don't.
// FilePath can contain wildcards for the filename.
function DeleteIfExists(FilePath)
{
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	try
	{
		fso.DeleteFile(FilePath);
	}
	catch(err)
	{
	}
}
