// This script expects as an argument a downloads.xml file.
// It creates a text report file downloads.txt listing each download
// package, along with file size, MD5 hash (if located) and web URL.

var fso = new ActiveXObject("Scripting.FileSystemObject");
var shellObj = new ActiveXObject("WScript.Shell");
var RequiredFile = "downloads.xml";

if (WScript.Arguments.Length < 1)
{
	WScript.Echo("ERROR: must specify " + RequiredFile + " path.");
	WScript.Quit();
}

var XmlFileName = WScript.Arguments.Item(0);

// Check that the input file is an XML file:
if (XmlFileName.slice(-RequiredFile.length).toLowerCase() != RequiredFile.toLowerCase())
{
	WScript.Echo("ERROR: must specify " + RequiredFile + " path.");
	WScript.Quit();
}

var xmlDoc = new ActiveXObject("Msxml2.FreeThreadedDOMDocument.3.0");
xmlDoc.async = false;
xmlDoc.load(XmlFileName);
if (xmlDoc.parseError.errorCode != 0)
{
	var myErr = xmlDoc.parseError;
	WScript.Echo("You have an XML error " + myErr.reason + " on line " + myErr.line + " at position " + myErr.linepos);
	WScript.Quit();
}

// Get folder path of downloads.xml:
var iLastBackslash = XmlFileName.lastIndexOf("\\");
var RootFolderPath = XmlFileName.slice(0, iLastBackslash);

var DownloadNodes = xmlDoc.selectNodes("/Downloads/Download");

var tsoReport = fso.OpenTextFile(fso.BuildPath(RootFolderPath, "Downloads.txt"), 2, true);

for (i = 0; i < DownloadNodes.length; i++)
{
	var DownloadNode = DownloadNodes[i];
	var FlavorRoot = DownloadNode.getAttribute("Flavor");
	var URL = DownloadNode.selectSingleNode("DownloadURL").text;
	var ExeFileName = FlavorRoot + ".exe";
	var DownloadPackage = fso.GetFile(fso.BuildPath(RootFolderPath, ExeFileName));
	var Size = FormatSize(DownloadPackage.size);
	var MD5FilePath = fso.BuildPath(RootFolderPath, FlavorRoot + ".md5.txt");
	var tso = fso.OpenTextFile(MD5FilePath, 1);
	var MD5 = tso.ReadLine().toUpperCase();

	tsoReport.WriteLine(ExeFileName);
	tsoReport.WriteLine("Size = " + Size + ", MD5 = " + MD5)
	tsoReport.WriteLine(URL);
	tsoReport.WriteLine();
}

tsoReport.Close();

function FormatSize(filesize)
{
	Units = ["bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]

	for (order = 0; order < Units.length; order++)
	{
		var standard = Math.pow(1024, order);

		if (filesize < standard * 1024)
			return ThreeSigFigsSize(filesize, standard, Units[order])
	}
	return filesize + ' bytes';
}

function ThreeSigFigsSize(filesize, Order, Unit)
{
	if (filesize >= 100 * Order)
	{
		filesize = Math.floor(filesize / Order);
		return filesize + ' ' + Unit;
	}
	if (filesize >= 10 * Order)
	{
		filesize = Math.floor(filesize * 10 / Order) / 10;
		return filesize + ' ' + Unit;
	}
	if (filesize >= Order)
	{
		filesize = Math.floor(filesize * 100 / Order) / 100;
		return filesize + ' ' + Unit;
	}
	return "unknown";
}
