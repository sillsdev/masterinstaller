﻿<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
  xmlns:html="http://www.w3.org/1999/xhtml"
  xmlns:dt="urn:schemas-microsoft-com:datatypes">
	<xsl:output method="html" doctype-system="" indent="yes"/>

	<!-- =============================================== -->
	<!-- Root Template                                   -->
	<!-- =============================================== -->
	<xsl:template match="/">
		<html>
			<head>
				<title>Master Installer Configuration Tool</title>
				<xsl:call-template name="script"/>
			</head>
			<body onload="Initialize();" bgcolor="#F8F8C8">
				<span style="font-size:250%">
					<b>Master Installer Configuration </b>
				</span><span style="font-size:80%">
					by <a href="mailto:alistair_imrie@sil.org?subject=Master Installer Auto-configuration Tool">Alistair Imrie</a>
				</span>
				<br/>&#169; 2006 <a href="http://www.sil.org">SIL International</a><br/><br/>
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
					<h2>Project Setup</h2>
					<h3>Make sure you have up-to-date products and Master Installer files from the Subversion repository.</h3>
					<xsl:apply-templates select="/MasterInstaller/AutoConfigure"/>
				</div>
				<div id="Stage2" style="position:absolute; visibility:hidden">
					<h2>General Configuration</h2>
					<xsl:apply-templates select="/MasterInstaller/General"/>
				</div>
				<div id="Stage3" style="position:absolute; visibility:hidden">
					<h2>Product Selection</h2>
					<xsl:apply-templates select="/MasterInstaller/Products"/>
				</div>
				<div id="Stage4" style="position:absolute; visibility:hidden">
					<h2>Select Single Instances</h2>
					<xsl:call-template name="SelectSingleInstances"/>
				</div>
				<div id="Stage5" style="position:absolute; visibility:hidden">
					<h2>CD allocation</h2>
					<div id="PrelimCompileDiv" valign="middle" style="position:absolute; visibility:hidden">
						<table>
							<tr>
								<td>
									<b>Important: </b>Some files' sizes are estimates - click button to calculate precise sizes:
								</td>
								<td>
									<button onclick="PrelimCompile();">Preliminary Compilation</button>
								</td>
							</tr>
						</table>
						<br/>
					</div>
					<xsl:call-template name="CDs"/>
				</div>
				<div id="PrelimCompileActiveDiv" style="position:absolute; visibility:hidden">
					<h3>Compiling: please wait...</h3>
				</div>
				<div id="Stage6" style="position:absolute; visibility:hidden">
					<h2>Ready to go...</h2>
					Select the tasks you want to be performed, then press the Go button.<br/><br/>
					<input id="WriteXml" type="checkbox" title="Write out an XML configuration file matching your settings."/>Write XML file<br/>
					<script type="text/javascript">document.getElementById("WriteXml").checked=true;</script>
					<input id="Compile" type="checkbox" title="Compile a setup.exe program with your settings."/>Compile master installer<br/>
					<script type="text/javascript">document.getElementById("Compile").checked=true;</script>
					<table>
						<tr>
							<td align="right">Password for digital certificate:</td>
							<td>
								<input id="CertificatePassword" type="password" size="12" onfocus="this.select();" title="Password to use when digitally signing files."/>
							</td>
						</tr>
					</table>
					<div id="CompileHelpsDiv" style="position:absolute; visibility:hidden">
						<input id="CompileHelps" type="checkbox" title="Compile InstallerHelp2.dll to provide embedded setup helps."/>Compile InstallerHelp2.dll<br/>
						<script type="text/javascript">document.getElementById("CompileHelps").checked=true;</script>
					</div>
					<input id="GatherFiles" type="checkbox" title="Gather files needed for your CD image into one folder."/>Gather files for CD image<br/>
					<script type="text/javascript">document.getElementById("GatherFiles").checked=true;</script>
					<input id="BuildIso" type="checkbox" title="Create ISO file which can be burned directly to CD. Requires Magic ISO console utility ($30)."/>Build ISO CD Image<br/>
					<script type="text/javascript">document.getElementById("BuildIso").checked=true;</script>
					<input id="BuildSfx" type="checkbox" title="Creates .exe file which extracts all files and folders needed on CD. Assumes 7-Zip utility is present."/>Build self-extracting zip of CD contents<br/>
					<script type="text/javascript">document.getElementById("BuildSfx").checked=false;</script>
					<br/>
					<br/>
					<button onclick="detailsEntered();" style="font-size:130%" title="Start the CD image build process">
						<b>&#160;&#160; Go! &#160;&#160;</b>
					</button><br/><br/>
				</div>
				<div id="PanicStoppingDiv" style="position:absolute; visibility:hidden">
					<h2>Stopping - please wait until current action finishes...</h2>
				</div>
				<div id="Stage7" style="position:absolute; visibility:hidden">
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
					Please wait while the master installer is built...<br/>
					<table id="CommentaryTable"/>
				</div>
			</body>
		</html>
	</xsl:template>


	<!-- =============================================== -->
	<!-- Project Configuration Template                  -->
	<!-- =============================================== -->
	<xsl:template match="/MasterInstaller/AutoConfigure">
		<table>
			<tr>
				<td align="right">
					<b>Output XML file name: </b>
				</td>
				<td>
					<input id="XmlFileName" type="text" onselect="InputTextSelected(this);" size="35" onfocus="this.select();" value="NewInstaller.xml" title="File name of new XML file used to compile the master installer."/>
				</td>
			</tr>
			<tr>
				<td align="right">
					<b>CD image path: </b>
				</td>
				<td>
					<input id="CdImagePath" type="text" onselect="InputTextSelected(this);" size="45" onfocus="this.select();" title="Target directory for files to be burned onto CD. If the directory does not exist, it will be created. If it does exist, it ought to be empty."/>
				</td>
				<script type="text/javascript">
					document.getElementById("CdImagePath").value=CheckCDsRelativePath("<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="CdImagePath"/>
					</xsl:call-template>");
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>CD volume label template: </b>
					<br/>
					<span style="font-size:70%">(Leave blank unless anticipating multiple CD set.)</span>
				</td>
				<td>
					<input id="CdLabelPrefix" type="text" onselect="InputTextSelected(this);" size="10" onfocus="this.select();" title="The text to be used as the basis for each CD's volume label. Each Cd's index will be added to form its default label."/>
				</td>
			</tr>
			<tr>
				<th align="right" style="font-size:130%">
					<br/>General Setup
				</th>
			</tr>
			<tr>
				<td align="right">
					<b>External Help source path:</b>
				</td>
				<td>
					<input id="ExternalHelpSource" type="text" onselect="InputTextSelected(this);" size="70" onfocus="this.select();" title="Location of files used to implement Software Support on the CD." onkeyup="TestExternalHelpSource();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("ExternalHelpSource").value=CheckSourceRelativePath("<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="ExternalHelpSource"/>
					</xsl:call-template>");
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>External Help destination folder:</b>
				</td>
				<td>
					<input id="ExternalHelpFileDest" type="text" onselect="InputTextSelected(this);" size="20" onfocus="this.select();" title="Relative path from master installer to root folder of external help file(s) - ONLY IF SOURCE PATH IS A FOLDER"/>
				</td>
				<script type="text/javascript">
					document.getElementById("ExternalHelpFileDest").value="<xsl:value-of select="ExternalHelpDestination"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Terms of use source path:</b>
				</td>
				<td>
					<input id="TermsOfUseSource" type="text" onselect="InputTextSelected(this);" size="70" onfocus="this.select();" title="Location of file(s) used as Terms of Use document(s)." onkeyup="TestTermsOfUseSource();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("TermsOfUseSource").value=CheckSourceRelativePath("<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="TermsOfUseSource"/>
					</xsl:call-template>");
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Terms of Use destination folder:</b>
				</td>
				<td>
					<input id="TermsOfUseFileDest" type="text" onselect="InputTextSelected(this);" size="20" onfocus="this.select();" title="Relative path from master installer to root folder of Terms of Use file(s) - ONLY IF SOURCE PATH IS A FOLDER"/>
				</td>
				<script type="text/javascript">
					document.getElementById("TermsOfUseFileDest").value="<xsl:value-of select="TermsOfUseDestination"/>";
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>CD size (MB):</b>
				</td>
				<td>
					<input id="CdSize" type="text" onselect="InputTextSelected(this);" size="5" onfocus="this.select();" title="Capacity of individual target CD"/>
				</td>
				<script type="text/javascript">
					document.getElementById("CdSize").value="<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="CdSize"/>
					</xsl:call-template>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Use Universal Disk Format (if this is to be a DVD):</b>
				</td>
				<td>
					<input id="UseUDF" type="checkbox" title="If checked, any .iso disk image file created will be set to use UDF."/>
				</td>
				<script type="text/javascript">
					document.getElementById("UseUDF").checked=<xsl:value-of select="UseUDF"/>
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Min file size (Bytes): </b>
				</td>
				<td>
					<input id="MinFileSize" type="text" onselect="InputTextSelected(this);" size="5" onfocus="this.select();" title="The amount of space actually used on CD for a 1-byte file"/>
				</td>
				<script type="text/javascript">
					document.getElementById("MinFileSize").value="<xsl:value-of select="MinFileSize"/>"
				</script>
			</tr>
		</table>
	</xsl:template>


	<!-- =============================================== -->
	<!-- General Configuration Template                  -->
	<!-- =============================================== -->
	<xsl:template match="/MasterInstaller/General">
		<table>
			<tr>
				<td align="right">
					<b>Title:</b>
				</td>
				<td>
					<input id="OverallTitle" type="text" onselect="InputTextSelected(this);" size="30" onfocus="this.select();" title="Overall title of master installer" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("OverallTitle").value="<xsl:value-of select="Title"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Product selection dialog subtitle:</b>
				</td>
				<td>
					<input id="ListSubtitle" type="text" onselect="InputTextSelected(this);" size="30" onfocus="this.select();" title="Text appended to overall title in product selection dialog box" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("ListSubtitle").value="<xsl:value-of select="ListSubtitle"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Product selection dialog background:</b>
				</td>
				<td>
					<table border="1">
						<tr>
							<td align="right">Main color:</td>
							<td>
								<select id="ListBackground">
									<option value="None">Blank</option>
									<option value="Green">Green (Redistributable – no caution needed)</option>
									<option value="Yellow">Yellow (Redistributable - caution Bible Translation Edition)</option>
									<option value="Red">Red (Not Redistributable – encumbered)</option>
									<option value="Blue">Blue (Not Redistributable – commercial)</option>
									<option value="Black">Black</option>
									<option value="White">White</option>
								</select>
							</td>
						</tr>
						<tr>
							<td align="right">Bitmap file:</td>
							<td>
								<select id="ListBackgroundBmp" onchange="InvalidateCompiledFiles();">
									<option value="None">None</option>
								</select>
								&#160;<button id="RefreshBitmapList" title="Scan above folder for bitmap files." onclick="RefreshBitmapList();">Refresh List</button>
							</td>
						</tr>
						<tr>
							<td align="right">X-offset:</td>
							<td>
								<input id="OffsetX" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Number of horizontal pixels (positive = right) to offset background bitmap of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("OffsetX").value="<xsl:value-of select="ListBackground/@OffsetX"/>"
								</script>
							</td>
						</tr>
						<tr>
							<td align="right">Y-offset:</td>
							<td>
								<input id="OffsetY" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Number of vertical pixels (positive = down) to offset background bitmap of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("OffsetY").value="<xsl:value-of select="ListBackground/@OffsetY"/>"
								</script>
							</td>
						</tr>
						<tr>
							<td align="right">Blend left:</td>
							<td>
								<input id="BlendLeft" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Percentage to blend-in background bitmap at left of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("BlendLeft").value="<xsl:value-of select="ListBackground/@BlendLeft"/>"
								</script>
							</td>
						</tr>
						<tr>
							<td align="right">Blend right:</td>
							<td>
								<input id="BlendRight" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Percentage to blend-in background bitmap at right of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("BlendRight").value="<xsl:value-of select="ListBackground/@BlendRight"/>"
								</script>
							</td>
						</tr>
						<tr>
							<td align="right">Blend top:</td>
							<td>
								<input id="BlendTop" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Percentage to blend-in background bitmap at top of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("BlendTop").value="<xsl:value-of select="ListBackground/@BlendTop"/>"
								</script>
							</td>
						</tr>
						<tr>
							<td align="right">Blend bottom:</td>
							<td>
								<input id="BlendBottom" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Percentage to blend-in background bitmap at bottom of product selection dialog box"/>
								<script type="text/javascript">
									document.getElementById("BlendBottom").value="<xsl:value-of select="ListBackground/@BlendBottom"/>"
								</script>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td align="right">
					<b>Initial text:</b>
				</td>
				<td>
					<table border="1">
						<tr>
							<td align="right">Message:</td>
							<td>
								<input id="InitialTextMessage" type="text" onselect="InputTextSelected(this);" size="45" onfocus="this.select();" title="Message to display above list of products"/>
							</td>
						</tr>
						<tr>
							<td align="right">Alignment:</td>
							<td>
								<select id="InitialTextAlignment">
									<option value="InitialTextLeftEdge">Left</option>
									<option value="InitialTextButtons">Buttons</option>
									<option value="InitialTextCheckBoxes">Checkboxes</option>
								</select>
							</td>
						</tr>
						<tr>
							<td align="right">Left edge offset:</td>
							<td>
								<input id="InitialTextLeftOffset" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Number of horizontal pixels (positive = right) to offset initial text"/>
							</td>
						</tr>
						<tr>
							<td align="right">Right edge offset:</td>
							<td>
								<input id="InitialTextRightOffset" type="text" onselect="InputTextSelected(this);" size="3" onfocus="this.select();" title="Number of horizontal pixels (positive = right) to offset initial text right-hand edge"/>
							</td>
						</tr>
					</table>
				</td>
			</tr>
			<tr>
				<td align="right">
					<b>Allow start from any CD in set</b>
				</td>
				<td>
					<input id="StartFromAnyCD" type="checkbox" title="If not checked, user must start from CD 1, otherwise a message will be displayed asking them to switch to CD 1."/>
				</td>
				<script type="text/javascript">
					document.getElementById("StartFromAnyCD").checked=<xsl:value-of select="StartFromAnyCD"/>;
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>List even one product</b>
				</td>
				<td>
					<input id="ListEvenOneProduct" type="checkbox" title="If checked, the user will get a list of products to select even if there is only one product. If not checked and there is only one product, it will be installed automatically."/>
				</td>
				<script type="text/javascript">
					document.getElementById("ListEvenOneProduct").checked=<xsl:value-of select="ListEvenOneProduct"/>
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Show final 'Installation has finished' message:</b>
				</td>
				<td>
					<input id="ShowInstallCompleteMessage" type="checkbox" title="If checked, a message box will be shown at the end of the entire installation sequence indicating that it has finished."/>
				</td>
				<script type="text/javascript">
					document.getElementById("ShowInstallCompleteMessage").checked=<xsl:if test="string-length(ShowInstallCompleteMessage)=0">
						<xsl:text>true</xsl:text>
					</xsl:if><xsl:if test="not(string-length(ShowInstallCompleteMessage)=0)">
						<xsl:value-of select="ShowInstallCompleteMessage"/>
					</xsl:if>;
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>List spacing adjust:</b>
				</td>
				<td>
					<input id="ListSpacingAdjust" type="text" onselect="InputTextSelected(this);" size="4" title="Number of pixels to adjust vertical spacing by, in main product list. Negative numbers are allowed, and they reduce the spacing." onfocus="this.select();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("ListSpacingAdjust").value="<xsl:value-of select="ListSpacingAdjust"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>'?' button adjust:</b>
				</td>
				<td>
					<input id="InfoButtonAdjust" type="text" onselect="InputTextSelected(this);" size="4" onfocus="this.select();" title="Number of pixels to adjust ? button size by, in main product list.Negative numbers are allowed, and they reduce the button size."/>
				</td>
				<script type="text/javascript">
					document.getElementById("InfoButtonAdjust").value="<xsl:value-of select="InfoButtonAdjust"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Secret product key prompt</b>
				</td>
				<td>
					<input id="KeyPromptNeedsShiftCtrl" type="checkbox" title="If checked, user must press [shift] + [ctrl] at startup in order to get a prompt for a Product Key."/>
				</td>
				<script type="text/javascript">
					document.getElementById("KeyPromptNeedsShiftCtrl").checked=<xsl:value-of select="KeyPromptNeedsShiftCtrl"/>;
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Get Key Title:</b>
				</td>
				<td>
					<input id="GetKeyTitle" type="text" onselect="InputTextSelected(this);" size="45" onfocus="this.select();" title="Title of dialog asking for product key (if applicable)" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("GetKeyTitle").value="<xsl:value-of select="GetKeyTitle"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>External help file:</b>
				</td>
				<td>
					<input id="ExternalHelpFile" type="text" onselect="InputTextSelected(this);" size="50" onfocus="this.select();" title="Relative path from master installer to main help file" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("ExternalHelpFile").value='<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="ExternalHelpFile"/>
					</xsl:call-template>'
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Help button text:</b>
				</td>
				<td>
					<input id="HelpButtonText" type="text" onselect="InputTextSelected(this);" size="20" onfocus="this.select();" title="Text on main help button" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("HelpButtonText").value="<xsl:value-of select="HelpButtonText"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Terms of Use file:</b>
				</td>
				<td>
					<input id="TermsOfUseFile" type="text" onselect="InputTextSelected(this);" size="50" onfocus="this.select();" title="Relative path from master installer to Terms of Use file" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("TermsOfUseFile").value='<xsl:call-template name="CppPathString">
						<xsl:with-param name="str" select="TermsOfUseFile"/>
					</xsl:call-template>'
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Terms of Use button text:</b>
				</td>
				<td>
					<input id="TermsButtonText" type="text" onselect="InputTextSelected(this);" size="20" onfocus="this.select();" title="Text on Terms of Use button" onchange="InvalidateCompiledFiles();"/>
				</td>
				<script type="text/javascript">
					document.getElementById("TermsButtonText").value="<xsl:value-of select="TermsButtonText"/>"
				</script>
			</tr>
			<tr>
				<td align="right">
					<b>Include Easter Eggs</b>
				</td>
				<td>
					<input id="IncludeEasterEggs" type="checkbox" title="Let's have some fun!"/>
				</td>
			</tr>
		</table>
	</xsl:template>


	<!-- =============================================== -->
	<!-- Products Configuration Template                 -->
	<!-- =============================================== -->
	<xsl:template match="/MasterInstaller/Products">
		<table>
			<xsl:for-each select="Product">
				<tr>
					<td align="right">
						<xsl:value-of select="AutoConfigure/Title"/>
						<input id="ProductTitle{count(preceding-sibling::Product)}" type="checkbox" onclick="showPage('ProductSetup{count(preceding-sibling::Product)}', this.checked); InvalidateCompiledFiles();">
							<xsl:if test="@List='true'">
								<xsl:attribute name="checked">true</xsl:attribute>
							</xsl:if>
						</input>
					</td>
					<td>
						<span id="ProductSetup{count(preceding-sibling::Product)}" style="position:absolute; visibility:hidden">
							<xsl:if test="not(string-length(@KeyId)=0)">
								<input id="UseKey{count(preceding-sibling::Product)}" type="checkbox" onclick="InvalidateCompiledFiles();"/>Requires Product Key
							</xsl:if>
						</span>
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:template>


	<!-- =============================================== -->
	<!-- Select Single Instances Template                -->
	<!-- =============================================== -->
	<xsl:template name="SelectSingleInstances">
		<h4>
			The products listed below have more than one variety.<br/>In each case, you must select which one to include in your CD set.
		</h4>
		<table border="1">
			<tr>
				<th>Product</th>
				<th>Varieties</th>
			</tr>
			<xsl:for-each select="/MasterInstaller/Products/Product">
				<tr id="DuplicateSetTr{count(preceding-sibling::Product)}" style="display:none">
					<td align="right" id="DuplicateSetTrTitle{count(preceding-sibling::Product)}"/>
					<td align="left" id="DuplicateSetTrOptions{count(preceding-sibling::Product)}"/>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:template>

	<!-- =============================================== -->
	<!-- CDs Configuration Template                      -->
	<!-- =============================================== -->
	<xsl:template name="CDs">
		<table border="1">
			<tr>
				<th>Product</th>
				<th>Size of files</th>
				<th>Include</th>
				<th>CD number</th>
			</tr>
			<xsl:for-each select="/MasterInstaller/Products/Product">
				<tr id="ProductCdTr{count(preceding-sibling::Product)}" style="display:none">
					<td align="center">
						<xsl:value-of select="AutoConfigure/Title"/>
					</td>
					<td align="center" id="FileSize{count(preceding-sibling::Product)}"></td>
					<td align="center">
						<input id="Included{count(preceding-sibling::Product)}" type="checkbox" title="Select to include, clear to omit this product from the CD." onclick="EnableCdIndex({count(preceding-sibling::Product)}, this.checked); UpdateCdTotals();">
							<xsl:if test="not(CD &lt; 0)">
								<xsl:attribute name="checked">true</xsl:attribute>
							</xsl:if>
						</input>
					</td>
					<td align="center">
						<input id="ProductCD{count(preceding-sibling::Product)}" value="0" onkeyup="UpdateCdTotals();" type="text" size="2" title="Index of CD to place this product on. Zero-based." onfocus="this.select();">
							<xsl:if test="CD >= 0">
								<xsl:attribute name="value">
									<xsl:value-of select="CD"/>
								</xsl:attribute>
							</xsl:if>
						</input>
					</td>
				</tr>
			</xsl:for-each>
		</table>
		<br/>
		<!-- Set up table for CD labels etc. Allow one CD per product, initially - we will control this interactively. -->
		<table border="1">
			<tr>
				<th>Index</th>
				<th>Title</th>
				<th>Volume Label</th>
				<th>Space Used</th>
				<th>Space Available</th>
				<th>Notes</th>
			</tr>
			<xsl:for-each select="/MasterInstaller/Products/Product">
				<tr id="CdTr{count(preceding-sibling::Product)}" style="display:none">
					<td align="center">
						<xsl:number value="count(preceding-sibling::Product)"/>
					</td>
					<td align="center">
						<input id="CdTitle{count(preceding-sibling::Product)}" type="text" size="30" value="{/MasterInstaller/CDs/CD[1 + current()/CD]/Title}" title="Title to display to user for CD {count(preceding-sibling::Product)}" onfocus="this.select();"/>
					</td>
					<td align="center">
						<input id="CdLabel{count(preceding-sibling::Product)}" type="text" value="{/MasterInstaller/CDs/CD[1 + current()/CD]/VolumeLabel}" title="Volume label of CD {count(preceding-sibling::Product)}" onfocus="this.select();"/>
					</td>
					<td align="center" id="CdUsed{count(preceding-sibling::Product)}"></td>
					<td align="center" id="CdSpace{count(preceding-sibling::Product)}"></td>
					<td align="left" id="CdNotes{count(preceding-sibling::Product)}" style="font-size:70%"></td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:template>

	<!--
This template is used as a recursive function to double-up '\' and to put '\' before '"'
-->
	<xsl:template name="CppPathString">
		<xsl:param name="str"/>
		<xsl:if test="contains($str, '\')">
			<xsl:call-template name="CppPathString">
				<xsl:with-param name="str" select="substring-before($str, '\')"/>
			</xsl:call-template>
			<xsl:value-of select="'\\'"/>
			<xsl:call-template name="CppPathString">
				<xsl:with-param name="str" select="substring-after($str, '\')"/>
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="not(contains($str, '\'))">
			<xsl:call-template name="SearchReplace">
				<xsl:with-param name="str" select="$str"/>
				<xsl:with-param name="find" select="'&quot;'"/>
				<xsl:with-param name="replace" select="'\&quot;'"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!--
This template is used as a recursive function to search and replace
-->
	<xsl:template name="SearchReplace">
		<xsl:param name="str"/>
		<xsl:param name="find"/>
		<xsl:param name="replace"/>
		<xsl:if test="contains($str, $find)">
			<xsl:call-template name="SearchReplace">
				<xsl:with-param name="str" select="substring-before($str, $find)"/>
				<xsl:with-param name="find" select="$find"/>
				<xsl:with-param name="replace" select="$replace"/>
			</xsl:call-template>
			<xsl:value-of select="$replace"/>
			<xsl:call-template name="SearchReplace">
				<xsl:with-param name="str" select="substring-after($str, $find)"/>
				<xsl:with-param name="find" select="$find"/>
				<xsl:with-param name="replace" select="$replace"/>
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="not(contains($str, $find))">
			<xsl:value-of select="$str"/>
		</xsl:if>
	</xsl:template>


	<!-- =============================================== -->
	<!-- Script Template                                 -->
	<!-- =============================================== -->
	<xsl:template name="script">
		<script type="text/javascript">
			<![CDATA[
  <!--
// This is line 7
var Initializing = true;
var UserPressedNextWhileInitializing = false;

var fso = new ActiveXObject("Scripting.FileSystemObject");
var shellObj = new ActiveXObject("WScript.Shell");

var MasterXmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
MasterXmlDoc.async = false;
MasterXmlDoc.load(document.URL);
if (MasterXmlDoc.parseError.errorCode != 0)
	alert("Error - XML file is invalid:\n" + MasterXmlDoc.parseError.reason + "\non line " + MasterXmlDoc.parseError.line + " at position " + MasterXmlDoc.parseError.linepos);

var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
var ProductsPath = shellObj.ExpandEnvironmentStrings("%PACKAGE_PRODUCTS%");
var CDsPath = shellObj.ExpandEnvironmentStrings("%CD_IMAGES%");
var CppFilePath = fso.BuildPath(MasterInstallerPath, "Code and Projects");
var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");
var BitmapsPath = fso.BuildPath(CppFilePath, "Bitmaps");
var InstallerDefinitionsPath = fso.BuildPath(MasterInstallerPath, "Installer Definitions");

if (CppFilePath == "%MASTER_INSTALLER%")
  alert("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
else if (ProductsPath == "%PACKAGE_PRODUCTS%")
  alert("WARNING: the PACKAGE_PRODUCTS environment variable has not been defined. You cannot create CD images without specifying where the products and documents are stored.");
else if (CDsPath == "%CD_IMAGES%")
  alert("WARNING: the CD_IMAGES environment variable has not been defined. You cannot create CD images without specifying where the image archives are stored.");

var NumProducts;
var NeededProducts;
var SourceFileLists;
var ProductSizes;
var ExternalHelpFileData;
var ExternalHelpNeeded;
var TermsOfUseFileData;
var TermsOfUseNeeded;
var CdDetails;
var MaxNumCds;
var DuplicateSets;
var NumDuplicateSets = 0;
var NumVisibleDuplicateSets = 0;
var VisibleStage = 1;
var MaxStage = 7;
var SetupExePath;
var AutorunInfPath;
var InstallerHelp2DllPath;
var CompiledFilesSizesKnown = false;
var PanicStopPressed = false;
var NextButton;
var PrevButton;
var SelectedTextElement;


// Called upon completion of page loading.
function Initialize()
{
	try
	{
		SelectBackgroundColor();
		RefreshBitmapList();
		
    NextButton = document.getElementById('NextButton');
		PrevButton = document.getElementById('PrevButton');
		showPage("BlockedContentWarning", false);
		showPage("Stage1", true);
    
		var ProductNodeList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
		NumProducts = ProductNodeList.length;

    InitSourceFileLists(); // Must come after assigning NumProducts
		FindDuplicateSets();
		NeededProducts = new Array();
		CdDetails = new Array();
		MaxNumCds = NumProducts; // Allow max of one CD per product
		InitAllProductSizes();

		for (iProduct = 0; iProduct < NumProducts; iProduct++)
			NeededProducts[iProduct] = false;

		TestExternalHelpSource();
		TestTermsOfUseSource();

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

// Returns the folder containing the source XML document.
function GetDocumentFolder()
{
	// Get URL of document, making sure we change backslashes, into forward ones, and the 
	// %20 mark with a proper space. These changes appeared new with IE7:
	var URL = document.URLUnencoded;
	URL = URL.replace(/\//g,"\\");
	URL = URL.replace(/%20/g," ");
	var iLastBackslash = URL.lastIndexOf("\\");
	var iEndOfProtocol = 0;
	if (URL.slice(0, 7) == "file:\\\\")
		iEndOfProtocol = 7;
	if (URL.slice(0, 8) == "file:\\\\\\")
		iEndOfProtocol = 8;
	return URL.substr(iEndOfProtocol, iLastBackslash - iEndOfProtocol);
}

// Analyzes whether the given path is a relative path or a full path specification.
// If relative, the location of the source XML file is prepended.
function CheckRelativePath(Path)
{
	var IsRelative = true;
	// Check if Path starts with a backslash:
	if (Path.substr(0, 1) == "\\")
		IsRelative = false;
	// Check if Path starts with a drive letter and colon:
	if (Path.substr(1, 2) == ":\\")
		IsRelative = false;

	// If Path was found to be relative, prepend the path of the source XML file:
	if (IsRelative)
	{
		var OrigPath = Path; // Keep a copy, just in case.
		
    Path = fso.BuildPath(GetDocumentFolder(), Path);
		
		// If the new path doesn't exist, try prepending the original path with the Cpp File Path:
		if (!fso.FolderExists(Path))
			Path = fso.BuildPath(CppFilePath, OrigPath);
	}
	return Path;
}

// Shows or hides a specified row of a table.
function showRow(sRow, bShow, RegColor, RowTitle)
{
    var row = document.getElementById(sRow);
    row.style.display = bShow ? 'block' : 'none';
	row.style.background = RegColor ? '' : '#E8E8E8'
	if (RowTitle)
		row.title = RowTitle;
}

function VerifyPage(CurrentStage)
{
	var Errors = "";
	var Path;
	var CulpritElement;

	switch (CurrentStage)
	{
		case 1:
			// Check at least one C++ file exists in the specified place:
			Path = fso.BuildPath(CppFilePath, "main.cpp");
			if (!fso.FileExists(Path))
			{
				Errors += "The MASTER_INSTALLER environment variable does not point to a folder containing the main.cpp Master Installer source file.\n";
				Errors += "\tYou must correct this, if you want to compile a master installer.\n";
			}
			else
			{
				// Check for existence of InstallerHelp.dll:
				Path = fso.BuildPath(CppFilePath, "InstallerHelp.dll");
				if (!fso.FileExists(Path))
				{
					Errors += "The expected file '" + Path + "' does not exist.\n";
					Errors += "\tYou must correct this, if you want to use locked installers in the master installer.\n";
				}

				// Check for existence of the Binary to C++ utility:
				Path = fso.BuildPath(UtilsPath, "Bin2Cpp.exe");
				if (!fso.FileExists(Path))
				{
					Errors += "The expected file '" + Path + "' does not exist.\n";
					Errors += "\tYou must correct this, if you want to use locked installers in the master installer.\n";
				}

				// Check for existence of InstallerHelp2.cpp:
				Path = fso.BuildPath(CppFilePath, "InstallerHelp2.cpp");
				if (!fso.FileExists(Path))
				{
					Errors += "The expected file '" + Path + "' does not exist.\n";
					Errors += "\tYou must correct this, if you want to use locked installers in the master installer.\n";
				}
			
			}

			// Check for existence of External Help file(s):
			Path = document.getElementById("ExternalHelpSource").value;
      Path = CheckSourceRelativePath(Path);
			if (Path.length > 0 && !fso.FileExists(Path) && !fso.FolderExists(Path))
			{
				Errors += "The External Help Source Path does not point to an existing file or folder.\n";
				Errors += "\tYou must correct this, if you want to include External Help file(s) in the master installer.\n";
				CulpritElement = document.getElementById("ExternalHelpSource");
			}

			// Check for existence of Terms of Use file(s):
			Path = document.getElementById("TermsOfUseSource").value;
      Path = CheckSourceRelativePath(Path);
			if (Path.length > 0 && !fso.FileExists(Path) && !fso.FolderExists(Path))
			{
				Errors += "The Terms of Use Source Path does not point to an existing file or folder.\n";
				Errors += "\tYou must correct this, if you want to include Terms of Use file(s) in the master installer.\n";
				CulpritElement = document.getElementById("TermsOfUseSource");
			}
			break;
		case 2:
			// If a Background Bitmap file is selected, make sure the blend values are not all zero:
			var BmpList = document.getElementById("ListBackgroundBmp");
			if (BmpList.value != "None")
			{
				if (document.getElementById("BlendLeft").value == 0
					&& document.getElementById("BlendRight").value == 0
					&& document.getElementById("BlendTop").value == 0
					&& document.getElementById("BlendBottom").value == 0)
				{
					Errors += "You have selected a background bitmap, but the blend values are all zero.\n";
					Errors += "\tYou must set blend values, if you want the background bitmap to be visible.\n";
					CulpritElement = document.getElementById("BackgroundBmp");
				}
			}
			break;		
	}
	if (Errors.length > 0)
	{
		var conf = confirm("Warning - not all data is valid:\n\n" + Errors + "\nClick OK only if you can live with yourself after making these errors.");
		if (!conf && CulpritElement)
			CulpritElement.focus();
		return conf;
	}

	return true;
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
			if (Stage > 1)
			{
				// If we're about to advance past the Project Setup page, Recalc file sizes:
				GenerateExtHelpAndTermsFileLists();
				InitAllProductSizes();
			}
			break;
		case 2:
			if (Stage > 2)
			{
				// If we're about to advance past the General Configuration page, apply defaults to CD
				// label and title, if blank, and see if Terms of Use and External Help are needed:
				var Title = document.getElementById('OverallTitle').value;
				var CdLabelPrefix = document.getElementById('CdLabelPrefix').value;
				for (iCd = 0; iCd < MaxNumCds; iCd++)
				{
					CdTitleElement = document.getElementById('CdTitle' + iCd);
					if (CdTitleElement.value == "")
						CdTitleElement.value = Title + " Disc " + (1 + iCd);
					CdLabelElement = document.getElementById('CdLabel' + iCd);
					if (CdLabelElement.value == "")
						CdLabelElement.value = CdLabelPrefix + (1 + iCd);
				}
				// See if Terms of Use and External Help are needed:
				TermsOfUseNeeded = (document.getElementById("TermsOfUseFile").value.length > 0);
				ExternalHelpNeeded = (document.getElementById("ExternalHelpFile").value.length > 0);
			}
			break;
		case 3:
			if (Stage > 3)
			{
				// If we're about to advance past the Product Selection page, Recalc needed products:
				CalcNeededProducts();
				ShowDuplicateSetData();
				WriteTotalSourceSizes();
			}
			break;
		case 4:
			if (Stage > 4)
			{
				// If we're about to advance past the Select Single Instances page, Adjust needed products:
				AdjustNeededProductsDuplicates();
			}
			break;
	}

	// Perform certain housekeeping tasks, depending on which stage we're about to enter:
	switch (Stage)
	{
		case 4:
			// If we're about to enter the Select Single Instances page, check that there is
			// anything to show:
			if (NumVisibleDuplicateSets < 1)
			{
				// There is nothing to show, so don't show this page:
				return false;
			}
			break;
		case 5:
			// If we're about to enter the CD Allocation page, Recalc CD Totals:
			for (i = 0; i < NumProducts; i++)
			{
				EnableCdIndex(i, document.getElementById('Included' + i).checked);
				var ProductSelected = document.getElementById('ProductTitle' + i).checked;
				var Title = ProductSelected ? "" : "This product is needed, even though you did not select it.";
				showRow('ProductCdTr' + i, IsProductNeeded(i), ProductSelected, Title);
			}
			UpdateCdTotals();
			break;
			
		case 7:
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
	
	// Deal with a few special cases:
	for (i = 0; i < NumProducts; i++)
		showPage('ProductSetup' + i, (Stage == 3 && document.getElementById('ProductTitle' + i).checked));
	showPage('CompileHelpsDiv', (Stage == 6 && IsAnyProductLocked()));
	showPage('PrelimCompileDiv', (Stage == 5 && !CompiledFilesSizesKnown));
	
	// Perform certain housekeeping tasks, depending on which stage we've just entered:
	switch (Stage)
	{
		case 2:
			// If the External Help file has a destination in the AutoConfigure that does not
			// match the start of the specified file path on this page, notify user:
			if (!document.getElementById("ExternalHelpFileDest").disabled)
			{
				var HelpFileDest = document.getElementById("ExternalHelpFileDest").value;
				var HelpFile = document.getElementById("ExternalHelpFile").value;
				var MsgNeeded = false;
				if (HelpFile.length > 0)
				{
					if (HelpFile.length <= HelpFileDest)
						MsgNeeded = true;
					else if (HelpFile.slice(0, HelpFileDest.length + 1) != HelpFileDest + "\\")
						MsgNeeded = true;
				}
				if (MsgNeeded)
				{
					alert("Warning - External Help file specified on this page has a path inconsistent with the Destination specified on the previous page.");
					document.getElementById("ExternalHelpFile").focus();
				}
			}
			// If the Terms of Use file has a destination in the AutoConfigure that does not
			// match the start of the specified file path on this page, notify user:
			if (!document.getElementById("TermsOfUseFileDest").disabled)
			{
				var TermsOfUseFileDest = document.getElementById("TermsOfUseFileDest").value;
				var TermsOfUseFile = document.getElementById("TermsOfUseFile").value;
				var MsgNeeded = false;
				if (TermsOfUseFile.length <= TermsOfUseFileDest)
					MsgNeeded = true;
				else if (TermsOfUseFile.slice(0, TermsOfUseFileDest.length + 1) != TermsOfUseFileDest + "\\")
					MsgNeeded = true;
				if (MsgNeeded)
				{
					alert("Warning - Terms of Use file specified on this page has a path inconsistent with the Destination specified on the previous page.");
					document.getElementById("TermsOfUseFile").focus();
				}
			}
			break;
	}
	return true;
}

// Add commas to a number string, every three digits.
function addCommas(nStr)
{
	nStr += '';
	x = nStr.split('.');
	x1 = x[0];
	x2 = x.length > 1 ? '.' + x[1] : '';
	var rgx = /(\d+)(\d{3})/;
	while (rgx.test(x1)) {
		x1 = x1.replace(rgx, '$1' + ',' + '$2');
	}
	return x1 + x2;
}

// Resets a flag which indicates whether or not we know the sizes of setup.exe, autorun.inf,
// InstallerHelp.dll and InstallerHelp2.dll
function InvalidateCompiledFiles()
{
	CompiledFilesSizesKnown = false;
}

// Populates the ListBackgroundBmp combo box with the names of any bitmap files found in the bitmap
// folder specified in the BitmapsPath variable.
function RefreshBitmapList()
{
  var Folder = fso.GetFolder(BitmapsPath);
	var fc = new Enumerator(Folder.files);
	document.getElementById("ListBackgroundBmp").options[0].text = "None";
	var count = 1;
	var fSelected = false;
	var OriginalFile = MasterXmlDoc.selectSingleNode("/MasterInstaller/General/ListBackground").text;
	for (; !fc.atEnd(); fc.moveNext())
	{
		var File = fc.item();
		if (File.Name.slice(-4).toLowerCase() == ".bmp")
		{
			document.getElementById("ListBackgroundBmp").options[count] = new Option(File.Name, File.Name);
			if (File.Name == OriginalFile)
			{
				document.getElementById("ListBackgroundBmp").options[count].selected = true;
				fSelected = true;
			}
			count++;
		}
	}
	document.getElementById("ListBackgroundBmp").options.length = count;
	if (!fSelected && OriginalFile.length > 0)
	{
		var Warning = 'Warning: Background bitmap specification "' + OriginalFile + '" is not valid.';
    if (OriginalFile.indexOf("\\") >= 0)
      if (!fso.FileExists(OriginalFile))
			  Warning = "Bitmap path '" + OriginalFile + "' specified in XML node /MasterInstaller/General/ListBackground does not exist. This may be legacy data. Bitmaps should be stored in the '" + BitmapsPath + "' folder and referenced by file name only.";
		alert(Warning);
	}
}

// Selects the background color item in the combo box that matches existing Red, Green and Blue values.
function SelectBackgroundColor()
{
	var BackgroundNode = MasterXmlDoc.selectSingleNode("/MasterInstaller/General/ListBackground");
	if (!BackgroundNode)
		return;
		
	var rgbRed = BackgroundNode.getAttribute("Red");
	var rgbGreen = BackgroundNode.getAttribute("Green");
	var rgbBlue = BackgroundNode.getAttribute("Blue");
	
	var Color;
	
	if (rgbRed == 196 && rgbGreen == 206 && rgbBlue == 44)
		Color = "Green";
	else if (rgbRed == 255 && rgbGreen == 242 && rgbBlue == 0)
		Color = "Yellow";
	else if (rgbRed == 204 && rgbGreen == 57 && rgbBlue == 33)
		Color = "Red";
	else if (rgbRed == 0 && rgbGreen == 103 && rgbBlue == 166)
		Color = "Blue";
	else if (rgbRed == 0 && rgbGreen == 0 && rgbBlue == 1)
		Color = "Black";
	else if (rgbRed == 255 && rgbGreen == 255 && rgbBlue == 255)
		Color = "White";

	var i;
	var ColorElement = document.getElementById("ListBackground");
	for (i = 0; i < ColorElement.options.length; i++)
	{
		if (ColorElement.options[i].value == Color)
		{
			ColorElement.options[i].selected = true;
			break;
		}
	}
}

// Records last input text element to get text selected.
function InputTextSelected(element)
{
	SelectedTextElement = element;
}

// Test if External Help Source is a folder. Enables or disables ExternalHelpFileDest
// accordingly.
function TestExternalHelpSource()
{
  Path = CheckSourceRelativePath(document.getElementById("ExternalHelpSource").value);
  var DisableDestination = !fso.FolderExists(Path);
	document.getElementById("ExternalHelpFileDest").disabled = DisableDestination;
}

// Test if Terms of Use Source is a folder. Enables or disables TermsOfUseFileDest
// accordingly.
function TestTermsOfUseSource()
{
  Path = CheckSourceRelativePath(document.getElementById("TermsOfUseSource").value);
  var DisableDestination = !fso.FolderExists(Path);
	document.getElementById("TermsOfUseFileDest").disabled = DisableDestination;
}

// Examines product tags to determine duplicates, and sets up an array of duplicate set markers.
function FindDuplicateSets()
{
	DuplicateSets = new Array();
	NumDuplicateSets = 0;
	var ProductList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (iProd = 0; iProd < NumProducts; iProd++)
	{
		if (DuplicateSets[iProd] == null)
		{
			var ProductNode = ProductList[iProd];
			var TagNode = ProductNode.selectSingleNode("Tag");
			var Indexes = FindProductsByTag(TagNode.text, false);
			if (Indexes.length > 1)
			{
				for (i2 = 0; i2 < Indexes.length; i2++)
					DuplicateSets[Indexes[i2]] = NumDuplicateSets;
				NumDuplicateSets++;
			}
		}
	}
	FillDuplicateSetTable();
}

// Fills in the Duplicate Sets table.
function FillDuplicateSetTable()
{
	var ProductList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
	
	for (iSet = 0; iSet < NumDuplicateSets; iSet++)
	{
		for (iProduct = 0; iProduct < NumProducts; iProduct++)
		{
			if (DuplicateSets[iProduct] == iSet)
			{
				var ProductNode = ProductList[iProduct];

				var TableData = document.getElementById('DuplicateSetTrTitle' + iSet);
				TableData.innerText = ProductNode.selectSingleNode('Title').text + " [" + ProductNode.selectSingleNode('Tag').text + "]";

				TableData = document.getElementById('DuplicateSetTrOptions' + iSet);
				TableData.innerHTML = '';

				// Find all duplicates:
				var FirstOption = true;
				for (i = 0; i < NumProducts; i++)
				{
					if (DuplicateSets[i] == iSet)
					{
						var innerHTMLSegment = '<input type="radio" name="Set' + iSet + '" id="DuplicateProduct' + i + '"';
						if (FirstOption)
						{
							innerHTMLSegment += ' checked="checked"';
							FirstOption = false;
						}
						innerHTMLSegment += '>' + ProductList[i].selectSingleNode('AutoConfigure/Title').text + '<br/>';
						TableData.innerHTML += innerHTMLSegment;
					}
				}
				
				// Force loop to next set:
				iProduct = NumProducts;
			}
		}
	}
}

// Fills in the Duplicate Sets table.
function ShowDuplicateSetData()
{
	NumVisibleDuplicateSets = 0;
	for (iSet = 0; iSet < NumDuplicateSets; iSet++)
	{
		for (iProduct = 0; iProduct < NumProducts; iProduct++)
		{
			showRow('DuplicateSetTr' + iSet, false, true);

			// Show deplicates set only for products that are requirements or dependencies of
			// selected products:
			if (IsProductNeeded(iProduct) && !document.getElementById('ProductTitle' + iProduct).checked && DuplicateSets[iProduct] == iSet)
			{
				showRow('DuplicateSetTr' + iSet, true, true);
				NumVisibleDuplicateSets++;
				
				// Force loop to next set:
				iProduct = NumProducts;
			}
		}
	}
}

// Adjusts the array of needed products according to user's selections on the Select Single
// Instances page.
function AdjustNeededProductsDuplicates()
{
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		var RadioButton = document.getElementById('DuplicateProduct' + iProduct);
		if (RadioButton)
			NeededProducts[iProduct] = RadioButton.checked;
	}
}

// Using the user's selection of products, works out which other products must be added
// to the CD distribution.
function CalcNeededProducts()
{
	var iProduct;

	// Reset all, initially:
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
		NeededProducts[iProduct] = false;

	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		if (document.getElementById('ProductTitle' + iProduct).checked)
		{
			NeededProducts[iProduct] = true;
			AddDependenciesOfProduct(iProduct);
		}
	}
}

// Works out which other products are needed by the given product.
function AddDependenciesOfProduct(iProduct)
{
	var ProductNode = MasterXmlDoc.selectSingleNode('/MasterInstaller/Products/Product[' + iProduct + ']');
	AddDependenciesOfProductNode(ProductNode);

	// Check for features:
	var ProductFeaturesList = ProductNode.selectNodes('Feature');
	for (iFeature = 0; iFeature < ProductFeaturesList.length; iFeature++)
		AddDependenciesOfProductNode(ProductFeaturesList[iFeature]);
}

// Works out which other products are needed by the given product.
function AddDependenciesOfProductNode(ProductNode)
{
	var PrerequisitesList = ProductNode.selectNodes('Prerequisite');
	if (PrerequisitesList != null)
		FindAndAddDependentProducts(PrerequisitesList);

	var RequirementsList = ProductNode.selectNodes('Requires');
	if (RequirementsList != null)
		FindAndAddDependentProducts(RequirementsList);
}

// Marks the products listed by the given prerequisites or requirements list, in the 
// NeededProducts array.
function FindAndAddDependentProducts(List)
{
	for (iProductDep = 0; iProductDep < List.length; iProductDep++)
	{
		var Dependency = List[iProductDep];
		var Tag = Dependency.getAttribute("Tag");
		var Indexes = FindProductsByTag(Tag, true);
		var Index = Indexes[0];
		var NeedRecursion = !(NeededProducts[Index]);
		NeededProducts[Index] = true;
		if (NeedRecursion)
			AddDependenciesOfProduct(Index);
	}
}

// Returns an array of indexes of products having the given Tag.
function FindProductsByTag(Tag, StopAfterFirstMatch)
{
	var Indexes = new Array();
	var ProductList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (i = 0; i < NumProducts; i++)
	{
		var ProductNode = ProductList[i];
		var TagNode = ProductNode.selectSingleNode("Tag");
		if (TagNode == null)
			alert("Missing tag in XML product " + ProductNode.selectSingleNode("Title").text);
		else
		{
			if (TagNode.text == Tag)
			{
				Indexes.push(i);
				if (StopAfterFirstMatch)
					return Indexes;
			}
		}
	}
	if (Indexes.length == 0)
		alert("Error: Invalid tag '" + Tag + "' used in XML file.");
	return Indexes;
}

// Returns true if the given product (referenced by its index) is needed on a CD.
function IsProductNeeded(i)
{
	return NeededProducts[i];
}

// Shows all needed Cds, along with the space left on each, by displaying or hiding
// rows in the CD table.
function showCdRows()
{
	var CdSize = document.getElementById("CdSize").value * 1024 * 1024;
	
	for (iCd = 0; iCd < MaxNumCds; iCd++)
	{
		// Check if the current CD is needed:
		CdNeeded = (iCd == 0 || CdDetails[iCd].CdInUse());
		showRow('CdTr' + iCd, CdNeeded, true);
		if (CdNeeded)
		{
			// Add commentary/notes about the CD:
			CdNotesElement = document.getElementById('CdNotes' + iCd);
			Notes = CdDetails[iCd].DisplayNotes;
			if (Notes.length == 0)
				Notes = " ";
			CdNotesElement.innerText = Notes;

			// Display how much space is used:
			CdUsedElement = document.getElementById('CdUsed' + iCd);
			var SpaceUsed = CdDetails[iCd].TotalSize;
			CdUsedElement.innerText = addCommas(SpaceUsed);

			// Display how much space is left:
			CdSpaceElement = document.getElementById('CdSpace' + iCd);
			var SpaceAvailable = CdSize - CdDetails[iCd].TotalSize;
			CdSpaceElement.innerText = addCommas(SpaceAvailable);
			
			var ExtraSpaceEstimate = 1024 * 1024;
			if (!CompiledFilesSizesKnown)
			{
				if (CdDetails[iCd].NeedsInstallerHelp2Dll)
					ExtraSpaceEstimate += 1024 * 1024;
			}

			// Do fancy stuff if CD is overfull or nearly full:
			if (SpaceAvailable < 0)
			{
				CdSpaceElement.style.background = "#FF6060";
				CdSpaceElement.title = "This CD is overfilled."
				CdNotesElement.innerText += "\nThis CD is overfilled."
			}
			else if (SpaceAvailable < ExtraSpaceEstimate)
			{
				CdSpaceElement.style.background = "#FFA0A0";
				CdSpaceElement.title = "This CD may be overfilled."
				CdNotesElement.innerText += "\nThis CD may be overfilled, once unknown overheads are included."
			}
			else
			{
				CdSpaceElement.style.background = "";
				CdSpaceElement.title = "";
			}
		}
	}
}

// If flag is true, enables the CD index table data. Otherwise, disables it.
function EnableCdIndex(i, flag)
{
	document.getElementById('ProductCD' + i).disabled = !flag;
}

// Initialize object containing details about a CD.
function CdDetails_t()
{
	this.TotalSize = 0;
	this.NeedsInstallerHelpDll = false;
	this.NeedsInstallerHelp2Dll = false;
	this.NeedsSetupExe = false;
	this.NeedsAutorunInf = false;
	this.DisplayNotes = "";
	this.CdInUse = IsCdNeeded;
}

// Returns true if the CD details object records that some space is used up.
function IsCdNeeded()
{
	return (this.TotalSize > 0);
}

// Translates an actual file size into the size that will be used on a CD.
function RoundCdFileSize(ActualSize)
{
	var MinFileSize = document.getElementById("MinFileSize").value;
	return (Math.ceil(ActualSize / MinFileSize) * MinFileSize);
}

// Returns the index of the CD that will contain the specified product,
// or -1 if product was expressly omitted.
function GetCdIndexOfProduct(iProduct)
{
	var Element = document.getElementById('ProductCD' + iProduct);
	if (Element)
	{
		if (!document.getElementById('Included' + iProduct).checked)
			return -1;

		var Value = document.getElementById('ProductCD' + iProduct).value;
		if (Value == "")
			return 0;
		var Cd = parseInt(Value);
		if (Cd >= 0 && Cd < MaxNumCds)
			return Cd;
	}
	alert("Error - CD index out of range [0.." + (MaxNumCds - 1) + "] for product " + iProduct + ".");
	return 0;
}

// Uses the user's settings to determine various details for each CD, particularly how much
// space is left.
function UpdateCdTotals()
{
	// Reset all, initially:
	for (i = 0; i < MaxNumCds; i++)
		CdDetails[i] = new CdDetails_t();

	// Deal with general case CD allocation:
	for (i = 0; i < NumProducts; i++)
	{
		if (IsProductNeeded(i))
		{
			var iCd = GetCdIndexOfProduct(i);
			if (iCd >= 0)
				CdDetails[iCd].TotalSize += GetProductSize(i);
		}
	}
	// Deal with special cases: the need for setup.exe, InstallerHelp.dll, etc
	// Estimate file sizes where not known:
	var RoundedSetupExeSize = RoundCdFileSize(152000);
	var RoundedInstallerHelpDllSize = GetRoundedFileSize(GetInstallerHelpDllFilePath());
	var RoundedAutorunInfSize = RoundCdFileSize(200);
	if (CompiledFilesSizesKnown)
	{
		RoundedSetupExeSize = GetRoundedFileSize(SetupExePath);
		RoundedAutorunInfSize = GetRoundedFileSize(AutorunInfPath);
	}
	var StartFromAnyCD = document.getElementById("StartFromAnyCD").checked;
	
	for (iCd = 0; iCd < MaxNumCds; iCd++)
	{
		if (CdDetails[iCd].CdInUse())
		{
			// Each CD must have setup.exe. This is a change from earlier versions, since it was discovered
			// possible to have an installer on a different CD reboot, leaving a non-startup CD in the drive:
			CdDetails[iCd].NeedsSetupExe = true;
			CdDetails[iCd].TotalSize += RoundedSetupExeSize;
			if (CompiledFilesSizesKnown)
				CdDetails[iCd].DisplayNotes += "Includes actual size of setup.exe\n";
			else
				CdDetails[iCd].DisplayNotes += "Includes estimated size of setup.exe\n";
			
			// See if the current CD is a start-up CD:
			if (iCd == 0 || StartFromAnyCD)
			{
				CdDetails[iCd].NeedsAutorunInf = true;
				CdDetails[iCd].TotalSize += RoundedAutorunInfSize;
				if (CompiledFilesSizesKnown)
					CdDetails[iCd].DisplayNotes += "Includes actual size of Autorun.inf\n";
				else
					CdDetails[iCd].DisplayNotes += "Includes estimated size of Autorun.inf\n";
				
				// We must include InstallerHelp.dll and InstallerHelp2.dll if any product is locked:
				if (IsAnyProductLocked())
				{
					var RoundedInstallerHelp2DllSize = RoundCdFileSize(1600000); // preliminary estimate
					if (CompiledFilesSizesKnown)
					{
						RoundedInstallerHelp2DllSize = GetRoundedFileSize(InstallerHelp2DllPath);
						CdDetails[iCd].DisplayNotes += "Includes actual sizes of InstallerHelp.dll and InstallerHelp2.dll\n";
					}
					else
						CdDetails[iCd].DisplayNotes += "Includes estimated sizes of InstallerHelp.dll and InstallerHelp2.dll\n";
					CdDetails[iCd].NeedsInstallerHelpDll = true;
					CdDetails[iCd].NeedsInstallerHelp2Dll = true;
					CdDetails[iCd].TotalSize += RoundedInstallerHelpDllSize;
					CdDetails[iCd].TotalSize += RoundedInstallerHelp2DllSize;
				}
			
				// We must also include External Help file(s) and Terms of Use file(s), if needed:
				if (TermsOfUseNeeded)
				{
					if (TermsOfUseFileData)
					{
						CdDetails[iCd].TotalSize += TermsOfUseFileData.TermsOfUseSize;
						CdDetails[iCd].DisplayNotes += "Includes actual size of Terms of Use file(s)\n";
					}
					else
						alert("Error - TermsOfUseFileData does not exist, but TermsOfUseSize is needed.");
				}
				if (ExternalHelpNeeded)
				{
					if (ExternalHelpFileData)
					{
						CdDetails[iCd].TotalSize += ExternalHelpFileData.ExternalHelpSize;
						CdDetails[iCd].DisplayNotes += "Includes actual size of External Help file(s)\n";
					}
					else
						alert("Error - ExternalHelpFileData does not exist, but ExternalHelpSize is needed.");
				}
			} // End if this is a start-up CD
		}  // End if this CD is in use
		CdDetails[iCd].DisplayNotes += "Does not include general overhead space requirements.\n";
	}
	showCdRows();
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
	}
	File.Close();
	fso.DeleteFile(TempFilePath);

	ReturnObject = new Object();
	ReturnObject.FileList = FileList;
	ReturnObject.RootFolder = RootFolder;
	return ReturnObject;
}

// Analyzes whether the given path is a relative path or a full path specification.
// If relative, the Path is prepended with a value derived from the ProductsPath node
// in the source XML's MasterInstaller/AutoConfigure section.
function CheckSourceRelativePath(Path)
{
	return CheckProductRelativePath(Path, ProductsPath);
}

// Analyzes whether the given path is a relative path or a full path specification.
// If relative, the Path is prepended with a value derived from the CDsPath node
// in the source XML's MasterInstaller/AutoConfigure section.
function CheckCDsRelativePath(Path)
{
	return CheckProductRelativePath(Path, CDsPath);
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

// Builds an array of file list place-holders for each product.
function InitSourceFileLists()
{
	SourceFileLists = new Array();
  
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		SourceFileLists[iProduct] = new Object();
		SourceFileLists[iProduct].ListData = null;
		SourceFileLists[iProduct].AllFilesFound = false;
	}
}

function GetSourceFileList(iProduct)
{
	if (SourceFileLists[iProduct].ListData == null)
		GenerateSourceFileList(iProduct);
	
	return SourceFileLists[iProduct];
}

// Builds an array of file lists for the specified product, where the files in the list are interpretted
// from the AutoConfigure/Source nodes of each product. (More than one source node per product
// is allowed.)
function GenerateSourceFileList(iProduct)
{
	var FileListData = new Array();
	var AllOk = true;
	try
	{
		var ProductNodeList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
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
				alert("Error - Product " + ProductNode.selectSingleNode('Title').text + " contains an AutoConfigure Source node with a NameWhenLocked attribute (" + NameWhenLocked + ") but multiple files matching.");
			if (NameWhenLocked)
				NewListData.NameWhenLocked = NameWhenLocked;
			NewListData.DestPath = DestPath;
			NewListData.SignAs = SignAs;

			FileListData[iSource] = NewListData;
		}
	}
	catch (err)
	{
		AllOk = false;
		alert("Error while initializing: " + err.description);
	}
	SourceFileLists[iProduct].ListData = FileListData;
	SourceFileLists[iProduct].AllFilesFound = AllOk;
}

// Builds file list each for the External Help file(s) and the Terms of Use file(s),
// from the Edit boxes nodes of each on the Project Setup page.
function GenerateExtHelpAndTermsFileLists()
{
	try
	{
		var ExternalHelpSource = document.getElementById("ExternalHelpSource").value;
		if (ExternalHelpSource.length > 0)
		{
	    ExternalHelpSource = CheckSourceRelativePath(ExternalHelpSource);
			var Recurse = false;
			if (fso.FolderExists(ExternalHelpSource))
				Recurse = true;
			ExternalHelpFileData = GetFileList(ExternalHelpSource, Recurse);
		}
		else
		{
			// Set up empty values:
			ExternalHelpFileData = new Object();
			ExternalHelpFileData.FileList = new Array();
			ExternalHelpFileData.RootFolder = "";
		}

		// Do the same for the Terms of Use file(s):
		var TermsOfUseSource = document.getElementById("TermsOfUseSource").value;
		if (TermsOfUseSource.length > 0)
		{
	    TermsOfUseSource = CheckSourceRelativePath(TermsOfUseSource);
			Recurse = false;
			if (fso.FolderExists(TermsOfUseSource))
				Recurse = true;
			TermsOfUseFileData = GetFileList(TermsOfUseSource, Recurse);
		}
		else
		{
			// Set up empty values:
			TermsOfUseFileData = new Object();
			TermsOfUseFileData.FileList = new Array();
			TermsOfUseFileData.RootFolder = "";
		}
	}
	catch (err)
	{
		alert("Error while initializing: " + err.description);
	}
}

// Initializes an array of total file sizes, one total for each product.
// (Sizes will be rounded to reflect the space they will occupy on a CD.)
function InitAllProductSizes()
{
	ProductSizes = new Array();
  
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		ProductSizes[iProduct] = -1;
	}
	
	// Do the actual calculations for the External Help file(s):
	if (ExternalHelpFileData)
	{
		ExternalHelpFileData.ExternalHelpSize = 0;
		var FileList = ExternalHelpFileData.FileList;
		for (iFile = 0; iFile < FileList.length; iFile++)
		{
			var File = fso.GetFile(FileList[iFile]);
			var ActualSize = File.size;
			ExternalHelpFileData.ExternalHelpSize += RoundCdFileSize(ActualSize);
		}
	}

	// Do the actual calculations for the Terms of Use file(s):
	if (TermsOfUseFileData)
	{
		TermsOfUseFileData.TermsOfUseSize = 0;
		var FileList = TermsOfUseFileData.FileList;
		for (iFile = 0; iFile < FileList.length; iFile++)
		{
			var File = fso.GetFile(FileList[iFile]);
			var ActualSize = File.size;
			TermsOfUseFileData.TermsOfUseSize += RoundCdFileSize(ActualSize);
		}
	}
}

// Builds an array of total file sizes for specified product.
// Sizes are rounded to reflect the space they will occupy on a CD.
function GetProductSize(iProduct)
{
	if (ProductSizes[iProduct] == -1)
	{
		var FileListData = GetSourceFileList(iProduct).ListData;
		var Total = 0;
		for (iData = 0; iData < FileListData.length; iData++)
		{
			var FileList = FileListData[iData].FileList;
			for (iFile = 0; iFile < FileList.length; iFile++)
			{
				var File = fso.GetFile(FileList[iFile]);
				var ActualSize = File.size;
				Total += RoundCdFileSize(ActualSize);
			}
		}
		ProductSizes[iProduct] = Total;
		
		// Deselect and disable the "Included" CD checkbox for any product with no files.
		// This is intended to be for products available on the web.
		if (Total == 0)
		{
			var CdElement = document.getElementById('Included' + iProduct);
			if (CdElement)
			{
				CdElement.checked = false;
				CdElement.disabled = true;
			}
		}
	}
	return ProductSizes[iProduct];
}

// Returns the amount of space needed on a CD for the given single file.
function GetRoundedFileSize(FilePath)
{
  var File = fso.GetFile(FilePath);
	var ActualSize = File.size;
	return RoundCdFileSize(ActualSize);
}

// Writes the total CD file size of each needed product in the relevant table.
function WriteTotalSourceSizes()
{
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		if (IsProductNeeded(iProduct))
		{
			SizeElement = document.getElementById("FileSize" + iProduct);
			SizeElement.innerText = addCommas(GetProductSize(iProduct));
		}
	}
}

// Makes sure the given folder path does not end with a backslash.
function RemoveTrailingBackslash(path)
{
	while (path.slice(-1) == "\\")
		path = path.slice(0, -1);
	return path;
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
	showPage("Stage7", false);
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
		showPage("Stage7", true);
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

// Compiles the setup.exe, autorun.inf, InstallerHelp.dll and InstallerHelp2.dll files,
// just so we can know for certain how big they will be, while looking at the CD allocation
// page.
function PrelimCompile()
{
	showPage('PrelimCompileDiv', false);
	showPage('PrelimCompileActiveDiv', true);
	showPage('Stage5', false);
	
	if (BuildCd(true, true, true, false, false, false, false, false) != 0)
		alert("The build has failed. Estimated values will still be used. Run a full build (from the last configuration page) to get more information about the error.");
	
	showPage('PrelimCompileActiveDiv', false);
	setPageNo(5);
}

// Responds to the user pressing the Go! button. Builds a CD image.
function detailsEntered()
{
	setPageNo(7);
	BuildCd(document.getElementById('WriteXml').checked,
		document.getElementById('CompileHelps').checked,
		document.getElementById('Compile').checked,
		document.getElementById('GatherFiles').checked,
		document.getElementById('BuildIso').checked,
		document.getElementById('BuildSfx').checked,
		true);
}

// Returns the user's choice of output XML file name.
function GetXmlFileName()
{
	var XmlFileName = document.getElementById('XmlFileName').value;
	
	// If user didn't append the .xml extension, we'll do it:
	if (XmlFileName.slice(-4).toLowerCase() != ".xml")
		XmlFileName += ".xml";

	return XmlFileName;
}

// Returns the "project name", which is simply the user's choice of output XML file name,
// minus the .xml extension.
function GetProjectName()
{
	var XmlFileName = GetXmlFileName();
	return XmlFileName.slice(0, -4);
}


// Returns the location of the (compiled) InstallerHelp.dll file.
// This location may be modified by the user.
function GetInstallerHelpDllFilePath()
{
  return fso.BuildPath(CppFilePath, "InstallerHelp.dll");
}

// Deletes the specified file(s), if they exist. Doesn't complain if they don't.
// FilePath can contain wildcards for the filename.
function DeleteIfExists(FilePath)
{
	try
	{
    fso.DeleteFile(FilePath);
	}
	catch(err)
	{
	}
}

// For some unknown reason, the fso.CopyFile() method sometimes fails
// with a "permission denied" error. This method uses DOS to do the file
// copy instead:
function AltCopy(Source, Dest)
{
	var Cmd = 'cmd /Q /D /C copy "' + Source + '" "' + Dest + '"';
  shellObj.Run(Cmd, 0, true);
}

// Builds a complete CD (set) image.
// Params:
//	fWriteXml - true if the reulting XML configuration document is to be saved;
//	fCompileHelps - true if any products are locked and need their setup help embedding in InstallerHelp2.dll;
//	fCompileSetup - true if Setup.exe is to be compiled;
//	fGatherFiles - true if all files for the CD are to be collected and copied to the CD image folder(s);
//	fCreateIso - true if CD image (ISO file) is to be produced. Only works if files are gathered and the miso.exe file in Utils folder is licensed.
//	fCreateSfx - true if self-extracting 7-zip file of gathered files is to be produced. Only works if files are gathered and the 7za.exe and 7zC.sfx files are in the Utils folder.
//	fDisplayCommentary - true if a running commentary is to be displayed during build.
// Note that the order actions needed to build the CD image is not necessarily reflected by the order of paramters.
function BuildCd(fWriteXml, fCompileHelps, fCompileSetup, fGatherFiles, fCreateIso, fCreateSfx, fDisplayCommentary)
{
  if (fDisplayCommentary)
		AddCommentary(0, "Initializing...", true);
	var FinalComment = "All finished successfully.<br/>Don't forget to commit the new CD to the source control repository."; // Optimistic initial value
	var ReturnValue = 0;
	// Everything is surrounded by a try...catch block, so we can trap errors:
	try
	{
		// I can't be bothered to write any more comments in this function.
		// Just read the AddCommentary lines.
		if (fDisplayCommentary)
			AddCommentary(1, "Collecting C++ Path", false);
		if (fDisplayCommentary)
			AddCommentary(1, "Collecting XML Path", false);
		var XmlFileName = GetXmlFileName();
		
		if (fDisplayCommentary)
			AddCommentary(1, "Processing Paths", false);
		var ProjectName = GetProjectName();
		var NewCompilationFolder = fso.BuildPath(InstallerDefinitionsPath, ProjectName);

		// Get a copy of our initial XML document, so we can play with it:
		if (fDisplayCommentary)
			AddCommentary(1, "Creating copy of XML file", false);
		var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
		xmlDoc.async = false;
		MasterXmlDoc.save(xmlDoc);
		if (xmlDoc.parseError.errorCode != 0)
		{
			if (fDisplayCommentary)
				AddCommentary(1, "Error - copied XML file is invalid:\n" + xmlDoc.parseError.reason + "\non line " + xmlDoc.parseError.line + " at position " + xmlDoc.parseError.linepos, true);
			return 1;
		}
		if (fDisplayCommentary)
			AddCommentary(1, "Done.", false);

		if (fCompileHelps)
		{
			// Write new C++ files to embed:
			PrepareInstallerHelp2Dll(xmlDoc, fDisplayCommentary);
		}

		// Make changes to our XML copy, according to user's settings:
		if (fDisplayCommentary)
			AddCommentary(0, "Collecting user settings...", true);
		ApplyUserSettings(xmlDoc);
		if (fDisplayCommentary)
			AddCommentary(1, "Done.", false);

		// Find out where we have to copy stuff to:
		var CdImagePath = document.getElementById('CdImagePath').value;
		CdImagePath = RemoveTrailingBackslash(CdImagePath);
    CdImagePath = CheckCDsRelativePath(CdImagePath);

		if (fWriteXml)
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Writing out new XML file...", true);
				
			// Change processing instruction to refer to XSLT style sheet for creating web downloads:
			for (var i=0; i<xmlDoc.childNodes.length; i++)
			{
				if (xmlDoc.childNodes.item(i).nodeType == 7)
				{
					var pi = xmlDoc.childNodes.item(i);
					if (pi.data.indexOf("ConfigureMasterInstaller") != -1)
						pi.data = "type=\"text/xsl\" href=\"ConfigureWebDownloads.xsl\"";
				}
      }
			//var pi = xmlDoc.createProcessingInstruction("xml-stylesheet", "type=\"text/xsl\" href=\"ConfigureWebDownloads.xsl\"");
			//xmlDoc.insertBefore(pi, xmlDoc.childNodes.item(0));
	 
			// Write out a copy of our new XML document. Note: the save() method does not work from a web browser.
			MakeSureFolderExists(NewCompilationFolder);
			var tso = fso.OpenTextFile(fso.BuildPath(NewCompilationFolder, XmlFileName), 2, true, -1);
			tso.Write(xmlDoc.xml);
			tso.Close();
			
			// If we are to gather files too, we'll write a second copy of the XML file in the
			// 'root' CD folder:
			if (fGatherFiles)
			{
				MakeSureFolderExists(CdImagePath);
				tso = fso.OpenTextFile(fso.BuildPath(CdImagePath, XmlFileName), 2, true);
				tso.Write(xmlDoc.xml);
				tso.Close();
			}

			if (fDisplayCommentary)
				AddCommentary(1, "Done.", false);
		}
		else
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Skipped writing out new XML file.", true);
		}

		if (fCompileSetup)
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Building setup.exe...", true);

      // Save current XmlDoc to temp file:
      MakeSureFolderExists(NewCompilationFolder);
      var tempPath = fso.BuildPath(NewCompilationFolder, "__temp.xml");
			var tso = fso.OpenTextFile(tempPath, 2, true, -1);
			tso.Write(xmlDoc.xml);
			tso.Close();
      
      // Build setup.exe:
      SetupExePath = CompileMasterInstaller(tempPath);
      
      // Delete temp file:
    	fso.DeleteFile(tempPath);

      // Test that setup.exe exists:
			if (!fso.FileExists(SetupExePath))
			{
				Exception = new Object();
				Exception.description = "Master installer [setup.exe] failed to compile and link.";
				throw(Exception);
			}

			if (fDisplayCommentary)
				AddCommentary(1, "Done.", false);

			// Generate an autorun.inf:
			if (fDisplayCommentary)
				AddCommentary(0, "Generating Autorun.inf", true);
			WriteAutorunInfFile(NewCompilationFolder);
		}

		if (fCompileSetup && fCompileHelps)
			CompiledFilesSizesKnown = true;

		if (fGatherFiles)
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Gathering files to disk image folder...", true);
			GatherFiles(CdImagePath);
			if (fDisplayCommentary)
				AddCommentary(1, "Done.", false);
		}
		if (fCreateIso)
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Creating ISO CD image(s)...", true);

			// Iterate over every CD:
			for (iCd = 0; iCd < MaxNumCds; iCd++)
			{
				// Only bother with CDs known to be needed:
				if (CdDetails[iCd].CdInUse())
				{
					// Generate full folder path to the current CD image:
					var CdFolder = GetCdFolderPath(CdImagePath, iCd);
					
					var UDF_arg = "";
					if (document.getElementById("UseUDF").checked)
						UDF_arg = " -UDF";
          shellObj.Run('wscript.exe "' + fso.BuildPath(UtilsPath, "CdImage.js") + '" "' + CdFolder + '"' + UDF_arg, 0, true);

					if (fDisplayCommentary)
						AddCommentary(1, "Done.", false);
				}
			}
		}
		if (fCreateSfx)
		{
			if (fDisplayCommentary)
				AddCommentary(0, "Creating self extracting 7-zip file.<br/><b>This will continue to run after the 'all finished' message.</b>", true, true);

			// Get path to the 7za.exe and 7zC.sfx files in the Utils folder:
			var ExePath = fso.BuildPath(UtilsPath, "7za.exe");
			var SfxPath = fso.BuildPath(UtilsPath, "7zC.sfx");

			// Iterate over every CD:
      for (iCd = 0; iCd < MaxNumCds; iCd++)
			{
				// Only bother with CDs known to be needed:
				if (CdDetails[iCd].CdInUse())
				{
					var CdFolder = GetCdFolderPath(CdImagePath, iCd);
					shellObj.CurrentDirectory = CdImagePath;
					var Cmd = '"' + ExePath + '" a "-sfx' + SfxPath + '" "' + GetCdLabel(iCd) + '.exe" "' + GetCdLabel(iCd) + '\\*" -r -mx=9 -mmt=on';
					shellObj.Run(Cmd, 1, false);
				}
			}

			if (fDisplayCommentary)
				AddCommentary(1, "Done.", false);
		}
	}
	catch(err)
	{
		FinalComment = "Error: " + (err.number & 0xFFFF) + "\n" + err.description;
		ReturnValue = 1;
	}
	if (fDisplayCommentary)
	{
		AddCommentary(0, "<h2>" + FinalComment + "</h2>", true, true);
		var PanicStopButton = document.getElementById("PanicStopButton");
		PanicStopButton.disabled = true;
		PrevButton.disabled = false;
	}
	return ReturnValue;
}

