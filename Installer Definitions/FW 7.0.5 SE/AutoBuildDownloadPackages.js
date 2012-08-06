// Fills in details for download packages automatically.
// This instance created AUTOMATICALLY during a previous run.
function AutomatePackages()
{
	SetElement("FlavorName1", "FW705_SE_A");
	SetElement("FlavorUrl1", "http://downloads.sil.org/FieldWorks/7.0.5/SE/FW705_SE_A.exe");
	AddFlavor();
	SetElement("FlavorName2", "FW705_SE_B");
	SetElement("FlavorUrl2", "http://downloads.sil.org/FieldWorks/7.0.5/SE/FW705_SE_B.exe");

	NextStage();

	SelectElement("IncludedF1P1", true);
	SelectElement("IncludedF1P2", false);
	SelectElement("IncludedF1P3", true);
	SelectElement("IncludedF1P4", false);
	SelectElement("IncludedF1P5", false);
	SelectElement("IncludedF1P6", true);

	SelectElement("IncludedF2P1", true);
	SelectElement("IncludedF2P2", false);
	SelectElement("IncludedF2P3", false);
	SelectElement("IncludedF2P4", false);
	SelectElement("IncludedF2P5", false);
	SelectElement("IncludedF2P6", false);

	NextStage();

	SetElement("OutputPath", "G:\\Software Package Builder\\Web Downloads\\FW 7.0.5 SE");
	SelectElement("WriteXml", true);
	SelectElement("WriteDownloadsXml", true);
	SelectElement("Compile", true);
	SelectElement("GatherFiles", true);
	SelectElement("BuildSfx", true);
	SetElement("SfxStyle", "UseFwSfx");
	SetElement("SaveSettings", false);
}
