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
							<td align="right">Path of parent folder for all output:</td>
							<td>
								<input id="OutputPath" type="text" size="60" onfocus="this.select();" title="Parent folder for all output flavors"/>
							</td>
						</tr>
					</table>
					<input id="WriteXml" type="checkbox" title="Write out an XML configuration file matching your settings for each flavor."/>Write master installer XML file for each flavor<br/>
					<script type="text/javascript">document.getElementById("WriteXml").checked=true;</script>
					<input id="WriteDownloadsXml" type="checkbox" title="Write out an XML configuration file matching your settings for the TestPresence ActiveX control."/>Write downloads.xml file<br/>
					<script type="text/javascript">document.getElementById("WriteDownloadsXml").checked=true;</script>
					<input id="Compile" type="checkbox" title="Compile a setup.exe program for each flavor."/>Compile master installer for each flavor<br/>
					<script type="text/javascript">document.getElementById("Compile").checked=true;</script>
					<input id="GatherFiles" type="checkbox" title="Gather files needed for each flavor into one folder."/>Gather files for each flavor<br/>
					<script type="text/javascript">document.getElementById("GatherFiles").checked=true;</script>
					<input id="BuildSfx" type="checkbox" title="Create self extracting 7-zip archive .exe file for each flavor." onclick="showPage('FwSfx', this.checked);"/>Build self-extracting download package for each flavor
					<script type="text/javascript">document.getElementById("BuildSfx").checked=true;</script>
					<span id="FwSfx" style="position:absolute; visibility:hidden">
						<select name="SfxStyle">
							<option value="Standard">[No user options; delete archive after setup]</option>
							<option value="UseFwSfx">[FieldWorks: User chooses destination, archive remains after setup]</option>
						</select>
						<script type="text/javascript">SelectSfxStyle();</script>
					</span>
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
var NextButton;
var PrevButton;
var PanicStopPressed = false;
var SelectedTextElement;
var VisibleStage = 1;
var MaxStage = 4;
var NumFlavors = 1;

var fso = new ActiveXObject("Scripting.FileSystemObject");	
var shellObj = new ActiveXObject("WScript.Shell");

var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
var ProductsPath = shellObj.ExpandEnvironmentStrings("%PACKAGE_PRODUCTS%");

