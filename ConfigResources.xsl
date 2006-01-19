<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<!--
Converts an XML definition of a master installer into Resources (.rc) that can be compiled
with the "setup" project to make an executable master installer.
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

	<!-- Define bitmap -->
	<xsl:if test="not(string-length(/MasterInstaller/General/ListBackground)=0)">
		<xsl:text>// Bitmap for product selection dialog background:&#13;</xsl:text>
		<xsl:text>BACKGROUND_BMP BITMAP "</xsl:text>
		<xsl:call-template name="CppPathString">
			<xsl:with-param name="str" select="/MasterInstaller/General/ListBackground"/>
		</xsl:call-template>
		<xsl:text>"&#13;</xsl:text>
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

</xsl:stylesheet>
