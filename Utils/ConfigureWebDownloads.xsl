<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
  xmlns:html="http://www.w3.org/1999/xhtml"
  xmlns:dt="urn:schemas-microsoft-com:datatypes">
	<xsl:output method="html" />

	<!-- =============================================== -->
	<!-- Root Template                                   -->
	<!-- =============================================== -->
	<xsl:template match="/">
		<html>
			<head>
				<title>Web Downloads Configuration Tool</title>
				<xsl:call-template name="script"/>
			</head>
			<body onload="Initialize();" bgcolor="#FEDCBA">
				<span style="font-size:250%">
					<b>Web Downloads Configuration </b>
				</span><span style="font-size:80%">
					by <a href="mailto:alistair_imrie@sil.org?subject=Web Downloads Auto-configuration Tool">Alistair Imrie</a>
				</span>
				<br/>&#169; 2006 - 2009 <a href="http://www.sil.org">SIL International</a><br/><br/>
				<button id="PrevButton" onclick='PrevStage()' disabled="true">&lt; Previous</button>
				<button id="NextButton" onclick='NextStage()' title='On to General Configuration'>&#160;&#160;&#160;&#160;Next &gt;&#160;&#160;</button>
				<div id="BlockedContentWarning" style="position:static; visibility:visible">
					<h2>
						Please make sure that blocked content <u>and</u> active scripts are enabled in your browser.
					</h2>
				</div>
				<div id="Stage1StillInitializing" style="position:absolute; visibility:hidden">
					<h2>Initializing - please wait...</h2>
				</div>
				<div id="Stage1" style="position:absolute; visibility:hidden">
					<h2>Flavors Setup</h2>
					<xsl:call-template name="CollectFlavors"/>
				</div>
				<div id="Stage2" style="position:absolute; visibility:hidden">
					<h2>Flavor Configurations</h2>
					<xsl:apply-templates select="/MasterInstaller/Products"/>
				</div>
				<div id="Stage3" style="position:absolute; visibility:hidden">
					<h2>Ready to go...</h2>
					Select the tasks you want to be performed, then press the Go button.<br/><br/>
					<table>
						<tr>
							<td align="right">Path of C++ source files:</td>
							<td>
								<input id="CppPath" type="text" size="60" onfocus="this.select();" title="Folder containing C++ files for compilation into master installer setup.exe"/>
								<script type="text/javascript">document.getElementById("CppPath").value=CppFilePath;</script>
							</td>
						</tr>
						<tr>
							<td align="right">Path of Products:</td>
							<td>
								<input id="ProductsPath" type="text" size="60" onfocus="this.select();" title="Source folder containing all products for the web downloads"/>
								<script type="text/javascript">document.getElementById("ProductsPath").value=ProductsPath;</script>
							</td>
						</tr>
						<tr>
							<td alight="right">Path of root folder for all output:</td>
							<td>
								<input id="OutputPath" type="text" size="60" onfocus="this.select();" title="Root folder for all flavors"/>
							</td>
						</tr>
					</table>
					<input id="WriteXml" type="checkbox" title="Write out an XML configuration file matching your settings for each flavor."/>Write master installer XML file for each flavor<br/>
					<script type="text/javascript">document.getElementById("WriteXml").checked=true;</script>
					<input id="WriteDownloadsXml" type="checkbox" title="Write out an XML configuration file matching your settings for the TestPresence ActiveX control."/>Write downloads.xml file<br/>
					<script type="text/javascript">document.getElementById("WriteDownloadsXml").checked=true;</script>
					<input id="Compile" type="checkbox" title="Compile a setup.exe program for each flavor."/>Compile master installer for each flavor<br/>
					<script type="text/javascript">document.getElementById("Compile").checked=true;</script>
					<input id="SignWithCertificate" type="checkbox" title="Sign each setup.exe with the certificate in the root folder on the CD in the specified drive."/>Sign each setup.exe - certificate location:
					<script type="text/javascript">document.getElementById("SignWithCertificate").checked=true;</script>
					<input id="CdDrive" type="text" onselect="InputTextSelected(this);" size="2" onfocus="this.select();" title="Drive (or folder) containing digital certifiate CD." value="D:"/><br/>
					<input id="GatherFiles" type="checkbox" title="Gather files needed for each flavor into one folder."/>Gather files for each flavor<br/>
					<script type="text/javascript">document.getElementById("GatherFiles").checked=true;</script>
					<input id="BuildSfx" type="checkbox" title="Create self extracting 7-zip archive .exe file for each flavor. Assumes 7-Zip utility is present."/>Build self-extracting download package for each flavor<br/>
					<script type="text/javascript">document.getElementById("BuildSfx").checked=true;</script>
					<br/>
					<br/>
					<button onclick="Go();" style="font-size:130%" title="Start flavor building process">
						<b>&#160;&#160; Go! &#160;&#160;</b>
					</button><br/><br/>
				</div>
				<div id="PanicStoppingDiv" style="position:absolute; visibility:hidden">
					<h2>Stopping - please wait until current action finishes...</h2>
				</div>
				<div id="Stage4" style="position:absolute; visibility:hidden">
					<br/>
					<table>
						<tr>
							<td valign="top">
								<h2>Running...</h2>
							</td>
							<td valign="top">
								<button id="PanicStopButton" onclick="PanicStop();" title="Panic! Stop the build process!">&#160;&#160; Stop! &#160;&#160;</button>
							</td>
						</tr>
					</table>
					Please wait while everthing is built...<br/>
					<table id="CommentaryTable"/>
				</div>
			</body>
		</html>
	</xsl:template>

	<!-- =============================================== -->
	<!-- Flavor Naming Template                  -->
	<!-- =============================================== -->
	<xsl:template name="CollectFlavors">
		<table id="FlavorTable">
			<th></th>
			<th>Flavor Name</th>
			<th>Download URL</th>
			<tr>
				<td>1.</td>
				<td>
					<input id="FlavorName1" type="text" onselect="InputTextSelected(this);" size="40" onfocus="this.select();" title="Name of flavor 1." value="Full Install"/>
				</td>
				<td>
					<input id="FlavorUrl1" type="text" onselect="InputTextSelected(this);" size="40" onfocus="this.select();" title="Download URL of flavor 1." value="http://downloads.sil.org/"/>
				</td>
			</tr>
		</table>
		<button id="AddFlavor" onclick='AddFlavor()'>Add new flavor</button>
		<button id="DeleteLastFlavor" onclick='DeleteLastFlavor()' disabled='true'>Delete last flavor</button>
	</xsl:template>

	<!-- =============================================== -->
	<!-- Flavor Configuration Template                      -->
	<!-- =============================================== -->
	<xsl:template match="/MasterInstaller/Products">
		<table id="FlavorConfigTable" border="1">
			<tr>
				<th>Product</th>
				<th id="FlavorConfig1"></th>
			</tr>
			<xsl:for-each select="Product">
				<tr>
					<td align="right">
						<xsl:value-of select="Title"/>
					</td>
					<td align="center">
						<input id="IncludedF1P{1+count(preceding-sibling::Product)}" type="checkbox">
							<xsl:if test="not(CD='-1')">
								<xsl:attribute name="checked">true</xsl:attribute>
							</xsl:if>
						</input>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:template>

	<!-- =============================================== -->
	<!-- Script Template                                 -->
	<!-- =============================================== -->
	<xsl:template name="script">
		<script type="text/javascript">
			<![CDATA[
	<!--
// This is line 6
var Initializing = true;
var UserPressedNextWhileInitializing = false;
var NumProducts;
var SourceFileLists;
var FileTotal = 0;
var ExternalHelpFileData = null;
var TermsOfUseFileData = null;
var NextButton;
var PrevButton;
var PanicStopPressed = false;
var SelectedTextElement;
var VisibleStage = 1;
var MaxStage = 4;
var NumFlavors = 1;

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

var CppFilePath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
var ProductsPath = shellObj.ExpandEnvironmentStrings("%PACKAGE_PRODUCTS%");

if (CppFilePath == "%MASTER_INSTALLER%")
	alert("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
else if (ProductsPath == "%PACKAGE_PRODUCTS%")
	alert("ERROR: the PACKAGE_PRODUCTS environment variable has not been defined. You cannot create web download packages without specifying where the products and documents are stored.");

var UtilsPath = fso.BuildPath(CppFilePath, "Utils");
var SevenZipExeFile = fso.BuildPath(UtilsPath, "7za.exe");
var BitmapsPath = fso.BuildPath(CppFilePath, "Bitmaps");

var f7ZipFoundSvn = false;

// Called upon completion of page loading.
function Initialize()
{
	try
	{
		NextButton = document.getElementById('NextButton');
		PrevButton = document.getElementById('PrevButton');
		showPage("BlockedContentWarning", false);
		showPage("Stage1", true);

		// Do a test for legacy data that specified a full path to the background bitmap:
		var bmp = document.XMLDocument.selectSingleNode("/MasterInstaller/General/ListBackground").text;
		if (bmp.indexOf("\\") >= 0)
		{
				if (!fso.FileExists(bmp))
				{
						Exception = new Object();
						Exception.description = "Bitmap path '" + bmp + "' specified in XML node /MasterInstaller/General/ListBackground does not exist. This may be legacy data. Bitmaps should be stored in the '" + BitmapsPath + "' folder and referenced by file name only."
						throw(Exception);
				}
		}

		var ProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');
		NumProducts = ProductNodeList.length;
		FileTotal = 0;
		GenerateSourceFileLists(); // Must come after assigning NumProducts and before calling FindDuplicateSets().
		GenerateExtHelpAndTermsFileLists();

		Initializing = false;
		if (UserPressedNextWhileInitializing)
		{
			NextStage();
			showPage("Stage1StillInitializing", false);
			UserPressedNextWhileInitializing = false;
			NextButton.disabled = false;
		}
		
		// Check if we can access the 7-zip tool:
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
				SevenZipExeFile = null;			
			}
		}
	}
	catch(err)
	{
		alert("Error while initializing: " + err.description);
	}
}

// Shows or hides a specified page or subpage of configuration settings.
function showPage(sPage, bShow)
{
	// Kludge to overcome problem where selected text causes a DIV section to not appear:
	if (SelectedTextElement)
	{
		SelectedTextElement.value = SelectedTextElement.value;
		SelectedTextElement = null;
	}		
	var Element = document.getElementById(sPage);
	Element.style.visibility = bShow ? 'visible' : 'hidden';
	Element.style.position = bShow ? 'static' : 'absolute';
}

// Returns to the previous page of configuration settings.
function PrevStage()
{
	if (VisibleStage > 1)
	{
		if (!VerifyPage(VisibleStage))
			return;

		VisibleStage--;

		if (!setPageNo(VisibleStage))
		{
			PrevStage();
			return;
		}
		if (VisibleStage == 1)
		{
			// Disable the Previous button:
			PrevButton.disabled = true;
			PrevButton.title = "";
		}
		else
			PrevButton.title = "Back to " + document.getElementById('Stage' + (VisibleStage - 1)).childNodes[0].innerText;

		// Enable the Next button:
		NextButton.disabled = false;
		NextButton.title = "On to " + document.getElementById('Stage' + (VisibleStage + 1)).childNodes[0].innerText;
	}
}

// Advances to the next page of configuration settings.
function NextStage()
{
	if (VisibleStage < MaxStage - 1 && !UserPressedNextWhileInitializing)
	{
		if (!VerifyPage(VisibleStage))
			return;
	}

	if (Initializing)
	{
		UserPressedNextWhileInitializing = true;
		NextButton.disabled = true;
		showPage("Stage1StillInitializing", true);
		showPage("Stage1", false);
		return;
	}

	if (VisibleStage < MaxStage - 1)
	{
		VisibleStage++;
		
		if (!setPageNo(VisibleStage))
		{
			NextStage();
			return;
		}
		if (VisibleStage == MaxStage - 1)
		{
			// Disable the Next button:
			NextButton.disabled = true;
			NextButton.title = "";
		}
		else
			NextButton.title = "On to " + document.getElementById('Stage' + (VisibleStage + 1)).childNodes[0].innerText;

		// Enable the Previous button:
		PrevButton.disabled = false;
		PrevButton.title = "Back to " + document.getElementById('Stage' + (VisibleStage - 1)).childNodes[0].innerText;
	}
}

// Records last input text element to get text selected.
function InputTextSelected(element)
{
	SelectedTextElement = element;
}

// Finds out which page of configuration settings is shown.
function GetStageNo()
{
	for (i = 1; i <= MaxStage; i++)
	{
		var StageDiv = document.getElementById('Stage' + i);
		if (StageDiv.style.visibility == 'visible')
			return i;
	}
	return 1; // Assume we're on a page related to the first page.
}

// Shows a specified page of configuration settings.
// Returns false if there is a reason not to show the specified page.
function setPageNo(Stage)
{
	// Perform certain housekeeping tasks, depending on which stage we're about to leave:
	switch (GetStageNo())
	{
		case 1:
			// If we're about to leave the Flavors Setup page, update the Flavor Configuration table:
			UpdateFlavorConfigTable();
			//RedrawFlavorConfigTable();
			break;
	}

	// Perform certain housekeeping tasks, depending on which stage we're about to enter:
	switch (Stage)
	{
		case 4:
			// If we're about to enter the Running page, reset the commentary and enable the Stop button:
			ResetCommentaryTable();
			var PanicStopButton = document.getElementById("PanicStopButton");
			PanicStopButton.disabled = false;
			PrevButton.disabled = true;
			break;
	}

	// Deal with main stages:
	for (i = 1; i <= MaxStage; i++)
		showPage('Stage' + i, Stage == i);

	return true;
}

function VerifyPage(CurrentStage)
{
	switch (CurrentStage)
	{
		case 1:
		var Flavor1 = document.getElementById('FlavorName1').value;
		if (Flavor1.length <= 0)
		{
			alert("Error - you must enter a name in the first flavor box.");
			return false;
		}
		if (NumFlavors >= 2)
		{
			for (i = 2; i <= NumFlavors; i++)
			{
				var Flavor = document.getElementById('FlavorName' + i).value;
				if (Flavor.length <= 0)
				{
					alert("Error - you must enter a name for flavor " + i + ".");
					return false;
				}
			}
		}
		break;
		case 3:
			// Check we have something in the root folder spec:
			if (document.getElementById('OutputPath').value.length < 2)
			{
				alert("You must specifiy a root folder for the output.");
				document.getElementById('OutputPath').focus();				
				return false;
			}
		break;
	}
	return true;
}

// Builds an array of file lists for each product, where the files in the list are interpretted
// from the AutoConfigure/Source nodes of each product. (More than one source node per product
// is allowed.)
function GenerateSourceFileLists()
{
	SourceFileLists = new Array();

	var ProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');
	
	// Iterate through all products sources:
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		var FileListData = new Array();
		var AllOk = true;
		try
		{
			var ProductNode = ProductNodeList[iProduct];
			var ProductSourceList = ProductNode.selectNodes('AutoConfigure/Source');
			for (iSource = 0; iSource < ProductSourceList.length; iSource++)
			{
				var ProductSource = ProductSourceList[iSource];
				var NameWhenLocked = ProductSource.getAttribute("NameWhenLocked");
				var Attributes = ProductSource.getAttribute("Attributes");
				var DestPath = ProductSource.getAttribute("DestPath");

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

				var NewListData = GetFileList(SourcePath, Recurse, Attributes);				
				if (NameWhenLocked && NewListData.FileList.length > 1)
					alert("Error - Product " + ProductNode.selectSingleNode('Title').text + " contains an AutoConfigure Source node with a NameWhenLocked attribute (" + NameWhenLocked + ") but multiple files matching.");
				if (NameWhenLocked)
					NewListData.NameWhenLocked = NameWhenLocked;
				NewListData.DestPath = DestPath;
				FileTotal += NewListData.FileList.length;

				FileListData[iSource] = NewListData;
			}
		}
		catch (err)
		{
			AllOk = false;
			alert("Error while initializing: " + err.description);
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
	var ExternalHelpSourceNode = document.XMLDocument.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpSource");
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
	var TermsOfUseSourceNode = document.XMLDocument.selectSingleNode("/MasterInstaller/AutoConfigure/TermsOfUseSource");
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
	{
		Path = fso.BuildPath(RelativePathPrepend, Path);
	}
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
			f7ZipFoundSvn = true;
	}
	File.Close();
	fso.DeleteFile(TempFilePath);

	ReturnObject = new Object();
	ReturnObject.FileList = FileList;
	ReturnObject.RootFolder = RootFolder;
	return ReturnObject;
}

function AddFlavor()
{
	NumFlavors++;
	var Table = document.getElementById("FlavorTable");
	var Cell;
	var Row = Table.insertRow();
	var NewCell = Row.insertCell();
	NewCell.innerText = NumFlavors + ".";
	NewCell = Row.insertCell();
	NewCell.innerHTML = '<input id="FlavorName' + NumFlavors + '" type="text" onselect="InputTextSelected(this);" size="40" onfocus="this.select();" title="Name of flavor ' + NumFlavors + '."/>'
	NewCell.focus();
	NewCell = Row.insertCell();
	NewCell.innerHTML = '<input id="FlavorUrl' + NumFlavors + '" type="text" onselect="InputTextSelected(this);" size="40" onfocus="this.select();" title="Download URL of flavor ' + NumFlavors + '."/>'
	var DeleteButton = document.getElementById("DeleteLastFlavor");
	DeleteButton.disabled = false;
	
	// Add a new column to the Flavor Config table, too:
	Table = document.getElementById("FlavorConfigTable");
	var HeaderRow = Table.rows(0);
	NewCell = HeaderRow.insertCell();
	NewCell.id = "FlavorConfig" + NumFlavors;
	for (i=1; i < Table.rows.length; i++)
	{
		var Row = Table.rows(i);
		NewCell = Row.insertCell();
		NewCell.align = "center";
		NewCell.innerHTML = '<input id="IncludedF' + NumFlavors + 'P' + i + '" type="checkbox">';
		var NewCheckBox = document.getElementById('IncludedF' + NumFlavors + 'P' + i);
		var PreviousCheckBox = document.getElementById('IncludedF' + 1 + 'P' + i);
		NewCheckBox.checked = PreviousCheckBox.checked;
	}
}

function DeleteLastFlavor()
{
	NumFlavors--;
	var Table = document.getElementById("FlavorTable");
	Table.deleteRow(NumFlavors);
	if (NumFlavors <= 1)
	{
		var DeleteButton = document.getElementById("DeleteLastFlavor");
		DeleteButton.disabled = true;
	}
}

// Makes sure the flavor names are updated in the Flavor Configuration table.
function UpdateFlavorConfigTable()
{
	var Table = document.getElementById("FlavorConfigTable");
	var HeaderRow = Table.rows(0);
	for (i=1; i < HeaderRow.cells.length; i++)
	{
		var CurrentFlavor = document.getElementById("FlavorName" + i).value;
		HeaderRow.cells(i).innerHTML = "<b>" + CurrentFlavor + "</b>";
		for (j=1; j < Table.rows.length; j++)
		{
			Element = document.getElementById("IncludedF" + i + "P" + j);
			if (Element)
				Element.title = 'Select to include, clear to omit "' + Table.rows(j).cells(0).innerText + '" from "' + CurrentFlavor + '"';
		}
	}
}

function RedrawFlavorConfigTable()
{
	var Table = document.getElementById("FlavorConfigTable");
}

// Responds to the user pressing the Go! button. Builds a CD image.
function Go()
{
	if (!VerifyPage(3))
		return;
		
	setPageNo(4);
	
	var FinalComment = "All finished successfully.<br/>Don't forget to commit the flavors to the source control repository."; // Optimistic initial value
	
	// Collect user's unstructions:
	var WriteFlavorXml = document.getElementById('WriteXml').checked;
	var WriteDownloadsXml = document.getElementById('WriteDownloadsXml').checked;
	var CompileFlavorXml = document.getElementById('Compile').checked;
	var SignSetupExe = document.getElementById('SignWithCertificate').checked;
	var GatherFlavorFiles = document.getElementById('GatherFiles').checked;
	var BuildFlavor7zip = document.getElementById('BuildSfx').checked;
	
	if (!WriteFlavorXml && !WriteDownloadsXml && !CompileFlavorXml && !GatherFlavorFiles && !BuildFlavor7zip)
		AddCommentary(0, "Nothing to do!", true);
	else
	{
	
		// Everything is surrounded by a try...catch block, so we can trap errors:
		try
		{
			ProductsPath = document.getElementById("ProductsPath").value;
			CppFilePath = document.getElementById("CppPath").value;
			var OutputPath = document.getElementById('OutputPath').value;
			MakeSureFolderExists(OutputPath);
			var XmlDocs = new Array();

			for (flavor = 1; flavor <= NumFlavors; flavor++)
			{				
				var FlavorName = document.getElementById("FlavorName" + flavor).value;
				AddCommentary(0, FlavorName, true);

				// Create folder for current flavor:
				var FlavorFolder = fso.BuildPath(OutputPath, FlavorName);
				MakeSureFolderExists(FlavorFolder);
				
				if (WriteFlavorXml)
				{
					// Copy XML file for current flavor:
					AddCommentary(1, "Writing XML file...", false);
					var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
					xmlDoc.async = false;
					document.XMLDocument.save(xmlDoc);
					if (xmlDoc.parseError.errorCode != 0)
					{
						AddCommentary(1, "Error - copied XML file is invalid:\n" + xmlDoc.parseError.reason + "\non line " + xmlDoc.parseError.line + " at position " + xmlDoc.parseError.linepos, true);
						return;
					}

					// Incorporate adjustments for current flavor into XML copy:
					AdjustMasterInstaller(xmlDoc, flavor);

					// Write XML file for current flavor:
					var FlavorXmlPath = fso.BuildPath(OutputPath, FlavorName + ".xml");
					var tso = fso.OpenTextFile(FlavorXmlPath, 2, true, -1);
					tso.Write(xmlDoc.xml);
					tso.Close();

					// Record data for use in preparing downloads.xml file:
					var XmlData = new Object();
					XmlData.Doc = xmlDoc;
					XmlData.Path = FlavorXmlPath;
					XmlData.Flavor = FlavorName;
					XmlData.FlavorURL = document.getElementById("FlavorUrl" + flavor).value;

					// Record number of missing products - where CD index is -1:
					var CdNodes = xmlDoc.selectNodes("/MasterInstaller/Products/Product/CD");
					var TotalMissing = 0;
					for (i = 0; i < CdNodes.length; i++)
						if (CdNodes[i].text == "-1")
							TotalMissing++;
					XmlData.TotalMissing = TotalMissing;
					XmlDocs.push(XmlData);
				}

				if (CompileFlavorXml)
				{
					// Compile master installer for current flavor:
					AddCommentary(1, "Compiling master installer...", false);
					var SetupExePath = CompileMasterInstaller(FlavorXmlPath);
					var FlavorSetupExePath = fso.BuildPath(FlavorFolder, "setup.exe");

					// Move master installer for current flavor into its proper folder:
					AddCommentary(1, "Moving setup.exe...", false);
					if (fso.FileExists(FlavorSetupExePath))
						fso.DeleteFile(FlavorSetupExePath);
					fso.MoveFile(SetupExePath, FlavorSetupExePath);
				}
				
				if (SignSetupExe)
				{
					var Drive = document.getElementById("CdDrive").value;
					var SignCmd = '"' + CppFilePath + '\\Utils\\Sign\\signcode.exe" -spc "' + Drive + '\\comodo.spc" -v "' + Drive + '\\comodo.pvk" -n "SIL Software Installer" -t http://timestamp.comodoca.com/authenticode -a sha1 "' + FlavorSetupExePath + '"';
					AddCommentary(1, "Signing setup.exe...", false);
					shellObj.Run(SignCmd, 1, true);
				}

				if (GatherFlavorFiles)
				{
					// Gather all the files needed for this flavor:
					AddCommentary(1, "Copying files...", false);
					GatherFiles(flavor, FlavorFolder);
				}
				
				if (BuildFlavor7zip)
				{
					// Compress the files for the current flavor:
					AddCommentary(1, "Compressing files...", false);
					Compress(FlavorFolder);
				}
				
				AddCommentary(1, "Done.", false);
			} // Next flavor
			
			if (WriteDownloadsXml)
			{
				// Generate the downloads.xml file for all the flavors:
				GenerateDownloadsXml(XmlDocs, OutputPath);				
			}
		}
		catch(err)
		{
			FinalComment = "Error: " + (err.number & 0xFFFF) + "\n" + err.description;
			ReturnValue = 1;
		}
	}

	AddCommentary(0, "<h2>" + FinalComment + "</h2>", true, true);
	var PanicStopButton = document.getElementById("PanicStopButton");
	PanicStopButton.disabled = true;
	PrevButton.disabled = false;
}

// Variable used in the commentary table while building a CD image:
var CommentaryColumns = 2;


// Deletes all rows in the commentary table.
function ResetCommentaryTable()
{
	var Table = document.getElementById("CommentaryTable");
	while (Table.rows.length > 0)
		Table.deleteRow(0);
}

// Responds to user pressing Stop! button while building a CD image.
function PanicStop()
{
	PrevButton.disabled = true;
	showPage("PanicStoppingDiv", true);
	showPage("Stage4", false);
	var PanicStopButton = document.getElementById("PanicStopButton");
	PanicStopButton.disabled = true;
	PanicStopPressed = true;
}

// Add some text to the commentary table.
function AddCommentary(Column, Text, fNewRow, fUseInnerHtml)
{
	// If the user has pressed the Stop! button, this is where we halt the build
	// process:
	if (PanicStopPressed)
	{
		PanicStopPressed = false;
		showPage("PanicStoppingDiv", false);
		showPage("Stage4", true);
		PrevButton.disabled = false;
		Exception = new Object();
		Exception.description = "User quit build process."
		throw(Exception);
	}
	var Table = document.getElementById("CommentaryTable");
	var Cell;
	if (fNewRow)
	{
		var Row = Table.insertRow();
		for (iCol = 0; iCol < CommentaryColumns; iCol++)
		{
			var NewCell = Row.insertCell();
			if (iCol == Column)
				Cell = NewCell;
		}
	}
	else
		Cell = Table.rows[Table.rows.length - 1].cells[Column];
		
	if (fUseInnerHtml == true)
		Cell.innerHTML = Text
	else
		Cell.innerText = Text;
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
	var Cmd = 'wscript.exe "' + fso.BuildPath(UtilsPath, "CompileXmlMasterInstaller.js") + '" "' + XmlFileName + '"';
	shellObj.Run(Cmd, 0, true);
 	iLastBackslash = XmlFileName.lastIndexOf("\\");
	var InputFolder = XmlFileName.substr(0, iLastBackslash);
	var SetupExePath = InputFolder + "\\setup.exe";
	return SetupExePath;
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
	var tso = fso.OpenTextFile(strOutputFile, 2, true);
	tso.Write(xslProc.output);
	tso.Close();
}

// Makes changes to the master installer settings based on the user's selections.
function AdjustMasterInstaller(xmlDoc, flavor)
{
	// Adjust CD index settings:
	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (iProduct = 0; iProduct < ProductNodeList.length; iProduct++)
	{
		var IncludedCheckBox = document.getElementById("IncludedF" + flavor + "P" + (iProduct + 1));
		var ProductNode = ProductNodeList[iProduct];
		var CdNode = ProductNode.selectSingleNode('CD');
		if (!IncludedCheckBox.checked)
		{
			CdNode.text = "-1";
			ProductNode.removeAttribute("List");
		}
			
		// If product is locked, there may be some substitutions specified:
		if (IsProductLocked(iProduct))
		{
			var AutoConfigureNode = ProductNode.selectSingleNode('AutoConfigure');
			if (AutoConfigureNode)
			{				
				// See if there is a new Destination folder specified.
				// This swap must be done in the original XML structure, as that is
				// where the destination is read from when files are copied:
				var OrigProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');
				var OrigAutoConfigureNode = OrigProductNodeList[iProduct].selectSingleNode('AutoConfigure');
				var NewDestinationNode = OrigAutoConfigureNode.selectSingleNode('WhenLocked/Destination');
				if (NewDestinationNode)
				{
					NewDestinationNode = NewDestinationNode.cloneNode(true);
					var CondemnedNode = OrigAutoConfigureNode.selectSingleNode('Destination');
					if (CondemnedNode)
						OrigAutoConfigureNode.replaceChild(NewDestinationNode, CondemnedNode);
				}
				// Carry out any general substitutions (in the new xmlDoc) specified:
				var SubstitutionsNode = AutoConfigureNode.selectSingleNode("WhenLocked/Substitutions");
				if (SubstitutionsNode)
				{
					var SubstitutionsList = SubstitutionsNode.selectNodes("*");
					for (iSub = 0; iSub < SubstitutionsList.length; iSub++)
					{
						var SubNode = SubstitutionsList[iSub].cloneNode(true);
						var Name = SubNode.tagName;
						var CondemnedNode = ProductNode.selectSingleNode(Name);
						ProductNode.replaceChild(SubNode, CondemnedNode);
					} // Next general substitution
				} // End if any general substitions are given
			} // End if AutoConfigureNode exists
		} // End if current product is locked
	}
}

// Generates downloads.xml file from array of XML doc data.
function GenerateDownloadsXml(XmlDocs, OutputPath)
{
	// Sort the XML files, by number of missing products:
	XmlDocs.sort(sortRank)

	// Open new file for output doc:
	var NewPath = fso.BuildPath(OutputPath, "downloads.xml");
	var tso = fso.OpenTextFile(NewPath, 2, true, -1);
	tso.WriteLine('<?xml version="1.0" encoding="utf-16"?>');
//	tso.WriteLine('<?xml-stylesheet type="text/xsl" href="TestDownloads.xsl" ?>');
	tso.WriteLine('<Downloads>');

	// Process each XML file, and write the results into the big new XML file:
	for (i = 0; i < XmlDocs.length; i++)
	{
		tso.WriteLine('<Download Flavor="' + XmlDocs[i].Flavor + '" Rank="' + (i+1) + '">');
		
		var FlavorURL = XmlDocs[i].FlavorURL;
		if (FlavorURL)
			if (FlavorURL.length > 0)
				tso.WriteLine('<DownloadURL>' + XmlDocs[i].FlavorURL + '</DownloadURL>');

		// Make a copy of the original XML document:
		var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
		xmlDoc.async = false;
		XmlDocs[i].Doc.save(xmlDoc);

		// Remove all the junk we don't need:
		RemoveNodes(xmlDoc, "/MasterInstaller/AutoConfigure");
		RemoveNodes(xmlDoc, "/MasterInstaller/General");
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
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/AutoConfigure");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/CriticalFile");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Install");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/StatusWindow");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Help");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Commentary");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/KillHangingWindows");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Preinstall");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/PostInstall");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/FlushReboot");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/MustNotDelayReboot");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/RebootTestRegPending");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/RebootTestWininit");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Feature/Title");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Feature/Tag");
		RemoveNodes(xmlDoc, "/MasterInstaller/Products/Product/Feature/MsiFeature");
		
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
}

