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

	<!-- Include external functions -->
	<xsl:text>// TestPresence functions:&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product/TestPresence">
		<xsl:if test="not(@Type='External')">
			<xsl:text>#include "</xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>.cpp"&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:text>&#13;</xsl:text>
	<xsl:text>// Preinstallation functions:&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product/Preinstall">
		<xsl:if test="not(@Type='External') and not(@IgnoreError='true')">
			<xsl:text>#include "</xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>.cpp"&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:text>&#13;</xsl:text>
	<xsl:text>// Installation functions:&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product/Install">
		<xsl:if test="@Type='Internal'">
			<xsl:text>#include "</xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>.cpp"&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
	<xsl:text>&#13;</xsl:text>
	<xsl:text>// Postinstallation functions:&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product/PostInstall">
		<xsl:if test="not(@Type='External')">
			<xsl:text>#include "</xsl:text>
			<xsl:value-of select="."/>
			<xsl:text>.cpp"&#13;</xsl:text>
		</xsl:if>
	</xsl:for-each>
</xsl:template>

</xsl:stylesheet>
