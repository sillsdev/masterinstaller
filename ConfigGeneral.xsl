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

	<!-- General configuration -->
	<xsl:text>// Main title for master installer:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszTitle = _T("</xsl:text>
	<xsl:value-of select="MasterInstaller/General/Title"/>
	<xsl:text>");&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Subtitle for product list:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszListSubtitle = _T("</xsl:text>
	<xsl:value-of select="MasterInstaller/General/ListSubtitle"/>
	<xsl:text>");&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Background color for product list:&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundR = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@Red"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundG = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@Green"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundB = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@Blue"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Background bitmap offsets for product list:&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpOffsetX = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@OffsetX"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpOffsetY = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@OffsetY"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Background bitmap blending weights for product list:&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpWeightTop = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@BlendTop"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpWeightBottom = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@BlendBottom"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpWeightLeft = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@BlendLeft"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>const int g_nListBackgroundBmpWeightRight = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListBackground/@BlendRight"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you want to bother to list a single product, when only one is available, set this to true:&#13;</xsl:text>
	<xsl:text>const bool g_fListEvenOneProduct = </xsl:text>
	<xsl:value-of select="MasterInstaller/General/ListEvenOneProduct"/>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you don't like the spacing of products in the selection menu, adjust it here:&#13;</xsl:text>
	<xsl:text>const int g_nListSpacingAdjust = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/ListSpacingAdjust"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you don't like the size of the "?" buttons in the selection menu, adjust it here:&#13;</xsl:text>
	<xsl:text>const int g_nInfoButtonAdjust = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/InfoButtonAdjust"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// This set to true when user is allowed to start from any CD in the set (otherwise they must start from CD 1):&#13;</xsl:text>
	<xsl:text>const bool g_fStartFromAnyCd = </xsl:text>
	<xsl:call-template name="StrOrFalseIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/StartFromAnyCD"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you need to hide the Product Key prompt unless the user is pressing Shift and Control, set this to true:&#13;</xsl:text>
	<xsl:text>const bool g_fKeyPromptNeedsShiftCtrl = </xsl:text>
	<xsl:call-template name="StrOrFalseIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/KeyPromptNeedsShiftCtrl"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Title for Product Key request dialog:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszGetKeyTitle = _T("</xsl:text>
	<xsl:value-of select="MasterInstaller/General/GetKeyTitle"/>
	<xsl:text>");&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// Initial text message:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszInitialText = </xsl:text>
	<xsl:call-template name="QuotedStrOrNullIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/InitialText"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>

	<xsl:text>// Alignment of initial text:&#13;</xsl:text>
	<xsl:text>const int g_nInitialTextAlign = </xsl:text>
	<xsl:if test="MasterInstaller/General/InitialText/@Align='left'">
		<xsl:text>eTextAlignLeftEdge</xsl:text>
	</xsl:if>
	<xsl:if test="MasterInstaller/General/InitialText/@Align='buttons'">
		<xsl:text>eTextAlignButtons</xsl:text>
	</xsl:if>
	<xsl:if test="MasterInstaller/General/InitialText/@Align='checkboxes'">
		<xsl:text>eTextAlignCheckBoxes</xsl:text>
	</xsl:if>
	<xsl:if test="string-length(MasterInstaller/General/InitialText/@Align)=0">
		<xsl:text>0</xsl:text>
	</xsl:if>
	<xsl:text>;&#13;</xsl:text>

	<xsl:text>// Left edge offset of initial text:&#13;</xsl:text>
	<xsl:text>const int g_nInitialTextLeftEdge = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/InitialText/@LeftEdge"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>

	<xsl:text>// Right edge offset of initial text:&#13;</xsl:text>
	<xsl:text>const int g_nInitialTextRightEdge = </xsl:text>
	<xsl:call-template name="StrOrZeroIfEmpty">
		<xsl:with-param name="str" select="MasterInstaller/General/InitialText/@RightEdge"/>
	</xsl:call-template>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you want external help to be available, set values for the file path and button text.&#13;</xsl:text>
	<xsl:text>// If not, set these both to NULL:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszExternalHelpFile = </xsl:text>
	<xsl:if test="string-length(MasterInstaller/General/ExternalHelpFile)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
	<xsl:if test="not(string-length(MasterInstaller/General/ExternalHelpFile)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:call-template name="BackslashDoubler">
			<xsl:with-param name="str" select="MasterInstaller/General/ExternalHelpFile"/>
		</xsl:call-template>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:text>;&#13;</xsl:text>
	
	<xsl:text>const TCHAR * g_pszHelpButtonText = </xsl:text>
	<xsl:if test="string-length(MasterInstaller/General/HelpButtonText)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
	<xsl:if test="not(string-length(MasterInstaller/General/HelpButtonText)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:call-template name="BackslashDoubler">
			<xsl:with-param name="str" select="MasterInstaller/General/HelpButtonText"/>
		</xsl:call-template>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>

	<xsl:text>// If you want terms of use to be available, set values for the file path and button text.&#13;</xsl:text>
	<xsl:text>// If not, set these both to NULL:&#13;</xsl:text>
	<xsl:text>const TCHAR * g_pszTermsOfUseFile = </xsl:text>
	<xsl:if test="string-length(MasterInstaller/General/TermsOfUseFile)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
	<xsl:if test="not(string-length(MasterInstaller/General/TermsOfUseFile)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:call-template name="BackslashDoubler">
			<xsl:with-param name="str" select="MasterInstaller/General/TermsOfUseFile"/>
		</xsl:call-template>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:text>;&#13;</xsl:text>
	
	<xsl:text>const TCHAR * g_pszTermsButtonText = </xsl:text>
	<xsl:if test="string-length(MasterInstaller/General/TermsButtonText)=0">
		<xsl:text>NULL</xsl:text>
	</xsl:if>
	<xsl:if test="not(string-length(MasterInstaller/General/TermsButtonText)=0)">
		<xsl:text>_T("</xsl:text>
		<xsl:call-template name="BackslashDoubler">
			<xsl:with-param name="str" select="MasterInstaller/General/TermsButtonText"/>
		</xsl:call-template>
		<xsl:text>")</xsl:text>
	</xsl:if>
	<xsl:text>;&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>
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
This template is used as a function to test a string and output 0 if empty.
-->
<xsl:template name="StrOrZeroIfEmpty">
	<xsl:param name="str"/>
	<xsl:if test="not(string-length($str)=0)"><xsl:value-of select="$str"/></xsl:if>
	<xsl:if test="string-length($str)=0">
		<xsl:text>0</xsl:text>
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

</xsl:stylesheet>