function CompileMasterInstaller(XmlFileName)
{
  var Cmd = 'wscript.exe "' + fso.BuildPath(UtilsPath, "CompileXmlMasterInstaller.js") + '" "' + XmlFileName + '" "-certificatepassword:' + document.getElementById('CertificatePassword').value + '"';
  if (document.getElementById("IncludeEasterEggs").checked)
    Cmd += " -E";
  shellObj.Run(Cmd, 0, true);
 	iLastBackslash = XmlFileName.lastIndexOf("\\");
	var InputFolder = XmlFileName.substr(0, iLastBackslash);
  var SetupExePath = InputFolder + "\\setup.exe";
  return SetupExePath;
}

// Returns true if the specified product is needed on a CD and is to be locked.
function IsProductLocked(ProductIndex)
{
	if (IsProductNeeded(ProductIndex))
	{
		var UseKeyElement = document.getElementById('UseKey' + ProductIndex);
		if (UseKeyElement)
			return UseKeyElement.checked;
	}
	return false;
}

// Returns true if any needed product is locked.
function IsAnyProductLocked()
{
	for (i = 0; i < NumProducts; i++)
		if (IsProductLocked(i))
			return true;
	return false;
}

// Embeds the setup help files (for any Key-protected products) in a file called
// InstallerHelp2.dll.
function PrepareInstallerHelp2Dll(xmlDoc, fDisplayCommentary)
{
	if (!IsAnyProductLocked())
	{
		if (fDisplayCommentary)
			AddCommentary(0, "Skipping crunching of help files - no product is locked.", true);
		return;
	}
	if (fDisplayCommentary)
		AddCommentary(0, "Crunching help files...", true);

  var HelpFileCruncher = fso.BuildPath(UtilsPath, "Bin2Cpp.exe");

	// Check that HelpFileCruncher exists:
	if (!fso.FileExists(HelpFileCruncher))
	{
		if (fDisplayCommentary)
			AddCommentary(1, "Error - Help File Cruncher '" + HelpFileCruncher + "' does not exist.", false);
		return;
	}

	// Check that CppFilePath exists:
	if (!fso.FolderExists(CppFilePath))
	{
		if (fDisplayCommentary)
			AddCommentary(1, "Error - InstallerHelp2 C++ files folder '" + CppFilePath + "' does not exist.", false);
		return;
	}

	// Create a new FileList.cpp file:
	var FileListCpp = fso.OpenTextFile(fso.BuildPath(CppFilePath, "FileList.cpp"), 2, true);
	// Create a new Helps.cpp file:
	var HelpsCpp = fso.OpenTextFile(fso.BuildPath(CppFilePath, "Helps.cpp"), 2, true);

	// Set up some objects to be used inside loop:
	var ProductList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
	var NewProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');

	// Determine how we are to handle relative paths:
	var RelativePathPrepend = ProductsPath;

	// Iterate over every locked product:
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		if (IsProductLocked(iProduct))
		{
			var ProductNode = ProductList[iProduct];
			if (fDisplayCommentary)
				AddCommentary(0, ProductNode.selectSingleNode("Title").text, true);
			var HelpNode = ProductNode.selectSingleNode("AutoConfigure/SetupHelp");
			if (HelpNode)
			{
				// Check if source path is relative:
				var HelpFile = CheckProductRelativePath(HelpNode.text, RelativePathPrepend);

				if (!fso.FileExists(HelpFile))
				{
					if (fDisplayCommentary)
						AddCommentary(1, "Error - Help File '" + HelpFile + "' does not exist.", false);
				}
				else
				{						
					// Make copy of help file name with no extension and all spaces
					// replaced with underscores:
					var HelpFileName = fso.GetFileName(HelpFile);
					var HelpFileTag = HelpFileName;
					iDot = HelpFileTag.lastIndexOf(".");
					if (iDot > 0)
						HelpFileTag = HelpFileTag.slice(0, iDot);
					HelpFileTag = HelpFileTag.replace(/ /g, "_");
					
					var OutputFile = "Product" + iProduct;
					var OutputFileWithExtn = OutputFile + ".cpp";
					var OutputPath = fso.BuildPath(CppFilePath, OutputFileWithExtn);
					
					if (fDisplayCommentary)
						AddCommentary(1, HelpFile, false);
					var CmdCrunch = '"' + HelpFileCruncher + '" "' + HelpFile + '" "' + OutputPath + '"';
          shellObj.Run(CmdCrunch, 7, true);

					FileListCpp.WriteLine('#include "' + fso.GetFileName(OutputPath) + '"');
					
					HelpsCpp.WriteLine('if (_tcscmp(pszStem, _T("' + HelpFileTag + '")) == 0)');
					HelpsCpp.WriteLine('{');
					HelpsCpp.WriteLine('	pbBuffer = rgb' + HelpFileTag + ';');
					HelpsCpp.WriteLine('	cbSize = sizeof(rgb' + HelpFileTag + ') / sizeof(rgb' + HelpFileTag + '[0]);');
					HelpsCpp.WriteLine('}');

					// Set new help tag in XML:
					var NewProductNode = NewProductNodeList[iProduct];
					var NewHelpNode = NewProductNode.selectSingleNode("Help");
					NewHelpNode.text = HelpFileName;
					NewHelpNode.setAttribute("Internal", "true");
				}
			}
			else
			{
				if (fDisplayCommentary)
					AddCommentary(1, "No help file specified", false);
			}
		} // End if current product is locked
	} // Next product

	HelpsCpp.Close();
	FileListCpp.Close();
	if (fDisplayCommentary)
		AddCommentary(1, "Done.", true);

	// Create a new folder for compilation output, like a Release folder, but named after the output XML File:
	if (fDisplayCommentary)
		AddCommentary(0, "Preparing folder for InstallerHelp2.dll compilation...", true);
	var ProjectName = GetProjectName();
	var NewCompilationFolder = fso.BuildPath(InstallerDefinitionsPath, ProjectName);
	MakeSureFolderExists(NewCompilationFolder);
	if (fDisplayCommentary)
		AddCommentary(1, "Done.", false);

	// Prepare the file containing all the compilation settings:
	if (fDisplayCommentary)
		AddCommentary(0, "Preparing file for compiler settings...", true);
	var CppRspFilePath = NewCompilationFolder + "\\" + ProjectName + "Cpp.rsp";

	var tsoCpp = fso.OpenTextFile(CppRspFilePath, 2, true);
	tsoCpp.WriteLine('/O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "INSTALLERHELP2_EXPORTS" /D "_VC80_UPGRADE=0x0710" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /FD /EHsc /MT /GS /W3 /c /Zi /TP /Fo"' + NewCompilationFolder + '\\InstallerHelp2.obj"');
	tsoCpp.WriteLine('"' + CppFilePath + '\\InstallerHelp2.cpp"');
	tsoCpp.Close();

	if (fDisplayCommentary)
		AddCommentary(1, "Done.", false);

	// Compile InstallerHelp2 C++ files:
	if (fDisplayCommentary)
		AddCommentary(0, "Compiling InstallerHelp2 C++ files...", true);
	var CompileStr = 'cl.exe @"' + CppRspFilePath + '" /nologo';
  if (shellObj.Run(CompileStr, 7, true) != 0)
	{
		Exception = new Object();
		Exception.description = "InstallerHelp2.dll compile command [" + CompileStr + "] failed.";
		throw(Exception);
	}
	if (fDisplayCommentary)
		AddCommentary(1, "Done.", false);

	// Prepare the file containing all the linker settings:
	if (fDisplayCommentary)
		AddCommentary(0, "Preparing file for linker settings...", true);
	InstallerHelp2DllPath = NewCompilationFolder + "\\InstallerHelp2.dll";
	var ObjRspFilePath = NewCompilationFolder + "\\" + ProjectName + "Obj.rsp";
	var tsoObj = fso.OpenTextFile(ObjRspFilePath, 2, true);
	tsoObj.WriteLine('/OUT:"' + InstallerHelp2DllPath + '" /INCREMENTAL:NO /NOLOGO /DLL /MANIFEST /MANIFESTFILE:"' + NewCompilationFolder + '\\InstallerHelp2.dll.intermediate.manifest" /DEF:"' + CppFilePath + '\\InstallerHelp2.def" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /IMPLIB:"' + NewCompilationFolder + '/InstallerHelp2.lib" /MACHINE:X86  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib');
	
	tsoObj.WriteLine('"' + NewCompilationFolder + '\\InstallerHelp2.obj"');
	tsoObj.Close();
	if (fDisplayCommentary)
		AddCommentary(1, "Done.", false);

	// Link the obj files to produce the InstallerHelp2.dll:
	if (fDisplayCommentary)
		AddCommentary(0, "Linking compiled files...", true);
	var LinkStr = 'link.exe @"' + ObjRspFilePath + '"'; 
  
  if (shellObj.Run(LinkStr, 7, true) != 0)
	{
		Exception = new Object();
		Exception.description = "InstallerHelp2.dll link command [" + LinkStr + "] failed.";
		throw(Exception);
	}

	// Test that InstallerHelp2.dll exists:
	if (!fso.FileExists(fso.BuildPath(NewCompilationFolder, "InstallerHelp2.dll")))
	{
		Exception = new Object();
		Exception.description = "InstallerHelp2.dll failed to compile and link.";
		throw(Exception);
	}

	if (fDisplayCommentary)
		AddCommentary(1, "Done.", true);
}

