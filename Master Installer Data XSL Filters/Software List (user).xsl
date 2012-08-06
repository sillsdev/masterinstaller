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
	<title>Software List (as the user sees it)</title>
  </head>
  <body bgcolor="#F8F8C8">
	<h1 align="center">Product List</h1>
	<xsl:apply-templates select="/MasterInstaller/Products/Product"/>
  </body>
</html>
</xsl:template>


<!-- ================================= -->
<!-- Product Template                  -->
<!-- ================================= -->
<xsl:template match="/MasterInstaller/Products/Product">
	<xsl:value-of select="Title"/>
	<br/>
</xsl:template>
</xsl:stylesheet>
