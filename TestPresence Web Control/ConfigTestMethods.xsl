<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<!--
Converts an XML definition of a master installer into C++ that can be compiled with the
Test Presence Web Control project.
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
	<xsl:text>&#13;&#13;</xsl:text>

	<xsl:text>// List of product test methods.&#13;</xsl:text>
	<xsl:text>static const Product_t Products[] =&#13;</xsl:text>
	<xsl:text>{&#13;</xsl:text>
	<xsl:for-each select="MasterInstaller/Products/Product">
		<xsl:text>&#09;{ _T("</xsl:text>
		<xsl:value-of select="Tag"/>
		<xsl:text>"), </xsl:text>
		<xsl:call-template name="QuotedStrOrNullIfEmpty">
		<xsl:with-param name="str" select="ProductCode"/>
		</xsl:call-template>
		<xsl:text>, </xsl:text>
		<xsl:if test="TestPresence/@Type='External'">
			<xsl:text>NULL</xsl:text>
		</xsl:if>
		<xsl:if test="not(TestPresence/@Type='External')">
			<xsl:call-template name="StrOrNullIfEmpty">
			<xsl:with-param name="str" select="TestPresence"/>
			</xsl:call-template>
		</xsl:if>
		<xsl:text>, </xsl:text>
		<xsl:text>},&#13;</xsl:text>
	</xsl:for-each>
	<xsl:text>};&#13;</xsl:text>
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

</xsl:stylesheet>