// Applies the text in the given text box element to the node in xmlDoc given by XPath.
function ApplyEditBoxSetting(xmlDoc, ElementId, XPath)
{
	var Text = document.getElementById(ElementId).value;
	var Node = xmlDoc.selectSingleNode(XPath);
	Node.text = Text;
}

// Applies the boolean in the given checkbox element to the node in xmlDoc given by XPath.
function ApplyCheckBoxSetting(xmlDoc, ElementId, XPath)
{
	var Checked = document.getElementById(ElementId).checked;
	var Node = xmlDoc.selectSingleNode(XPath);
	Node.text = Checked ? "true" : "false";
}

// Make changes to an XML documemt, according to user's settings:
function ApplyUserSettings(xmlDoc)
{
	// Settings from Project Setup:
	ApplyEditBoxSetting(xmlDoc, "CdImagePath", "/MasterInstaller/AutoConfigure/CdImagePath");
	
	ApplyEditBoxSetting(xmlDoc, "ExternalHelpSource", "/MasterInstaller/AutoConfigure/ExternalHelpSource");
	ApplyEditBoxSetting(xmlDoc, "ExternalHelpFileDest", "/MasterInstaller/AutoConfigure/ExternalHelpDestination");
	if (document.getElementById("ExternalHelpFile").value.length < 1 || document.getElementById("HelpButtonText").value.length < 1)
	{
		xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpSource").text = "";
		xmlDoc.selectSingleNode("/MasterInstaller/AutoConfigure/ExternalHelpDestination").text = "";
	}

	// Settings from General:
	ApplyEditBoxSetting(xmlDoc, "OverallTitle", "/MasterInstaller/General/Title")
	ApplyEditBoxSetting(xmlDoc, "ListSubtitle", "/MasterInstaller/General/ListSubtitle")

	// For the product selection dialog background, put the bitmap file name as the main value,
	// and translate the color selection into our RGB values as atrributes:
	var BmpList = document.getElementById("ListBackgroundBmp");
	if (BmpList.value != "None")
	{
		var Node = xmlDoc.selectSingleNode("/MasterInstaller/General/ListBackground");
		Node.text = BmpList.value;
	}
	var ColorSelectionElement = document.getElementById("ListBackground");
	var rgbRed = 0;
	var rgbGreen = 0;
	var rgbBlue = 0;
	switch(ColorSelectionElement.value)
	{
		case "Green":
			rgbRed = 196;
			rgbGreen = 206;
			rgbBlue = 44;
			break;
		case "Yellow":
			rgbRed = 255;
			rgbGreen = 242;
			rgbBlue = 0;
			break;
		case "Red":
			rgbRed = 204;
			rgbGreen = 57;
			rgbBlue = 33;
			break;
		case "Blue":
			rgbRed = 0;
			rgbGreen = 103;
			rgbBlue = 166;
			break;
		case "Black":
			rgbRed = 0;
			rgbGreen = 0;
			rgbBlue = 1; // zero used as "no background", so use kludge
			break;
		case "White":
			rgbRed = 255;
			rgbGreen = 255;
			rgbBlue = 255;
			break;
	}
	var BackgroundNode = xmlDoc.selectSingleNode("/MasterInstaller/General/ListBackground");
	if (rgbRed != 0 || rgbGreen != 0 || rgbBlue != 0)
	{
		BackgroundNode.setAttribute("Red", rgbRed);
		BackgroundNode.setAttribute("Green", rgbGreen);
		BackgroundNode.setAttribute("Blue", rgbBlue);
	}
	BackgroundNode.setAttribute("OffsetX", document.getElementById("OffsetX").value);
	BackgroundNode.setAttribute("OffsetY", document.getElementById("OffsetY").value);

	BackgroundNode.setAttribute("BlendLeft", document.getElementById("BlendLeft").value);
	BackgroundNode.setAttribute("BlendRight", document.getElementById("BlendRight").value);
	BackgroundNode.setAttribute("BlendTop", document.getElementById("BlendTop").value);
	BackgroundNode.setAttribute("BlendBottom", document.getElementById("BlendBottom").value);

	ApplyEditBoxSetting(xmlDoc, "InitialTextMessage", "/MasterInstaller/General/InitialText");
	var InitialNode = xmlDoc.selectSingleNode("/MasterInstaller/General/InitialText");
	var InitialTextAlignSelectionElement = document.getElementById("InitialTextAlignment");
	var InitialTextAlignment;
	switch(InitialTextAlignSelectionElement.value)
	{
		case "InitialTextLeftEdge":
			InitialTextAlignment = "left";
			break;
		case "InitialTextButtons":
			InitialTextAlignment = "buttons";
			break;
		case "InitialTextCheckBoxes":
			InitialTextAlignment = "checkboxes";
			break;
	}
	InitialNode.setAttribute("Align", InitialTextAlignment);
	InitialNode.setAttribute("LeftEdge", document.getElementById("InitialTextLeftOffset").value);
	InitialNode.setAttribute("RightEdge", document.getElementById("InitialTextRightOffset").value);

	ApplyCheckBoxSetting(xmlDoc, "ListEvenOneProduct", "/MasterInstaller/General/ListEvenOneProduct");
	ApplyCheckBoxSetting(xmlDoc, "ShowInstallCompleteMessage", "/MasterInstaller/General/ShowInstallCompleteMessage");
	ApplyEditBoxSetting(xmlDoc, "ListSpacingAdjust", "/MasterInstaller/General/ListSpacingAdjust")
	ApplyEditBoxSetting(xmlDoc, "InfoButtonAdjust", "/MasterInstaller/General/InfoButtonAdjust")
	ApplyCheckBoxSetting(xmlDoc, "StartFromAnyCD", "/MasterInstaller/General/StartFromAnyCD");
	ApplyCheckBoxSetting(xmlDoc, "KeyPromptNeedsShiftCtrl", "/MasterInstaller/General/KeyPromptNeedsShiftCtrl");
	ApplyEditBoxSetting(xmlDoc, "GetKeyTitle", "/MasterInstaller/General/GetKeyTitle")
	ApplyEditBoxSetting(xmlDoc, "ExternalHelpFile", "/MasterInstaller/General/ExternalHelpFile")
	ApplyEditBoxSetting(xmlDoc, "HelpButtonText", "/MasterInstaller/General/HelpButtonText")
	ApplyEditBoxSetting(xmlDoc, "TermsOfUseFile", "/MasterInstaller/General/TermsOfUseFile")
	ApplyEditBoxSetting(xmlDoc, "TermsButtonText", "/MasterInstaller/General/TermsButtonText")

	// Settings from CD Allocation:
	var CdsNode = xmlDoc.createElement("CDs");
	for (iCd = 0; iCd < MaxNumCds; iCd++)
	{
		if (iCd == 0 || CdDetails[iCd].CdInUse())
		{
			var CdNode = xmlDoc.createElement("CD");
			
			var TitleNode = xmlDoc.createElement("Title");
			TitleNode.text = document.getElementById("CdTitle" + iCd).value;
			CdNode.appendChild(xmlDoc.createTextNode("\n\t\t\t"));
			CdNode.appendChild(TitleNode);

			var LabelNode = xmlDoc.createElement("VolumeLabel");
			LabelNode.text = document.getElementById("CdLabel" + iCd).value;
			CdNode.appendChild(xmlDoc.createTextNode("\n\t\t\t"));
			CdNode.appendChild(LabelNode);			

			CdNode.appendChild(xmlDoc.createTextNode("\n\t\t"));

			CdsNode.appendChild(xmlDoc.createTextNode("\n\t\t"));
			CdsNode.appendChild(CdNode);
		}
	}
	CdsNode.appendChild(xmlDoc.createTextNode("\n\t"));
	
	var MasterInstallerNode = xmlDoc.selectSingleNode('/MasterInstaller');
	MasterInstallerNode.appendChild(xmlDoc.createTextNode("\n\t"));
	MasterInstallerNode.appendChild(CdsNode);
	MasterInstallerNode.appendChild(xmlDoc.createTextNode("\n"));

	// Individual products' CD allocation:
	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
	{
		if (IsProductNeeded(iProduct))
		{
			var ProductNode = ProductNodeList[iProduct];
			var CdIndex = GetCdIndexOfProduct(iProduct); // This could be -1 if product omitted.
			var CdNode = xmlDoc.createElement("CD");
			CdNode.text = CdIndex;
			// Remove any previous CD nodes:
			var PreviousCdNodes = ProductNode.selectNodes("CD");
			PreviousCdNodes.removeAll();
			// Add new CD node:
			ProductNode.appendChild(CdNode);
			ProductNode.appendChild(xmlDoc.createTextNode("\n\t\t"));

			// If product is locked, there may be some substitutions specified:
			if (IsProductLocked(iProduct))
			{
				var AutoConfigureNode = ProductNode.selectSingleNode('AutoConfigure');
				if (AutoConfigureNode)
				{				
					// See if there is a new Destination folder specified.
					// This swap must be done in the original XML structure, as that is
					// where the destination is read from when files are copied:
					var OrigProductNodeList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
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
		} // End if current product is needed
	} // Next product

	// Settings from Product Selection:
	// Finally, remove nodes where product not needed. This means that the 
	// new xmlDoc and the original MasterXmlDoc will be out of sync from
	// now on, in terms of product numbers etc.
	var ProductNodeList = xmlDoc.selectNodes('/MasterInstaller/Products/Product');
	for (i = 0; i < NumProducts; i++)
	{
		if (IsProductNeeded(i))
		{
			var ProductNode = ProductNodeList.peekNode();
			// Make sure selected products get listed to user:
			if (document.getElementById('ProductTitle' + i).checked)
				ProductNode.setAttribute("List", "true");

			// Make sure key-protected products are only protected if so configured in this tool:
			if (!IsProductLocked(i))
				ProductNode.removeAttribute("KeyId");

			ProductNodeList.nextNode();
		}
		else
		{
			ProductNodeList.removeNext();
		}
	}
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

// Writes an Autorun.inf file, based on user's settings.
function WriteAutorunInfFile(TargetFolder)
{
  AutorunInfPath = fso.BuildPath(TargetFolder, "Autorun.inf");
	var tso = fso.OpenTextFile(AutorunInfPath, 2, true);
	tso.WriteLine('[autorun]');
	tso.WriteLine('icon=setup.exe');
	tso.WriteLine('open=setup.exe -autorun');
	tso.WriteLine('shell\setup=&Install ' + document.getElementById("OverallTitle").value);
	tso.WriteLine('shell\setup\command=setup.exe');
	tso.WriteLine('label=' + document.getElementById("OverallTitle").value);
	tso.Close();
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

// Fetch label for given CD:
function GetCdLabel(CdIndex)
{
	CdLabelElement = document.getElementById('CdLabel' + CdIndex);
	var CdLabel = CdLabelElement.value;
	if (CdLabel == "")
		CdLabel = CdIndex;
	return CdLabel;
}

// Generate full folder path to the given CD's image:
function GetCdFolderPath(CdImagePath, CdIndex)
{
	var CdFolder = CdImagePath;
	CdFolder += "\\" + GetCdLabel(CdIndex);
	return CdFolder;
}

// Copies files from their various sources to the CD image set.
function GatherFiles(CdImagePath)
{
	// Find out how many bytes we need to copy, for our progress indicator:
	var TotalBytesToCopy = 0;
	for (iProduct = 0; iProduct < NumProducts; iProduct++)
		if (IsProductNeeded(iProduct))
			TotalBytesToCopy += GetProductSize(iProduct);
	var TotalDone = 0;

	var StartFromAnyCD = document.getElementById("StartFromAnyCD").checked;
  
	// Determine how we are to handle certain relative paths:
	var RelativePathPrepend = ProductsPath;

	// Iterate over every CD:
	for (iCd = 0; iCd < MaxNumCds; iCd++)
	{
		// Only bother with CDs known to be needed:
		if (CdDetails[iCd].CdInUse())
		{
			// Generate full folder path to the current CD image:
			var CdFolder = GetCdFolderPath(CdImagePath, iCd);
			AddCommentary(0, "Disk: " + iCd, true);
			CdFolder += "\\";
			MakeSureFolderExists(CdFolder);

			// See if we need Setup.exe and autorun.inf files:
			if (CdDetails[iCd].NeedsSetupExe)
			{
				AddCommentary(1, "Copying setup.exe", false);
				if (SetupExePath != null)
					fso.CopyFile(SetupExePath, CdFolder, true);
				else
					AddCommentary(0, "Error - path of setup.exe not defined", true);
			}
			if (CdDetails[iCd].NeedsAutorunInf)
			{
				AddCommentary(1, "Copying Autorun.inf", false);
				if (AutorunInfPath != null)
					fso.CopyFile(AutorunInfPath, CdFolder, true);
				else
					AddCommentary(0, "Error - path of Autorun.inf not defined", true);
			}
			// See if we need InstallerHelp.dll:
			if (CdDetails[iCd].NeedsInstallerHelpDll)
			{
				AddCommentary(1, "Copying InstallerHelp.dll", false);
				fso.CopyFile(GetInstallerHelpDllFilePath(), CdFolder, true);
			}			
			
			// See if we need InstallerHelp2.dll:
			var UsingInstallerHelp2Dll = (IsAnyProductLocked() && (iCd == 0 || StartFromAnyCD));
			if (UsingInstallerHelp2Dll)
			{
				AddCommentary(1, "Copying InstallerHelp2.dll", false);
				fso.CopyFile(InstallerHelp2DllPath, CdFolder, true);
			}

			// See if we need External Help file(s):
			if (ExternalHelpNeeded && (iCd == 0 || StartFromAnyCD))
			{
				AddCommentary(1, "Copying External Help file(s)", false);
				if (!ExternalHelpFileData)
				{
					Exception = new Object();
					Exception.description = "External Help is required, but no source file data exists.";
					throw(Exception);
				}
				var FileList = ExternalHelpFileData.FileList;
				var RootFolder = ExternalHelpFileData.RootFolder;
				var Destination = CdFolder;
				if (!document.getElementById("ExternalHelpFileDest").disabled)
					Destination = fso.BuildPath(CdFolder, document.getElementById("ExternalHelpFileDest").value);
				for (i = 0; i < FileList.length; i++)
				{
					var SourcePath = FileList[i];
					var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, Destination);
					MakeSureFolderExists(TargetFullPath);
					TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

					// For some unknown reason, the fso.CopyFile() method fails
					// with a "permission denied" error, so we'll use our alternate
					// function instead:
					AltCopy(FileList[i], TargetFullPath);
					//fso.CopyFile(FileList[i], Destination, true);

					if (!fso.FileExists(TargetFullPath))
						AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
				} // Next file
			}

			// See if we need Terms of Use file(s):
			if (TermsOfUseNeeded && (iCd == 0 || StartFromAnyCD))
			{
				AddCommentary(1, "Copying Terms of Use file(s)", false);
				if (!TermsOfUseFileData)
				{
					Exception = new Object();
					Exception.description = "Terms of Use file(s) is required, but no source file data exists.";
					throw(Exception);
				}
				var FileList = TermsOfUseFileData.FileList;
				var RootFolder = TermsOfUseFileData.RootFolder;
				var Destination = CdFolder;
				if (!document.getElementById("TermsOfUseFileDest").disabled)
					Destination = fso.BuildPath(CdFolder, document.getElementById("TermsOfUseFileDest").value);
				for (i = 0; i < FileList.length; i++)
				{
					var SourcePath = FileList[i];
					var TargetFullPath = GetDestinationFolder(SourcePath, RootFolder, CdFolder);
					MakeSureFolderExists(TargetFullPath);
					TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

					// For some unknown reason, the fso.CopyFile() method fails
					// with a "permission denied" error, so we'll use our alternate
					// function instead:
					AltCopy(FileList[i], TargetFullPath);
					//fso.CopyFile(FileList[i], Destination, true);

					if (!fso.FileExists(TargetFullPath))
						AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
				} // Next file
			}

			// Copy all other files:
			for (iProduct = 0; iProduct < NumProducts; iProduct++)
			{
				if (IsProductNeeded(iProduct))
				{
					if (GetCdIndexOfProduct(iProduct) == iCd)
					{
						var ProductNodeList = MasterXmlDoc.selectNodes('/MasterInstaller/Products/Product');
						var ProductNode = ProductNodeList[iProduct];
						var ProductTitle = ProductNode.selectSingleNode("Title").text;
						AddCommentary(1, "[" + Math.round((100 * TotalDone / TotalBytesToCopy)) + "%] Copying " + ProductTitle + " files", false);
						var Destination = CdFolder + ProductNode.selectSingleNode('AutoConfigure/Destination').text;
						var FileListData = GetSourceFileList(iProduct).ListData;
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
								if (IsProductLocked(iProduct))
									if (Substitution)
										UsingSub = true;

								if (UsingSub)
									TargetFullPath = fso.BuildPath(TargetFullPath, Substitution);
								else
									TargetFullPath = fso.BuildPath(TargetFullPath, fso.GetFileName(SourcePath));

								// For some unknown reason, the fso.CopyFile() method fails
								// with a "permission denied" error, so we'll use our alternate
								// function instead:
								AltCopy(FileList[i], TargetFullPath);
								//fso.CopyFile(FileList[i], Destination, true);
								
								if (SignAs != null)
								{
									// Sign the current file:
									var Cmd = '"' + fso.BuildPath(UtilsPath, "SignMaster.exe") + '" "' + TargetFullPath + '" -d "' + SignAs + '" -p "' + document.getElementById('CertificatePassword').value + '"';
									shellObj.Run(Cmd, 1, true);
								}

								if (!fso.FileExists(TargetFullPath))
									AddCommentary(0, "Error - could not copy from\n" + FileList[i] + " to\n" + TargetFullPath, true);
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
								var HelpSource = CheckProductRelativePath(SetupHelpNode.text, RelativePathPrepend);

								// For some unknown reason, the fso.CopyFile() method fails
								// with a "permission denied" error, so we'll use our alternate
								// function instead:
								AltCopy(HelpSource, CdFolder + SetupHelpTargetNode.text);
							}
						}

						TotalDone += GetProductSize(iProduct);
					} // End if current product goes on current CD
				} // End if current product is needed
			} // Next product
		} // End if current CD is in use
		AddCommentary(1, "Done.", false);
	} // Next CD
}

-->
]]>
		</script>
	</xsl:template>
</xsl:stylesheet>
