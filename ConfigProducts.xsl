<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<!--
Converts an XML definition of a master installer into C++ that can be compiled with the "setup"
project to make an executable master installer.
-->

<!-- Introductory warning message -->
<xsl:template match="/">
	<xsl:text>/*&#13;</xsl:text>
	<xsl:text>&#09;This file is ALWAYS PRODUCED AUTOMATICALLY.&#13;</xsl:text>
	<xsl:text>&#09;Do not edit it, as it may be overwritten without warning.&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>
	<xsl:text>&#09;The source for producing this file is an XML document.&#13;</xsl:text>
	<xsl:text>*/&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<!-- Declare external functions -->
	<xsl:text>// External functions:&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product/TestPresence">
		<xsl:if test="not(@Type='External')">
			<xsl:text>extern bool </xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion, const _TCHAR * pszCriticalFile);&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="MasterInstaller/Products/Product/Preinstall">
		<xsl:if test="not(@Type='External') and not(@IgnoreError='true')">
			<xsl:text>extern int </xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>(const _TCHAR * pszCriticalFile);&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="MasterInstaller/Products/Product/Install">
		<xsl:if test="@Type='Internal'">
			<xsl:text>extern DWORD </xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>(bool fFlag, const _TCHAR * pszCriticalFile);&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="MasterInstaller/Products/Product/PostInstall">
		<xsl:if test="not(@Type='External')">
			<xsl:text>extern int </xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>(const _TCHAR * pszCriticalFile);&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:text>&#13;</xsl:text>

	<!-- Product configuration -->
	<xsl:text>// List of software products in this package.&#13;</xsl:text>
	<xsl:text>static SoftwareProduct Products[] =&#13;</xsl:text>
	<xsl:text>{&#13;</xsl:text>

	<xsl:for-each select="MasterInstaller/Products/Product">
		<xsl:text>&#09;{&#13;</xsl:text>
		
		<xsl:variable name="ProductCode" select="ProductCode"/>

		<!-- Product Tag, used in dependency lists -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="Tag"/>
		</xsl:call-template>
		<xsl:text>, // Name used in dependency lists&#13;</xsl:text>

		<!-- Nice name of product, displayed to user -->
		<xsl:text>&#09;&#09;_T("</xsl:text>
		<xsl:value-of select="Title"/>
		<xsl:text>"), // Title&#13;</xsl:text>

		<!-- MSI Product Code -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="$ProductCode"/>
		</xsl:call-template>
		<xsl:text>, // MSI Product Code&#13;</xsl:text>

		<!-- MSI Feature - Always NULL in main products -->
		<xsl:text>&#09;&#09;NULL, // MSI Feature name&#13;</xsl:text>

		<!-- Version number of product, used in Product Key tests -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="@Version"/>
		</xsl:call-template>
		<xsl:text>, // Version for Product Key test&#13;</xsl:text>

		<!-- Product Key ID number (or -1 if not locked) -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="ReplaceEmpty">
			<xsl:with-param name="str" select="@KeyId"/>
			<xsl:with-param name="replace" select="'-1'"/>
		</xsl:call-template>
		<xsl:text>, // Product Key ID&#13;</xsl:text>

		<!-- Flag which dtermines if user gets to select product -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="@List"/>
		</xsl:call-template>
		<xsl:text>, // Flag saying if product should be offered to user&#13;</xsl:text>

		<!-- Critical file, with optional condition flag -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="not(string-length(CriticalFile/@Flag)=0)">
			<xsl:text>&amp;</xsl:text>
			<xsl:value-of select="CriticalFile/@Flag"/>
			<xsl:text>, // Critical file condition flag&#13;</xsl:text>
			<xsl:text>&#09;&#09;_T("</xsl:text>
			<xsl:call-template name="BackslashDoubler">
				<xsl:with-param name="str" select="CriticalFile/True"/>
			</xsl:call-template>
			<xsl:text>"), // Critical file condition true&#13;</xsl:text>
			<xsl:text>&#09;&#09;_T("</xsl:text>
			<xsl:call-template name="BackslashDoubler">
				<xsl:with-param name="str" select="CriticalFile/False"/>
			</xsl:call-template>
			<xsl:text>")</xsl:text>
		</xsl:if>
		<xsl:if test="string-length(CriticalFile/@Flag)=0">
			<xsl:text>NULL, // Critical file condition flag&#13;</xsl:text>
			<xsl:text>&#09;&#09;_T("</xsl:text>
			<xsl:call-template name="BackslashDoubler">
				<xsl:with-param name="str" select="CriticalFile"/>
			</xsl:call-template>
			<xsl:text>"), // Critical file condition true&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL</xsl:text>
		</xsl:if>
		<xsl:text>, // Critical file condition false&#13;</xsl:text>

		<!-- CD Index -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:value-of select="CD"/>
		<xsl:text>, // CD index (zero-based)&#13;</xsl:text>

		<!-- Flag to say if user needs Windows 2000 or better, in order to install this product -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="MustHaveWin2kOrBetter"/>
		</xsl:call-template>
		<xsl:text>, // MustHaveWin2kOrBetter&#13;</xsl:text>

		<!-- Flag to say if user needs admin rights, in order to install this product -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="MustBeAdmin"/>
		</xsl:call-template>
		<xsl:text>, // MustBeAdmin&#13;</xsl:text>

		<!-- Flag to say if pending reboot must be executed first -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="FlushReboot"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if pending reboot must be executed first&#13;</xsl:text>

		<!-- Flag to say if product needs the PendingFileRenameOperations registry setting to be used in detecting a pending reboot. -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="RebootTestRegPending"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if product needs the PendingFileRenameOperations registry setting to be used in detecting a pending reboot&#13;</xsl:text>

		<!-- Flag to say if product needs to detect the presence of file wininit.ini when testing for a pending reboot. -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="RebootTestWininit"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if product needs to detect the presence of file wininit.ini when testing for a pending reboot&#13;</xsl:text>

		<!-- Flag to say if hanging windows must be killed before the product is installed. -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="KillHangingWindows"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if hanging windows must be killed before the product is installed.&#13;</xsl:text>

		<!-- Preinstallation function -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="Preinstall/@Type='External'">
			<xsl:text>NULL, // No Preinstallation function&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
				<xsl:with-param name="str" select="Preinstall"/>
			</xsl:call-template>
			<xsl:text>, // Preinstallation command&#13;</xsl:text>
		</xsl:if>
		<xsl:if test="not(Preinstall/@Type='External')">
			<xsl:call-template name="StrOrNullIfEmpty">
				<xsl:with-param name="str" select="Preinstall"/>
			</xsl:call-template>
			<xsl:text>, // Preinstallation function&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No Preinstallation command&#13;</xsl:text>
		</xsl:if>
		<!-- Flag to say if we may ingore returned error code from PreInstall function -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="Preinstall/@IgnoreError='true'"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if we may ignore returned error code from PreInstall function&#13;</xsl:text>

		<!-- Installer command line text, with optional condition flag -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="Install/@Type='Internal'">
			<xsl:text>NULL, // No Install Cmd condition&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No Install Cmd for true condition&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No Install Cmd for false condition&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No MsiFlags&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No MsiVersion&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No MsiUpgrade&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="StrOrNullIfEmpty">
				<xsl:with-param name="str" select="Install"/>
			</xsl:call-template>
			<xsl:text>, // Internal install function&#13;</xsl:text>
		</xsl:if>
		<xsl:if test="not(Install/@Type='Internal')">
			<xsl:if test="not(string-length(Install/@Flag)=0)">
				<xsl:text>&amp;</xsl:text>
				<xsl:value-of select="Install/@Flag"/>
				<xsl:text>, // Install Cmd condition flag&#13;</xsl:text>
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
					<xsl:with-param name="str" select="Install/True"/>
				</xsl:call-template>
				<xsl:text>, // Install Cmd condition true&#13;</xsl:text>
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
					<xsl:with-param name="str" select="Install/False"/>
				</xsl:call-template>
				<xsl:text>, // Install Cmd condition false&#13;</xsl:text>
			</xsl:if>
			<xsl:if test="string-length(Install/@Flag)=0">
				<xsl:text>NULL, // No Install Cmd condition&#13;</xsl:text>
				<xsl:text>&#09;&#09;</xsl:text>
				<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
					<xsl:with-param name="str" select="Install"/>
				</xsl:call-template>
				<xsl:text>, // External Install Cmd&#13;</xsl:text>
				<xsl:text>&#09;&#09;NULL, // No Install Cmd for false condition&#13;</xsl:text>
			</xsl:if>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="Install/@MsiFlags"/>
			</xsl:call-template>
			<xsl:text>, // MsiFlags&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="StrOrFalseIfEmpty">
				<xsl:with-param name="str" select="Install/@TestAnsiConversion"/>
			</xsl:call-template>
			<xsl:text>, // TestAnsiConversion&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="Install/@MsiVersion"/>
			</xsl:call-template>
			<xsl:text>, // MsiVersion&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="Install/@MsiUpgrade"/>
			</xsl:call-template>
			<xsl:text>, // MsiUpgrade&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No internal Install function&#13;</xsl:text>
		</xsl:if>

		<!-- dwSuccessCodeOverride -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrZeroIfEmpty">
			<xsl:with-param name="str" select="Install/@SuccessReturns"/>
		</xsl:call-template>
		<xsl:text>, // Success code override&#13;</xsl:text>
		
		<!-- Flag to say if pending reboot must be executed right after installation -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="MustNotDelayReboot"/>
		</xsl:call-template>
		<xsl:text>, // Flag to say if pending reboot must be executed right after installation&#13;</xsl:text>

		<!-- Download Web Site -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="DownloadURL"/>
		</xsl:call-template>
		<xsl:text>, // Download Web URL&#13;</xsl:text>

		<!-- Commentary during installation -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="Commentary"/>
		</xsl:call-template>
		<xsl:text>, // Commentary during installation&#13;</xsl:text>

		<!-- Status Window Control -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="StatusWindow"/>
		</xsl:call-template>
		<xsl:text>, // What to do with status window during installation &#13;</xsl:text>

		<!-- Flag indicating if status dialog is to pause till user presses Continue -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="StatusWindow/@StatusPauseWin98"/>
		</xsl:call-template>
		<xsl:text>, // StatusPauseWin98 flag&#13;</xsl:text>

		<!-- TestPresence function -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="TestPresence/@Type='External'">
			<xsl:text>NULL, // No TestPresence function&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
				<xsl:with-param name="str" select="TestPresence"/>
			</xsl:call-template>
		<xsl:text>, // TestPresence command&#13;</xsl:text>
		</xsl:if>
		<xsl:if test="not(TestPresence/@Type='External')">
			<xsl:call-template name="StrOrNullIfEmpty">
				<xsl:with-param name="str" select="TestPresence"/>
			</xsl:call-template>
			<xsl:text>, // TestPresence function&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No TestPresence command&#13;</xsl:text>
		</xsl:if>

		<!-- TestPresence version parameter -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
			<xsl:with-param name="str" select="TestPresence/@Version"/>
		</xsl:call-template>
		<xsl:text>, // TestPresence version parameter&#13;</xsl:text>

		<!-- Post-installation function -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:if test="PostInstall/@Type='External'">
			<xsl:text>NULL, // No Postinstallation function&#13;</xsl:text>
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
				<xsl:with-param name="str" select="PostInstall"/>
			</xsl:call-template>
		<xsl:text>, // Postinstallation command&#13;</xsl:text>
		</xsl:if>
		<xsl:if test="not(PostInstall/@Type='External')">
			<xsl:call-template name="StrOrNullIfEmpty">
				<xsl:with-param name="str" select="PostInstall"/>
			</xsl:call-template>
			<xsl:text>, // Postinstallation function&#13;</xsl:text>
			<xsl:text>&#09;&#09;NULL, // No Postinstallation command&#13;</xsl:text>
		</xsl:if>

		<!-- Help tag -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="QuotedCppPathStringOrNullIfEmpty">
			<xsl:with-param name="str" select="Help"/>
		</xsl:call-template>
		<xsl:text>, // Help tag&#13;</xsl:text>
		
		<!-- Flag which determines if Help Tag is an external file or not -->
		<xsl:text>&#09;&#09;</xsl:text>
		<xsl:call-template name="StrOrFalseIfEmpty">
			<xsl:with-param name="str" select="Help/@Internal"/>
		</xsl:call-template>
		<xsl:text>, // Flag which determines if Help Tag is an internal file or not&#13;</xsl:text>

		<!-- End of array -->
		<xsl:text>&#09;},&#13;</xsl:text>
		
		<!-- Add named features as "products" in their own right -->
		<xsl:for-each select="Feature">
			<xsl:text>&#09;{ // Product sub-feature, defined only for testing presence after installation&#13;</xsl:text>

			<!-- Product tag, used in dependency lists -->
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="Tag"/>
			</xsl:call-template>
			<xsl:text>, // Name used in dependency lists&#13;</xsl:text>

			<!-- Nice name of product, displayed to user -->
			<xsl:text>&#09;&#09;_T("</xsl:text>
			<xsl:value-of select="Title"/>
			<xsl:text>"), // Title&#13;</xsl:text>

			<!-- MSI Product Code -->
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="$ProductCode"/>
			</xsl:call-template>
			<xsl:text>, // MSI Product Code&#13;</xsl:text>

			<!-- Product tag, used in dependency lists -->
			<xsl:text>&#09;&#09;</xsl:text>
			<xsl:call-template name="QuotedStrOrNullIfEmpty">
				<xsl:with-param name="str" select="MsiFeature"/>
			</xsl:call-template>
			<xsl:text>, // MSI feature name&#13;</xsl:text>

			<!-- End of array -->
			<xsl:text>&#09;},&#13;</xsl:text>
		</xsl:for-each>
		
	</xsl:for-each>

	<xsl:text>};&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>


	<!-- Pre-requisite dependencies -->
	<xsl:choose>
		<xsl:when test="count(MasterInstaller/Products/Product/Prerequisite)=0">
			<xsl:text>// No pre-installation dependencies.&#13;</xsl:text>
			<xsl:text>static const Mapping * Prerequisites = NULL;&#13;</xsl:text>
			<xsl:text>static const int kctPrerequisites = 0;&#13;</xsl:text>			
		</xsl:when>
		<xsl:otherwise>
			<xsl:text>// List of pre-installation dependencies.&#13;</xsl:text>
			<xsl:text>static const Mapping Prerequisites[] =&#13;</xsl:text>
			<xsl:text>{&#13;</xsl:text>
			<xsl:for-each select="MasterInstaller/Products/Product">
				<xsl:variable name="Tag" select="Tag"/>
				<xsl:for-each select="Prerequisite">
					<xsl:text>&#09;{ _T("</xsl:text>
					<xsl:value-of select="$Tag"/>
					<xsl:text>"), _T("</xsl:text>
					<xsl:value-of select="@Tag"/>
					<xsl:text>"), </xsl:text>
					
					<!-- See if a Version is given -->
					<xsl:if test="not(string-length(@Version)=0)">
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@Version"/>
						</xsl:call-template>
						<xsl:text>, </xsl:text>
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@Version"/>
						</xsl:call-template>
					</xsl:if>
					<!-- else assume min and max versions -->
					<xsl:if test="string-length(@Version)=0">
						<!-- Min version -->
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@MinVersion"/>
						</xsl:call-template>
						<xsl:text>, </xsl:text>
						<!-- Max version -->
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@MaxVersion"/>
						</xsl:call-template>
					</xsl:if>

					<!-- There is no FailMsg in prerequisites -->
					<xsl:text>, NULL</xsl:text>
					<xsl:text> },&#13;</xsl:text>

				</xsl:for-each>
			</xsl:for-each>
			<xsl:text>};&#13;</xsl:text>
			<xsl:text>static const int kctPrerequisites = </xsl:text><xsl:number value="count(MasterInstaller/Products/Product/Prerequisite)"/><xsl:text>;&#13;</xsl:text>
		</xsl:otherwise>
	</xsl:choose>
	<xsl:text>&#13;</xsl:text>

	<!-- Post-installation dependencies -->
	<xsl:choose>
		<xsl:when test="count(MasterInstaller/Products/Product/Requires)=0 and count(MasterInstaller/Products/Product/Feature/Requires)=0">
			<xsl:text>// No post-installation dependencies.&#13;</xsl:text>
			<xsl:text>static const Mapping * RunDependencies = NULL;&#13;</xsl:text>
			<xsl:text>static const int kctRunDependencies = 0;&#13;</xsl:text>			
		</xsl:when>
		<xsl:otherwise>
			<!-- Note that there is a for loop for products, and a nested for loop for features -->
			<xsl:text>// List of post-installation dependencies.&#13;</xsl:text>
			<xsl:text>static const Mapping RunDependencies[] =&#13;</xsl:text>
			<xsl:text>{&#13;</xsl:text>
			<xsl:for-each select="MasterInstaller/Products/Product">
				<xsl:variable name="Tag" select="Tag"/>
				<xsl:for-each select="Requires">
					<xsl:text>&#09;{ _T("</xsl:text>
					<xsl:value-of select="$Tag"/>
					<xsl:text>"), _T("</xsl:text>
					<xsl:value-of select="@Tag"/>
					<xsl:text>"), </xsl:text>
					
					<!-- See if a Version is given -->
					<xsl:if test="not(string-length(@Version)=0)">
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@Version"/>
						</xsl:call-template>
						<xsl:text>, </xsl:text>
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@Version"/>
						</xsl:call-template>
					</xsl:if>
					<!-- else assume min and max versions -->
					<xsl:if test="string-length(@Version)=0">
						<!-- Min version -->
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@MinVersion"/>
						</xsl:call-template>
						<xsl:text>, </xsl:text>
						<!-- Max version -->
						<xsl:call-template name="QuotedStrOrNullIfEmpty">
							<xsl:with-param name="str" select="@MaxVersion"/>
						</xsl:call-template>
					</xsl:if>
					<xsl:text>, _T("</xsl:text>
					<xsl:value-of select="@FailMsg"/>
					<xsl:text>")</xsl:text>

					<xsl:text> },&#13;</xsl:text>
				</xsl:for-each>
				
				<xsl:for-each select="Feature">
					<xsl:variable name="FTag" select="Tag"/>
					<xsl:for-each select="Requires">
						<xsl:text>&#09;{ _T("</xsl:text>
						<xsl:value-of select="$FTag"/>
						<xsl:text>"), _T("</xsl:text>
						<xsl:value-of select="@Tag"/>
						<xsl:text>"), </xsl:text>

						<!-- See if a Version is given -->
						<xsl:if test="not(string-length(@Version)=0)">
							<xsl:call-template name="QuotedStrOrNullIfEmpty">
								<xsl:with-param name="str" select="@Version"/>
							</xsl:call-template>
							<xsl:text>, </xsl:text>
							<xsl:call-template name="QuotedStrOrNullIfEmpty">
								<xsl:with-param name="str" select="@Version"/>
							</xsl:call-template>
						</xsl:if>
						<!-- else assume min and max versions -->
						<xsl:if test="string-length(@Version)=0">
							<!-- Min version -->
							<xsl:call-template name="QuotedStrOrNullIfEmpty">
								<xsl:with-param name="str" select="@MinVersion"/>
							</xsl:call-template>
							<xsl:text>, </xsl:text>
							<!-- Max version -->
							<xsl:call-template name="QuotedStrOrNullIfEmpty">
								<xsl:with-param name="str" select="@MaxVersion"/>
							</xsl:call-template>
						</xsl:if>
						<xsl:text>, _T("</xsl:text>
						<xsl:value-of select="@FailMsg"/>
						<xsl:text>")</xsl:text>

						<xsl:text> },&#13;</xsl:text>
					</xsl:for-each>	
				</xsl:for-each>
			</xsl:for-each>
			<xsl:text>};&#13;</xsl:text>
			<xsl:text>static const int kctRunDependencies = </xsl:text><xsl:number value="count(MasterInstaller/Products/Product/Requires) + count(MasterInstaller/Products/Product/Feature/Requires)"/><xsl:text>;&#13;</xsl:text>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<!--