if (MasterInstallerPath == "%MASTER_INSTALLER%")
	alert("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
else if (ProductsPath == "%PACKAGE_PRODUCTS%")
	alert("ERROR: the PACKAGE_PRODUCTS environment variable has not been defined. You cannot create web download packages without specifying where the products and documents are stored.");
	
var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");
var BitmapsPath = fso.BuildPath(MasterInstallerPath, "Bitmaps");

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

		Initializing = false;
		if (UserPressedNextWhileInitializing)
		{
			NextStage();
			showPage("Stage1StillInitializing", false);
			UserPressedNextWhileInitializing = false;
			NextButton.disabled = false;
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

	// Deal with a special case:
	showPage('FwSfx', (Stage == 3 && document.getElementById('BuildSfx').checked));

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

// Uses hueristic to select default SFX style.
function SelectSfxStyle()
{
	var StyleList = document.getElementById("SfxStyle");
	StyleList.options[0].selected = true; // Just in case we can't find a better option to select.
	var InstallerTitle = document.XMLDocument.selectSingleNode('/MasterInstaller/General/Title');
	if (InstallerTitle != null)
		if (InstallerTitle.text.indexOf("FieldWorks") != -1)
			StyleList.options[1].selected = true;
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
	var GatherFlavorFiles = document.getElementById('GatherFiles').checked;
	var BuildFlavor7zip = document.getElementById('BuildSfx').checked;
	var SfxStyle = document.getElementById('SfxStyle').value;
	
	if (!WriteFlavorXml && !WriteDownloadsXml && !CompileFlavorXml && !GatherFlavorFiles && !BuildFlavor7zip)
		AddCommentary(0, "Nothing to do!");
	else
	{
		// Everything is surrounded by a try...catch block, so we can trap errors:
		try
		{
			var OutputPath = document.getElementById('OutputPath').value;
			MakeSureFolderExists(OutputPath);
			var XmlDocs = new Array();

			for (flavor = 1; flavor <= NumFlavors; flavor++)
			{				
				var FlavorName = document.getElementById("FlavorName" + flavor).value;
				AddCommentary(0, FlavorName);

				if (WriteFlavorXml)
				{
					// Copy XML file for current flavor:
					AddCommentary(1, "Writing XML file...");
					var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
					xmlDoc.async = false;
					document.XMLDocument.save(xmlDoc);
					if (xmlDoc.parseError.errorCode != 0)
					{
						AddCommentary(0, "Error - copied XML file is invalid:\n" + xmlDoc.parseError.reason + "\non line " + xmlDoc.parseError.line + " at position " + xmlDoc.parseError.linepos);
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

				// The tasks of building setup.exe, gathering the files for a download package, and
				// building a self-extracting archive are now delegated to the RebuildWebDownload script:
				if (CompileFlavorXml || GatherFlavorFiles || BuildFlavor7zip)
				{
					AddCommentary(1, "Running RebuildWebDownload utility to do the following:");
					var Cmd = 'wscript.exe "' + fso.BuildPath(UtilsPath, 'RebuildWebDownload.js') + '" "' + FlavorXmlPath + '" -NoShowLog';

					// Put in relevant options:
					if (CompileFlavorXml)
						AddCommentary(2, "build setup.exe");
					else
						Cmd += " -NoBuildSetupExe";
						
					if (GatherFlavorFiles)
						AddCommentary(2, "gather files for download package");
					else
						Cmd += " -NoGatherFiles";
						
					if (BuildFlavor7zip)
					{
						AddCommentary(2, "build self-extracting archive");
						WriteSfxConfig(OutputPath, FlavorName, SfxStyle);
					}
					else
						Cmd += " -NoBuildSfx";

					// Run the script:
					shellObj.Run(Cmd, 1, true);
					
					// Collect the text from the log file and add it to our commentary:
					var ReportFile = fso.BuildPath(OutputPath, "RebuildWebDownload.log");
					if (fso.FileExists(ReportFile))
					{
						var File = fso.OpenTextFile(ReportFile, 1);
						while (!File.AtEndOfStream)
							AddCommentary(1, File.ReadLine());
						File.Close();
						
						// Delete the log file:
						fso.DeleteFile(ReportFile);
					}
				}
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

	AddCommentary(0, "<h2>" + FinalComment + "</h2>", true);
	var PanicStopButton = document.getElementById("PanicStopButton");
	PanicStopButton.disabled = true;
	PrevButton.disabled = false;
}


// Deletes all rows in the commentary table.
function ResetCommentaryTable()
{
	var Table = document.getElementById("CommentaryTable");
	while (Table.rows.length > 0)
		Table.deleteRow(0);
}

// Responds to user pressing Stop! button while building a download image.
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
function AddCommentary(IndentLevel, Text, fUseInnerHtml)
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
	var Row = Table.insertRow();
	var NewCell = Row.insertCell();
		
	var indentString = "";
	for (ind = 0; ind < IndentLevel; ind++)
		indentString += "    ";
	
	if (fUseInnerHtml == true)
		NewCell.innerHTML = indentString + Text;
	else
		NewCell.innerText = indentString + Text;
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
		if (IsProductLocked(ProductNode))
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

// Returns true if specified product is locked.
function IsProductLocked(ProductNode)
{
	if (ProductNode.getAttribute("KeyId") != null && ProductNode.getAttribute("KeyId") != "")
		return true;
	return false;
}

function WriteSfxConfig(OutputPath, FlavorName, SfxStyle)
{
	var ConfigFile = fso.BuildPath(OutputPath, FlavorName) + ".Config.txt";

	// Create configuration file to bind to self-extracting archive:
	var tsoConfig = fso.OpenTextFile(ConfigFile, 2, true, 0); // Must be UTF-8; ASCII will do for us
	
	switch (SfxStyle)
	{
		case 'UseFwSfx':
			tsoConfig.WriteLine(';!@Install@!UTF-8!');
			tsoConfig.WriteLine('Title="SIL FieldWorks Installation (' + FlavorName + ')"');
			tsoConfig.WriteLine('HelpText="Double-click the file \'' + FlavorName + '.exe\' to extract the installation files and run the installer.\n"');
			tsoConfig.WriteLine('InstallPath="%%S"');
			tsoConfig.WriteLine('ExtractTitle="Extracting installation files"');
			tsoConfig.WriteLine('ExtractDialogText="Preparing the \'' + FlavorName + '\' files for installation"');
			tsoConfig.WriteLine('RunProgram="fm0:\\"' + FlavorName + '\\setup.exe\\""');
			tsoConfig.WriteLine('GUIFlags="6240"');
			tsoConfig.WriteLine('BeginPrompt="The FieldWorks installation files (' + FlavorName + ') will be extracted to the folder specified below, and then the installer will run.\nIMPORTANT: You will need to keep the FieldWorks installer in that folder if you wish to apply future patch upgrades."');
			tsoConfig.WriteLine('ExtractPathText="Select a folder to store the installation files:"');
			tsoConfig.WriteLine(';!@InstallEnd@!');
			break;
		case 'Standard':
			tsoConfig.WriteLine(';!@Install@!UTF-8!');
			tsoConfig.WriteLine('Title="SIL \'' + FlavorName + '\' Software Package"');
			tsoConfig.WriteLine('HelpText="Double-click the file \'' + FlavorName + '.exe\' to extract the installation files and run the installer.\nThe extracted files will be deleted when the installer has finished.\n\nRun the file with the -nr option to simply extract the files and leave them.\n\nFile extraction will be to the same folder where this file is, in both cases."');
			tsoConfig.WriteLine('InstallPath="%%S"');
			tsoConfig.WriteLine('Delete="%%S\\' + FlavorName + '"');
			tsoConfig.WriteLine('ExtractTitle="Extracting installation files"');
			tsoConfig.WriteLine('ExtractDialogText="Preparing the \'' + FlavorName + '\' files for installation"');
			tsoConfig.WriteLine('RunProgram="fm0:\\"' + FlavorName + '\\setup.exe\\""');
			tsoConfig.WriteLine(';!@InstallEnd@!');
			break;
	}
	tsoConfig.Close();
}

-->
]]>
		</script>
	</xsl:template>
</xsl:stylesheet>
