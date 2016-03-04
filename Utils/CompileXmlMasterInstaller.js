// This script takes an XML file, and compiles/links it into a setup.exe Master Installer file.
// Prerequisite: a suitable version of Microsoft Visual Studio must be installed. (VS 2005 and 2008 work.)


//var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");

//if (MasterInstallerPath == "%MASTER_INSTALLER%")
//{
//	WScript.Echo("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
//	WScript.Quit();
//}
//var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");

//var CppFilePath = fso.BuildPath(MasterInstallerPath, "Code and Projects");
//var BitmapsPath = fso.BuildPath(CppFilePath, "Bitmaps");

// Write new C++ files to reflect this new configuration:
// PrepareCppFiles(CppFilePath, xmlDoc);

// Prepare the file containing all the compilation settings:
var CppRspFilePath = NewCompilationFolder + "\\" + "Cpp.rsp";
PrepareCppRspFile(CppRspFilePath, CppFilePath, NewCompilationFolder);

// Compile all C++ files:
var CompileCmd = 'cl.exe @"' + CppRspFilePath + '" /nologo';
shellObj.Run(CompileCmd, 7, true);

// Compile resource file:
var ResourceCmd = 'rc.exe /i"' + BitmapsPath + '" ' + (EasterEggs ? '/D "EASTER_EGGS" ' : '') + '/fo"' + NewCompilationFolder + '/resources.res" "' + CppFilePath + '\\resources.rc"';
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

// Embed the manifest file specifying "requireAdministrator":
var numRetries = 5;
while (numRetries > 0)
{
	var MtCmd = 'mt.exe -manifest "' + fso.BuildPath(CppFilePath, 'setup.manifest') + '" -outputresource:"' + SetupExePath + '";#1';
	var ret = shellObj.Run(MtCmd, 7, true);
	if (ret == 31)
	{
		numRetries--;
		if (numRetries == 0)
		{
			WScript.Echo("mt.exe found " + SetupExePath + " was locked. Trying again. If problem persists, switch off anti-virus real-time file checking.");
			numRetries = 5;
		}
	}
	else
		numRetries = 0;
}

// Attempt to sign the .exe file:
var Cmd;
if (CertificatePassword == null)
	Cmd = '"' + fso.BuildPath(UtilsPath, "SignMaster.exe") + '" "' + SetupExePath + '" -d "SIL Software Installer"';
else
	Cmd = '"' + fso.BuildPath(UtilsPath, "SignMaster.exe") + '" "' + SetupExePath + '" -d "SIL Software Installer" -p "' + CertificatePassword + '"';
shellObj.Run(Cmd, 1, true);

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



// Processes the given xmlDoc to generate the C++ files needed to configure the Setup.exe program.
// function PrepareCppFiles(CppFilePath, xmlDoc)
// {
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGeneral.xsl", CppFilePath + "\\ConfigGeneral.cpp");
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigDisks.xsl", CppFilePath + "\\ConfigDisks.cpp");
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigProducts.xsl", CppFilePath + "\\ConfigProducts.cpp");
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigFunctions.xsl", CppFilePath + "\\ConfigFunctions.cpp");
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGlobals.xsl", CppFilePath + "\\AutoGlobals.h");
	// ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigResources.xsl", CppFilePath + "\\AutoResources.rc");
// }

// Uses the given XSL file (path) to process the given xmlDoc, outputting to strOutputFile (path).
// function ProcessConfigFile(xmlDoc, strInputXsl, strOutputFile)
// {
	// var xslt = new ActiveXObject("Msxml2.XSLTemplate.3.0");
	// var xslDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
	// var xslProc;
	// xslDoc.async = false;
	// xslDoc.load(strInputXsl);
	// if (xslDoc.parseError.errorCode != 0)
	// {
		// var myErr = xslDoc.parseError;
		// Exception = new Object();
		// Exception.description = "XSL error in " + strInputXsl + ": " + myErr.reason + "\non line " + myErr.line + " at position " + myErr.linepos;
		// throw (Exception);
	// }
	// xslt.stylesheet = xslDoc;
	// xslProc = xslt.createProcessor();
	// xslProc.input = xmlDoc;
	// xslProc.transform();
	// var fso = new ActiveXObject("Scripting.FileSystemObject");
	// var tso = fso.OpenTextFile(strOutputFile, 2, true);
	// tso.Write(xslProc.output);
	// tso.Close();
// }

// Prepares a file of Setup.exe configuration settings for the C++ compiler.
function PrepareCppRspFile(RspFilePath, CppFilePath, CompilationFolder)
{
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var tso = fso.OpenTextFile(RspFilePath, 2, true);
	tso.WriteLine('/O1 /Ob1 /Os /Oy /GL /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" ' + (EasterEggs ? '/D "EASTER_EGGS" ' : '') + '/GF /EHsc /MT /GS- /Gy /Fo"' + CompilationFolder + '\\\\" /Fd"' + CompilationFolder + '\\vc80.pdb" /W3 /c /Zi /TP');
	tso.WriteLine('"' + CppFilePath + '\\UsefulStuff.cpp"');
	tso.WriteLine('"' + CppFilePath + '\\UniversalFixes.cpp"');
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
	tso.WriteLine('/OUT:"' + SetupExePath + '" /INCREMENTAL:NO /NOLOGO /LIBPATH:"Msi.lib" /MANIFEST /MANIFESTFILE:"' + CompilationFolder + '\\setup.exe.intermediate.manifest" /SUBSYSTEM:WINDOWS /SWAPRUN:CD /OPT:REF /OPT:ICF /LTCG /MACHINE:X86 version.lib shlwapi.lib msi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib');
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
	tso.WriteLine('"' + CompilationFolder + '\\UsefulStuff.obj"');
	tso.WriteLine('"' + CompilationFolder + '\\UniversalFixes.obj"');
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
	catch (err)
	{
	}
}