This template is used as a function to double up all '\' characters in a file path.
-->
<xsl:template name="BackslashDoubler">
	<xsl:param name="str"/>
	<xsl:call-template name="SearchReplace">
		<xsl:with-param name="str" select="$str"/>
		<xsl:with-param name="find" select="'\'"/>
		<xsl:with-param name="replace" select="'\\'"/>
	</xsl:call-template>
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
		</xsl:call-template><xsl:value-of select="$replace"/>
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

<!--
If str is empty, writes NULL, else doubles-up '\' and puts '\' before '"'
-->
<xsl:template name="QuotedCppPathStringOrNullIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:call-template name="CppPathString">
			<xsl:with-param name="str" select="$str"/>
		</xsl:call-template>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
</xsl:template>

<!--
This template is used as a recursive function to double-up '\' and to put '\' before '"'
-->
<xsl:template name="CppPathString">
	<xsl:param name="str"/>
	<xsl:if test="contains($str, '\')">
		<xsl:call-template name="CppPathString">
			<xsl:with-param name="str" select="substring-before($str, '\')"/>
		</xsl:call-template><xsl:value-of select="'\\'"/>
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
This template is used as a function to replace one string, if it matches a given value, with
another.
-->
<xsl:template name="ReplaceText">
	<xsl:param name="str"/>
	<xsl:param name="match"/>
	<xsl:param name="replace"/>
	<xsl:if test="not($str=$match)">
		<xsl:value-of select="$str"/>
	</xsl:if>
	<xsl:if test="$str=$match">
		<xsl:value-of select="$replace"/>
	</xsl:if>
