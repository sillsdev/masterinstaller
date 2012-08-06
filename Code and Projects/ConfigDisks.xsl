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

	<!-- CD configuration -->
	<xsl:text>// List of CDs used with this package.&#13;</xsl:text>
	<xsl:text>static const DiskDetail DiskDetails[] =&#13;</xsl:text>
	<xsl:text>{&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/CDs/CD">
		<xsl:text>&#09;{&#13;</xsl:text>
		<xsl:text>&#09;&#09;_T("</xsl:text>
		<xsl:value-of select="Title"/>
		<xsl:text>"),&#13;</xsl:text>
		<xsl:text>&#09;&#09;_T("</xsl:text>
		<xsl:value-of select="VolumeLabel"/>
		<xsl:text>"),&#13;</xsl:text>
		<xsl:text>&#09;},&#13;</xsl:text>
	</xsl:for-each>
	<xsl:text>};&#13;</xsl:text>
	<xsl:text>&#13;</xsl:text>
</xsl:template>

</xsl:stylesheet>
