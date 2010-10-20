// This script takes a full path to a file, and a text description of the file,
// and signs the file with the digital certificate found on the SIL certificate CD.

var fso = new ActiveXObject("Scripting.FileSystemObject");
var shellObj = new ActiveXObject("WScript.Shell");

if (WScript.Arguments.Length < 2)
{
	WScript.Echo("ERROR: must specify a file to be signed and a description of the file.");
	WScript.Quit();
}

var FileToBeSigned = WScript.Arguments.Item(0);
var TextDescription = WScript.Arguments.Item(1);

var MasterInstallerPath = shellObj.ExpandEnvironmentStrings("%MASTER_INSTALLER%");
if (MasterInstallerPath == "%MASTER_INSTALLER%")
{
	WScript.Echo("ERROR: the MASTER_INSTALLER environment variable has not been defined. This probably means you have not run the InitUtils.exe application in the Master Installer's Utils folder.");
	WScript.Quit();
}
var UtilsPath = fso.BuildPath(MasterInstallerPath, "Utils");

var CertificatePath = shellObj.ExpandEnvironmentStrings("%DIGITAL_CERT_DRIVE%");
if (CertificatePath == "%DIGITAL_CERT_DRIVE%")
	WScript.Echo("Cannot sign '" + FileToBeSigned + "', as the DIGITAL_CERT_DRIVE environment variable has not been set. This probably means you need to re-run the InitUtils.exe application in the Master Installer's Utils folder.");
else
{
	var CertFilePath = fso.BuildPath(CertificatePath, "comodo.spc");
	var CertKeyPath = fso.BuildPath(CertificatePath, "comodo.pvk");
	var fFoundCert = false;
	var drive = fso.GetDrive(CertificatePath);
	if (drive.IsReady)
		if (fso.FileExists(CertFilePath) && fso.FileExists(CertKeyPath))
		fFoundCert = true;
	if (!fFoundCert)
		WScript.Echo("About to sign '" + FileToBeSigned + "' - insert digital certificate CD into " + CertificatePath + " drive, then click OK, or just click OK anyway to proceed without signing.");
	if (drive.IsReady)
	{
		if (fso.FileExists(CertFilePath) && fso.FileExists(CertKeyPath))
		{
			var SignCodePath = fso.BuildPath(UtilsPath, "Sign\\signcode.exe");
			var SignCmd = '"' + SignCodePath + '" -spc "' + CertFilePath + '" -v "' + CertKeyPath + '" -n "' + TextDescription + '" -t http://timestamp.comodoca.com/authenticode -a sha1 "' + FileToBeSigned + '"';
			shellObj.Run(SignCmd, 0, true);
		}
	}
}
