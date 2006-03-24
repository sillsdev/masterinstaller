<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
  xmlns:html="http://www.w3.org/1999/xhtml"
  xmlns:ms="urn:schemas-microsoft-com:xslt"
  xmlns:dt="urn:schemas-microsoft-com:datatypes">
<xsl:output method="html" />

<!-- =============================================== -->
<!-- Root Template                                   -->
<!-- =============================================== -->
<xsl:template match="/">
<html>
  <head>
	<title>Software depenendecies - descriptive list</title>
  </head>
  <body bgcolor="#F8F8C8">
	<h1 align="center">Software dependencies (prerequisites and requirements)</h1>
	<br/>
	<xsl:apply-templates select="/MasterInstaller/Products/Product"/>
 </body>
</html>
</xsl:template>


<!-- ================================= -->
<!-- Product Template                  -->
<!-- ================================= -->
<xsl:template match="/MasterInstaller/Products/Product">
	<xsl:variable name="MainTitle" select="Title"/>
	<h2><xsl:value-of select="Title"/></h2>
	<h3><xsl:text>To install:</xsl:text></h3>
	<xsl:if test="count(Prerequisite)=0">
		<xsl:text>You need no extra software installed.</xsl:text>
		<br/>
	</xsl:if>
	<xsl:if test="count(Prerequisite)>0">
		<xsl:text>You can only install if the following are present:</xsl:text>
		<br/>
		<UL>
			<xsl:variable name="PrerequisiteList">
				<xsl:call-template name="GetPrerequisites">
					<xsl:with-param name="tag" select="Tag"/>
				</xsl:call-template>
			</xsl:variable>
			<xsl:call-template name="OutputFilteredList">
				<xsl:with-param name="List" select="$PrerequisiteList"/>
				<xsl:with-param name="MainTitle" select="$MainTitle"/>
			</xsl:call-template>
		</UL>
	</xsl:if>
	<h3><xsl:text>To run:</xsl:text></h3>
	<xsl:if test="count(Requires)=0 and count(Feature/Requires)=0">
		<xsl:text>You need no extra software installed.</xsl:text>
		<br/>
	</xsl:if>
	<xsl:if test="count(Requires)>0">
		<xsl:text>You need to have the following software:</xsl:text>
		<UL>
			<xsl:variable name="RequirementList">
				<xsl:call-template name="GetRequirements">
					<xsl:with-param name="tag" select="Tag"/>
				</xsl:call-template>
			</xsl:variable>
			<xsl:call-template name="OutputFilteredList">
				<xsl:with-param name="List" select="$RequirementList"/>
				<xsl:with-param name="MainTitle" select="$MainTitle"/>
			</xsl:call-template>
		</UL>
	</xsl:if>
	<xsl:for-each select="Feature">
		<xsl:if test="count(Requires)>0">
			<xsl:text>If you installed the </xsl:text>
			<b><xsl:value-of select="Title"/></b>
			<xsl:text> feature, you need:</xsl:text>
		<UL>
			<xsl:variable name="FeatureRequirementList">
				<xsl:call-template name="GetFeatureRequirements">
					<xsl:with-param name="tag" select="Tag"/>
				</xsl:call-template>
			</xsl:variable>
			<xsl:call-template name="OutputFilteredList">
				<xsl:with-param name="List" select="$FeatureRequirementList"/>
				<xsl:with-param name="MainTitle" select="$MainTitle"/>
			</xsl:call-template>
		</UL>
		</xsl:if>
	</xsl:for-each>
	<br/>
</xsl:template>

<xsl:template name="GetPrerequisites">
	<xsl:param name="tag"/>
	<xsl:param name="MinVersion"/>
	<xsl:param name="MaxVersion"/>
	<xsl:param name="Version"/>
	<xsl:param name="PrereqIndex" select="0"/>
	
	<!-- Iterate over every product, except where a duplicate Tag is found: -->
	<xsl:for-each select="/MasterInstaller/Products/Product[Tag=$tag][1]">
		<Product>
			<Title><xsl:value-of select="Title"/></Title>
			<Tag><xsl:value-of select="$tag"/></Tag>
			<MinVersion><xsl:value-of select="$MinVersion"/></MinVersion>
			<MaxVersion><xsl:value-of select="$MaxVersion"/></MaxVersion>
			<Version><xsl:value-of select="$Version"/></Version>
			<URL><xsl:value-of select="DownloadURL"/></URL>
		</Product>
		<xsl:for-each select="Prerequisite">
			<xsl:call-template name="GetPrerequisites">
				<xsl:with-param name="tag" select="@Tag"/>
				<xsl:with-param name="MinVersion" select="@MinVersion"/>
				<xsl:with-param name="MaxVersion" select="@MaxVersion"/>
				<xsl:with-param name="Version" select="@Version"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

