<?xml version="1.0" encoding="UTF-8"?>
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
	<title>Show Version Info</title>
  </head>
  <body bgcolor="#F8F8C8">
	<h1 align="center">Product Version Info</h1>
	<xsl:apply-templates select="/MasterInstaller/Products"/>
  </body>
</html>
</xsl:template>


<!-- =============================================== -->
<!-- Project Configuration Template                  -->
<!-- =============================================== -->
<xsl:template match="/MasterInstaller/Products">
	<table border="1">
	<xsl:for-each select="Product">
		<tr>
			<td align="right" valign="middle"><xsl:value-of select="AutoConfigure/Title"/></td>
			<td align="center">
				<xsl:if test="not(string-length(Contact/VersionInfo)=0)">
					<table>
					<tr><td align="center">
					<iframe width="600" height="300">
						<xsl:attribute name="src"><xsl:value-of select="Contact/VersionInfo"/></xsl:attribute>
					</iframe>
					</td></tr>
					<tr>
						<td align="center">
							<a>
								<xsl:attribute name="href"><xsl:value-of select="Contact/VersionInfo"/></xsl:attribute>
								<xsl:attribute name="target">_blank</xsl:attribute>
								<xsl:value-of select="Contact/VersionInfo"/>
							</a>
						</td>
					</tr>
					</table>
				</xsl:if>
				<xsl:if test="string-length(Contact/VersionInfo)=0">
					<xsl:text>[none]</xsl:text>
				</xsl:if>
			</td>
		</tr>
	</xsl:for-each>
	</table>
</xsl:template>

</xsl:stylesheet>
