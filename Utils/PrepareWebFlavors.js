// JScript to create single XML file for all flavors of a particular web download.
// Called with one parameter: The full path to a folder which contains all the flavors
// of distributions, one per subfolder, each subfolder containing a master installer
// XML file. This tool creates a file "downloads.xml" in the given base directory, which
// contains only the necessary information for the Test Presence activeX control system.
// If the parameter is missing, then this script merely sets up the registry
// creating a shell extension, so that a folder can be right-clicked on to create the
// aforementioned XML file.

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
	var RegFile = fso.BuildPath(Path, "PrepareWebFlavors.reg");
	var tso = fso.OpenTextFile(RegFile, 2, true);
	tso.WriteLine('Windows Registry Editor Version 5.00');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\PrepareWebFlavors]');
	tso.WriteLine('"EditFlags"=hex:01,00,00,00');
	tso.WriteLine('@="Prepare web distribution flavors XML file"');
	tso.WriteLine('[HKEY_CLASSES_ROOT\\Folder\\shell\\PrepareWebFlavors\\command]');
	// OK, deep breath, now:
	tso.WriteLine('@="C:\\\\windows\\\\system32\\\\wscript.exe \\"' + bs2Path + '\\\\PrepareWebFlavors.js\\" \\"%1\\"\"');
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

// Iterate over folders, collecting valid XML files:
var XmlDocs = new Array();
var BaseFolder = fso.GetFolder(SourceFolder);
var SubFolders = new Enumerator(BaseFolder.SubFolders);

for (; !SubFolders.atEnd(); SubFolders.moveNext())
{
	TestFolderForValidXmlFiles(SubFolders.item().Path);
}

// Sort the XML files, by number of missing products:
XmlDocs.sort(sortRank)

// Open new file for output doc:
var NewPath = fso.BuildPath(SourceFolder, "downloads.xml");
var tso = fso.OpenTextFile(NewPath, 2, true, -1);
tso.WriteLine('<?xml version="1.0" encoding="utf-16"?>');
tso.WriteLine('<?xml-stylesheet type="text/xsl" href="TestDownloads.xsl" ?>');
tso.WriteLine('<Downloads>');

// Process each XML file, and write the results into the big new XML file:
for (i = 0; i < XmlDocs.length; i++)
{
	tso.WriteLine('<Download Flavor="' + XmlDocs[i].Flavor + '" Rank="' + (i+1) + '">');

	// Make a copy of the original XML document:
	var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
	xmlDoc.async = false;
	XmlDocs[i].Doc.save(xmlDoc);

	// Remove all the junk we don't need:
	RemoveNodes("/MasterInstaller/AutoConfigure");
	RemoveNodes("/MasterInstaller/General");
	Products = xmlDoc.selectNodes("/MasterInstaller/Products/Product");
	for (p = 0; p < Products.length; p++)
	{
		Products[p].removeAttribute("KeyId");
		Products[p].removeAttribute("Version");
		// Remove any comments:
		var ProductChildNodes = Products[p].childNodes;
		for (c = 0; c < ProductChildNodes.length; c++)
		{
			if (ProductChildNodes[c].nodeType == 8) // NODE_COMMENT
			{
				Products[p].removeChild(ProductChildNodes[c]);
				c--;
			}
		}			
	}
	RemoveNodes("/MasterInstaller/Products/Product/AutoConfigure");
	RemoveNodes("/MasterInstaller/Products/Product/CriticalFile");
	RemoveNodes("/MasterInstaller/Products/Product/Install");
	RemoveNodes("/MasterInstaller/Products/Product/StatusWindow");
	RemoveNodes("/MasterInstaller/Products/Product/Help");
	RemoveNodes("/MasterInstaller/Products/Product/Commentary");
	RemoveNodes("/MasterInstaller/Products/Product/KillHangingWindows");
	RemoveNodes("/MasterInstaller/Products/Product/Preinstall");
	RemoveNodes("/MasterInstaller/Products/Product/PostInstall");
	RemoveNodes("/MasterInstaller/Products/Product/FlushReboot");
	RemoveNodes("/MasterInstaller/Products/Product/MustNotDelayReboot");
	RemoveNodes("/MasterInstaller/Products/Product/RebootTestRegPending");
	RemoveNodes("/MasterInstaller/Products/Product/RebootTestWininit");
	RemoveNodes("/MasterInstaller/Products/Product/Feature/Title");
	RemoveNodes("/MasterInstaller/Products/Product/Feature/Tag");
	RemoveNodes("/MasterInstaller/Products/Product/Feature/MsiFeature");
	
	var XmlPortion = xmlDoc.xml;
	var iFirstNewline = XmlPortion.search("\n");
	if (iFirstNewline)
	{
		var XmlPortion2 = XmlPortion.slice(iFirstNewline + 1);
		var iSecondNewline = XmlPortion2.search("\n");
		if (iSecondNewline)
		{
			XmlPortion = XmlPortion2.slice(iSecondNewline + 1);
		}
	}
	tso.Write(XmlPortion);
	tso.WriteLine('</Download>');
}
tso.WriteLine('</Downloads>');
tso.Close();


// Searches given folder for XML files, and tests to see if they contain the <MasterInstaller>
// node. If so, the XML document is loaded and added to the XmlDocs array.
function TestFolderForValidXmlFiles(FolderSpec)
{
	var Folder = fso.GetFolder(FolderSpec);
	var Files = new Enumerator(Folder.files);
	for (; !Files.atEnd(); Files.moveNext())
	{
		var CurrentFile = Files.item().Path;

		// Check if the CurrentFile is an XML file:
		if (CurrentFile.slice(-4).toLowerCase() == ".xml")
		{
			// Open the XML file and check for the <MasterInstaller> node:
			var xmlDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
			xmlDoc.async = false;
			xmlDoc.load(CurrentFile);
			if (xmlDoc.parseError.errorCode != 0)
			{
				var myErr = xmlDoc.parseError;
				Exception = new Object();
				Exception.description = "XML error in " + CurrentFile + ": " + myErr.reason + "\non line " + myErr.line + " at position " + myErr.linepos;
				throw(Exception);
			}
			if (xmlDoc.selectSingleNode("/MasterInstaller") != null)
			{
				var XmlData = new Object();
				XmlData.Doc = xmlDoc;
				XmlData.Path = CurrentFile;
				// For name of flavor, use folder name:
				var iLastBackslash = FolderSpec.lastIndexOf("\\");
				XmlData.Flavor = FolderSpec.slice(iLastBackslash + 1);
				// Record number of missing products - where CD index is -1:
				var CdNodes = xmlDoc.selectNodes("/MasterInstaller/Products/Product/CD");
				var TotalMissing = 0;
				for (i = 0; i < CdNodes.length; i++)
					if (CdNodes[i].text == "-1")
						TotalMissing++;
				XmlData.TotalMissing = TotalMissing;
				XmlDocs.push(XmlData);
			}
		}
	}
}

// The function passed to the sort() method on our XmlDocs array.
function sortRank(a, b)
{
	var x = a.TotalMissing;
	var y = b.TotalMissing;
	return((x < y) ? -1 : ((x > y) ? 1 : 0));
}

// Removes all nodes passed after a call to selectNodes().
function RemoveNodes(XPath)
{
	Nodes = xmlDoc.selectNodes(XPath);
	while (Nodes.peekNode() != null)
		Nodes.removeNext();
}