</xsl:template>

<!--
This template is used as a function to replace an empty string with a given value.
-->
<xsl:template name="ReplaceEmpty">
	<xsl:param name="str"/>
	<xsl:param name="replace"/>
	<xsl:if test="not(string-length($str)=0)">
		<xsl:value-of select="$str"/>
	</xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:value-of select="$replace"/>
	</xsl:if>
</xsl:template>

<!--
This template is used as a function to test a string and output "false" if empty.
-->
<xsl:template name="StrOrFalseIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)"><xsl:value-of select="$str"/></xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>false</xsl:text>
	</xsl:if>
</xsl:template>

<!--
This template is used as a function to test a string and output "NULL" if empty, else output
the string.
-->
<xsl:template name="StrOrNullIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)">
		<xsl:value-of select="$str"/>
	</xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
</xsl:template>

<!--
This template is used as a function to test a string and output "NULL" if empty, else put 
quotes around it.
-->
<xsl:template name="QuotedStrOrNullIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:value-of select="$str"/>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
</xsl:template>

<!--
This template is used as a function to test a string and output "0" if empty, else output
the string.
-->
<xsl:template name="StrOrZeroIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)">
		<xsl:value-of select="$str"/>
	</xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>0</xsl:text>
	</xsl:if>
</xsl:template>

</xsl:stylesheet>