// The function passed to the sort() method on our XmlDocs array.
function sortRank(a, b)
{
	var x = a.TotalMissing;
	var y = b.TotalMissing;
	return((x < y) ? -1 : ((x > y) ? 1 : 0));
}

// Removes all nodes passed after a call to selectNodes().
function RemoveNodes(xmlDoc, XPath)
{
	Nodes = xmlDoc.selectNodes(XPath);
	while (Nodes.peekNode() != null)
		Nodes.removeNext();
}

// Copies files from their various sources to the CD image set.
function GatherFiles(iFlavor, DestinationPath)
{
	MakeSureFolderExists(DestinationPath);
	var FilesCopied = 0;

	// See if we need InstallerHelp.dll or InstallerHelp2.dll:
	if (IsAnyProductLocked())
	{
		var InstallerHelpDll = "InstallerHelp.dll";
		var InstallerHelpDllSourcePath = fso.BuildPath(CppFilePath, InstallerHelpDll);
		var InstallerHelpDllDestPath = fso.BuildPath(DestinationPath, InstallerHelpDll);
		fso.CopyFile(InstallerHelpDllSourcePath, InstallerHelpDllDestPath, true);
		AddCommentary(0, "<b>Warning - InstallerHelp2.dll cannot be generated by this tool</b>", true, true);
	}

	// See if we need External Help file(s):
	if (ExternalHelpFileData.FileList.length > 0)
	{
		AddCommentary(1, "Copying External Help file(s)", false);
		var FileList = ExternalHelpFileData.FileList;
		var RootFolder = ExternalHelpFileData.RootFolder;
		var Destination = DestinationPath;
		var ExternalHelpDestNode = document.XMLDocument.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpDestination");
		if (ExternalHelpDestNode)
			Destination = fso.BuildPath(DestinationPath, ExternalHelpDestNode.text);
		for (i = 0; i < FileList.length; i++)
		{
			var SourcePath = FileList[i];
			var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, Destination);
			MakeSureFolderExists(TargetFullPath);
			TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));
			AddCommentary(1, "[" + Math.round((100 * FilesCopied / FileTotal)) + "%] Copying " + SourcePath, false);

			// For some unknown reason, the fso.CopyFile() method fails
			// with a "permission denied" error, so we'll use our alternate
			// function instead:
			AltCopy(FileList[i], TargetFullPath);
			//fso.CopyFile(FileList[i], Destination, true);

			if (!fso.FileExists(TargetFullPath))
				AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
			FilesCopied++;
		} // Next file
	}

	// See if we need Terms of Use file(s):
	if (TermsOfUseFileData.FileList.length > 0)
	{
		AddCommentary(1, "Copying Terms of Use file(s)", false);
		var FileList = TermsOfUseFileData.FileList;
		var RootFolder = TermsOfUseFileData.RootFolder;
		var Destination = DestinationPath;
		var TermsOfUseDestNode = document.XMLDocument.selectSingleNode("/MasterInstaller/AutoConfigure/TermsOfUseDestination");
		if (TermsOfUseDestNode)
			Destination = fso.BuildPath(DestinationPath, TermsOfUseDestNode.text);
		for (i = 0; i < FileList.length; i++)
		{
			var SourcePath = FileList[i];
			var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, DestinationPath);
			MakeSureFolderExists(TargetFullPath);
			TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));
			AddCommentary(1, "[" + Math.round((100 * FilesCopied / FileTotal)) + "%] Copying " + SourcePath, false);

			// For some unknown reason, the fso.CopyFile() method fails
			// with a "permission denied" error, so we'll use our alternate
			// function instead:
			AltCopy(FileList[i], TargetFullPath);
			//fso.CopyFile(FileList[i], Destination, true);

			if (!fso.FileExists(TargetFullPath))
				AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
			FilesCopied++;
		} // Next file
	}

	// Copy all other files:
	var ProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');
	for (iProduct = 0; iProduct < ProductNodeList.length; iProduct++)
	{
		if (IsProductNeeded(iFlavor, iProduct))
		{
			var ProductNode = ProductNodeList[iProduct];
			var ProductTitle = ProductNode.selectSingleNode("Title").text;
			var Destination = fso.BuildPath(DestinationPath, ProductNode.selectSingleNode('AutoConfigure/Destination').text);
			var FileListData = SourceFileLists[iProduct].ListData;
			for (iData = 0; iData < FileListData.length; iData++)
			{
				var FileList = FileListData[iData].FileList;
				var RootFolder = FileListData[iData].RootFolder;
				var Substitution = FileListData[iData].NameWhenLocked;
				var DestPath = FileListData[iData].DestPath;

				for (i = 0; i < FileList.length; i++)
				{
					var SourcePath = FileList[i];
					var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, Destination);
					if (DestPath)
						TargetFullPath = fso.BuildPath(TargetFullPath, DestPath);
					MakeSureFolderExists(TargetFullPath);

					var UsingSub = false;
					if (IsProductLocked(iProduct))
						if (Substitution)
							UsingSub = true;

					if (UsingSub)
						TargetFullPath = fso.BuildPath(TargetFullPath, Substitution);
					else
						TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));
					AddCommentary(1, "[" + Math.round((100 * FilesCopied / FileTotal)) + "%] Copying " + SourcePath, false);

					// For some unknown reason, the fso.CopyFile() method fails
					// with a "permission denied" error, so we'll use our alternate
					// function instead:
					AltCopy(FileList[i], TargetFullPath);
					//fso.CopyFile(FileList[i], Destination, true);

					if (!fso.FileExists(TargetFullPath))
						AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
					FilesCopied++;
				} // Next file
			} // Next source node
			
			// If the Product is not locked, copy the setup help file:
			if (!IsProductLocked(iProduct))
			{
				var SetupHelpNode = ProductNode.selectSingleNode("AutoConfigure/SetupHelp");
				var SetupHelpTargetNode = ProductNode.selectSingleNode('Help');
				if (SetupHelpNode && SetupHelpTargetNode)
				{
					// Check if source path is relative:
					var HelpSource = CheckProductRelativePath(SetupHelpNode.text, ProductsPath);
					AddCommentary(1, "[" + Math.round((100 * FilesCopied / FileTotal)) + "%] Copying " + HelpSource, false);

					// For some unknown reason, the fso.CopyFile() method fails
					// with a "permission denied" error, so we'll use our alternate
					// function instead:
					AltCopy(HelpSource, fso.BuildPath(DestinationPath, SetupHelpTargetNode.text));
					FilesCopied++;
				}
			}
		}
	} // Next product
	AddCommentary(1, "Done.", false);
}