<xsl:template name="GetRequirements">
	<xsl:param name="tag"/>
	<xsl:param name="MinVersion"/>
	<xsl:param name="MaxVersion"/>
	<xsl:param name="Version"/>
	<xsl:param name="FailMsg"/>
	<xsl:param name="ReqIndex" select="0"/>
	
	<!-- Iterate over every product, except where a duplicate Tag is found: -->
	<xsl:for-each select="/MasterInstaller/Products/Product[Tag=$tag][1]">
		<Product>
			<Title><xsl:value-of select="Title"/></Title>
			<Tag><xsl:value-of select="$tag"/></Tag>
			<MinVersion><xsl:value-of select="$MinVersion"/></MinVersion>
			<MaxVersion><xsl:value-of select="$MaxVersion"/></MaxVersion>
			<Version><xsl:value-of select="$Version"/></Version>
			<FailMsg><xsl:value-of select="$FailMsg"/></FailMsg>
			<URL><xsl:value-of select="DownloadURL"/></URL>
		</Product>
		<xsl:for-each select="Requires">
			<xsl:call-template name="GetRequirements">
				<xsl:with-param name="tag" select="@Tag"/>
				<xsl:with-param name="MinVersion" select="@MinVersion"/>
				<xsl:with-param name="MaxVersion" select="@MaxVersion"/>
				<xsl:with-param name="Version" select="@Version"/>
				<xsl:with-param name="FailMsg" select="@FailMsg"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

<xsl:template name="GetFeatureRequirements">
	<xsl:param name="tag"/>
	<!-- Iterate over every feature, except where a duplicate Tag is found: -->
	<xsl:for-each select="/MasterInstaller/Products/Product/Feature[Tag=$tag][1]">
		<xsl:for-each select="Requires">
			<xsl:call-template name="GetRequirements">
				<xsl:with-param name="tag" select="@Tag"/>
				<xsl:with-param name="MinVersion" select="@MinVersion"/>
				<xsl:with-param name="MaxVersion" select="@MaxVersion"/>
				<xsl:with-param name="Version" select="@Version"/>
				<xsl:with-param name="FailMsg" select="@FailMsg"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

<xsl:template name="OutputFilteredList">
	<xsl:param name="List"/>
	<xsl:param name="MainTitle"/>

	<xsl:for-each select="ms:node-set($List)/Product[not(Title=preceding::Product/Title)]">
		<xsl:if test="not($MainTitle=Title)">
			<LI>
				<xsl:value-of select="Title"/><br/>
				<small>
					<xsl:variable name="HighestMin">
						<xsl:call-template name="GetMaxValue">
							<xsl:with-param name="List" select="$List"/>
							<xsl:with-param name="Title" select="Title"/>
							<xsl:with-param name="Field" select="'MinVersion'"/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:if test="not(string-length($HighestMin)=0)">
						<xsl:text>Minimum version: </xsl:text>
						<xsl:value-of select="$HighestMin"/>
						<br/>
					</xsl:if>
					<xsl:variable name="LowestMax">
						<xsl:call-template name="GetMinValue">
							<xsl:with-param name="List" select="$List"/>
							<xsl:with-param name="Title" select="Title"/>
							<xsl:with-param name="Field" select="'MaxVersion'"/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:if test="not(string-length($LowestMax)=0)">
						<xsl:text>Maximum version: </xsl:text>
						<xsl:value-of select="$LowestMax"/>
						<br/>
					</xsl:if>
					<xsl:if test="not(string-length(Version)=0)">
						Required Version: <xsl:value-of select="Version"/><br/>
					</xsl:if>
					<xsl:if test="not(string-length(FailMsg)=0)">
						Consequence if missing: <xsl:value-of select="FailMsg"/><br/>
					</xsl:if>
					<xsl:if test="not(string-length(URL)=0)">
						Available from:
						<a>
							<xsl:attribute name="href"><xsl:value-of select="URL"/></xsl:attribute>
							<xsl:value-of select="URL"/>
						</a>
					</xsl:if>
				</small>
			</LI>
		</xsl:if>
	</xsl:for-each>
</xsl:template>

<xsl:template name="GetMaxValue">
	<xsl:param name="List"/>
	<xsl:param name="Title"/>
	<xsl:param name="Field"/>
	<xsl:value-of select="ms:node-set($List)/Product[Title=$Title]/*[name()=$Field and not(string-length(.)=0) and not(. &lt; following::Product[Title=$Title]/*[name()=$Field])]"/>
</xsl:template>

<xsl:template name="GetMinValue">
	<xsl:param name="List"/>
	<xsl:param name="Title"/>
	<xsl:param name="Field"/>
	<xsl:value-of select="ms:node-set($List)/Product[Title=$Title]/*[name()=$Field and not(string-length(.)=0) and not(. &gt; following::Product[Title=$Title]/*[name()=$Field])]"/>
</xsl:template>

</xsl:stylesheet>
