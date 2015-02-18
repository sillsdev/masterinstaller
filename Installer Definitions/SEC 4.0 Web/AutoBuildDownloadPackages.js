// Fills in details for download packages automatically.
// This instance created AUTOMATICALLY during a previous run.
function AutomatePackages()
{
	SetElement("FlavorName1", "SEC_FullInstall");
	SetElement("FlavorUrl1", "http://downloads.sil.org/SILConverters/4.0/FullInstall.exe");
	AddFlavor();
	SetElement("FlavorName2", "SEC_PackageOnly");
	SetElement("FlavorUrl2", "http://downloads.sil.org/SILConverters/4.0/PackageOnly.exe");

	NextStage();

	SelectElement("IncludedF1P1", true);
	SelectElement("IncludedF1P2", false);
	SelectElement("IncludedF1P3", false);
	SelectElement("IncludedF1P4", false);
	SelectElement("IncludedF1P5", true);
	SelectElement("IncludedF1P6", false);
	SelectElement("IncludedF1P7", true);

	SelectElement("IncludedF2P1", true);
	SelectElement("IncludedF2P2", false);
	SelectElement("IncludedF2P3", false);
	SelectElement("IncludedF2P4", false);
	SelectElement("IncludedF2P5", false);
	SelectElement("IncludedF2P6", false);
	SelectElement("IncludedF2P7", false);

	NextStage();

	SetElement("OutputPath", "G:\\Software Package Builder\\Web Downloads\\SEC 4.0 Web");
	SelectElement("WriteXml", true);
	SelectElement("WriteDownloadsXml", false);
	SelectElement("Compile", true);
	SelectElement("GatherFiles", true);
	SelectElement("BuildSfx", true);
	SetElement("SfxStyle", "Standard");
	SetElement("SaveSettings", false);
}