// Returns true if the specified product is needed in the specified flavor.
function IsProductNeeded(iFlavor, iProduct)
{
	var Element = document.getElementById("IncludedF" + iFlavor + "P" + (iProduct + 1));
	return Element.checked;
}

// Returns true if any needed product is locked.
function IsAnyProductLocked()
{
	var ProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');

	for (i = 0; i < ProductNodeList.length; i++)
		if (ProductNodeList[i].getAttribute("KeyId") != null && ProductNodeList[i].getAttribute("KeyId") != "")
			return true;
	return false;
}

// Returns true if any needed product is locked.
function IsProductLocked(iProduct)
{
	var ProductNodeList = document.XMLDocument.selectNodes('/MasterInstaller/Products/Product');

	if (ProductNodeList[iProduct].getAttribute("KeyId") != null && ProductNodeList[iProduct].getAttribute("KeyId") != "")
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
		alert("Error - SourcePath " + SourcePath + " does not contain expected root " + RootFolder);
	var SectionStart = RootFolder.length;
	var SectionEnd = SourcePath.lastIndexOf("\\")
	var ExtraFolders = SourcePath.slice(SectionStart, SectionEnd);
	TargetFullPath = fso.BuildPath(TargetFullPath, ExtraFolders);
	
	return TargetFullPath;
}

