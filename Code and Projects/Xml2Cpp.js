// This script takes an XML file, and parses it according to 3 XSL style sheets, to yield 3 C++
// files.
if (WScript.Arguments.Length < 1)
{
	WScript.Echo("ERROR - needs 1 argument: Input XML file.");
	WScript.Quit();
}
var strInputXml = WScript.Arguments.Item(0);
//WScript.Echo("Input File = " + strInputXml);
var xmlDoc = new ActiveXObject("Msxml2.DOMDocument.3.0");
xmlDoc.async = false;
xmlDoc.load(strInputXml);
if (xmlDoc.parseError.errorCode != 0)
{
	var myErr = xmlDoc.parseError;
	WScript.Echo("You have an XML error " + myErr.reason + " on line " + myErr.line + " at position " + myErr.linepos);
}
else
{
	ProcessFile(xmlDoc, "ConfigGeneral.xsl", "ConfigGeneral.cpp")
	ProcessFile(xmlDoc, "ConfigDisks.xsl", "ConfigDisks.cpp")
	ProcessFile(xmlDoc, "ConfigProducts.xsl", "ConfigProducts.cpp")
	ProcessFile(xmlDoc, "ConfigFunctions.xsl", "ConfigFunctions.cpp")
	ProcessFile(xmlDoc, "ConfigGlobals.xsl", "AutoGlobals.h")
	ProcessFile(xmlDoc, "ConfigResources.xsl", "AutoResources.rc")
}

function ProcessFile(xmlDoc, strInputXsl, strOutputFile)
{
	var xslt = new ActiveXObject("Msxml2.XSLTemplate.3.0");
	var xslDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
	var xslProc;
	xslDoc.async = false;
	xslDoc.load(strInputXsl);
	if (xslDoc.parseError.errorCode != 0)
	{
		var myErr = xslDoc.parseError;
		WScript.Echo("XSL error in " + strInputXsl + ": " + myErr.reason + " on line " + myErr.line + " at position " + myErr.linepos);
	}
	else
	{
		xslt.stylesheet = xslDoc;
		xslProc = xslt.createProcessor();
		xslProc.input = xmlDoc;
		xslProc.transform();
		var fso = new ActiveXObject("Scripting.FileSystemObject");
		var tso = fso.OpenTextFile(strOutputFile, 2, true);
		tso.Write(xslProc.output);
	}
}
