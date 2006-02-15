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
	<title>Software depenendecies - full tree</title>
  </head>
  <body bgcolor="#F8F8C8">
	<h1 align="center">Software depenendecies (prerequisites and requirements)</h1>
	<table>
		<xsl:apply-templates select="/MasterInstaller/Products/Product"/>
	</table>
  </body>
</html>
</xsl:template>


<!-- ================================= -->
<!-- Product Template                  -->
<!-- ================================= -->
<xsl:template match="/MasterInstaller/Products/Product">
	<xsl:call-template name="RecurseTag">
		<xsl:with-param name="tag" select="Tag"/>
	</xsl:call-template>
	<xsl:for-each select="Feature">
		<tr>
			<td valign="top" align="left">
				<xsl:value-of select="Title"/>
			</td>
			<td valign="top" align="left">
				<xsl:if test="count(Requires)>0">
					<table>
						<tr>
							<th align="left">Requirements</th>
						</tr>
						<xsl:for-each select="Requires">
							<xsl:call-template name="RecurseTag">
								<xsl:with-param name="tag" select="@Tag"/>
							</xsl:call-template>
						</xsl:for-each>
					</table>
				</xsl:if>
			</td>
		</tr>
	</xsl:for-each>
</xsl:template>

<xsl:template name="RecurseTag">
	<xsl:param name="tag"/>
	<!-- Iterate over every product, except where a duplicate Tag is found: -->
	<xsl:for-each select="/MasterInstaller/Products/Product[Tag=$tag][1]">
		<tr>
			<td valign="top" align="left">
				<xsl:value-of select="Title"/>
			</td>
			<td valign="top" align="left">
				<xsl:if test="count(Prerequisite)>0 or count(Requires)>0">
					<table>
						<tr>
							<xsl:if test="count(Prerequisite)>0">
								<td valign="top">
								<table>
								<tr>
									<th align="left">Prerequisites</th>
								</tr>
								<xsl:for-each select="Prerequisite">
									<xsl:call-template name="RecurseTag">
										<xsl:with-param name="tag" select="@Tag"/>
									</xsl:call-template>
								</xsl:for-each>
								</table>
								</td>
							</xsl:if>
							<xsl:if test="count(Requires)>0">
								<td valign="top">
								<table>
								<tr>
									<th align="left">Requirements</th>
								</tr>
								<xsl:for-each select="Requires">
									<xsl:call-template name="RecurseTag">
										<xsl:with-param name="tag" select="@Tag"/>
									</xsl:call-template>
								</xsl:for-each>
								</table>
								</td>
							</xsl:if>
						</tr>
					</table>
				</xsl:if>
			</td>
		</tr>
	</xsl:for-each>
</xsl:template>


</xsl:stylesheet>