// For some unknown reason, the fso.CopyFile() method sometimes fails
// with a "permission denied" error. This method uses DOS to do the file
// copy instead:
function AltCopy(Source, Dest)
{
	var Cmd = 'cmd /Q /D /C copy "' + Source + '" "' + Dest + '"';
	var shellObj = new ActiveXObject("WScript.Shell");
	shellObj.Run(Cmd, 0, true);
}

// Uses the 7-zip tool to make a self-extracting archive of the specified folder.
function Compress(SourceFolder)
{
	if (SevenZipExeFile == null)
	{
		AddCommentary(1, "7-zip utility not found...", false);
		return;
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

	// Get list of all files to be used in archive:
	f7ZipFoundSvn = false;
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
	if (f7ZipFoundSvn)
	{
		// Add files:
		for (i = 0; i < FileList.length; i++)
		{
			// Remove SourceFolder from FileList[i]:
			if (FileList[i].indexOf(SourceFolder) == 0)
				FileList[i] = RootFolder + FileList[i].slice(SourceFolder.length);
			
			var tso = fso.OpenTextFile(ListFile, 8, true);
			tso.WriteLine('"' + FileList[i] + '"');
			tso.Close();
		}
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
	tso.WriteLine('copy /b "' + UtilsPath + '\\7zSD_new.sfx" + "' + ConfigFile + '" + "' + ZipFile + '" "' + SfxFile + '"');
	tso.Close();

	// Run the temporary batch file we have been building:
	Cmd = 'cmd /D /C ""' + BatchFile + '""';
	shellObj.Run(Cmd, 1, true);
	// Delete the batch and list files:
	fso.DeleteFile(BatchFile);
	if (fso.FileExists(ListFile))
		fso.DeleteFile(ListFile);

	// If the md5sums utility exists where we expect, use it to generate an md5 hash
	// of the new archive file:
	var Md5SumsPath = fso.BuildPath(CppFilePath, "Utils\\md5sums.exe");
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
}


-->
]]>
</script>
</xsl:template>
</xsl:stylesheet>
