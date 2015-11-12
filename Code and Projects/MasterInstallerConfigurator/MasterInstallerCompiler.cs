// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.IO;

namespace MasterInstallerConfigurator
{
	class MasterInstallerCompiler
	{
		public static void Compile(ConfigurationModel model, string certificatePassword, string masterInstallerPath, IConfigurationView view)
		{
			var _utilsPath = Path.Combine(masterInstallerPath, "Utils");
			var cppFilePath = Path.Combine(masterInstallerPath, "Code and Projects");
			var bitmapFilePath = Path.Combine(cppFilePath, "Bitmaps");
			if (CheckLegacyBitmapSetting(model, bitmapFilePath, view))
			{
				var tempFolder = CreateTempCompilationFolder(model);
				GenerateConfigGeneralCpp(cppFilePath, model);
				GenerateConfigDisksCpp(cppFilePath, model);
				/*
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigProducts.xsl", CppFilePath + "\\ConfigProducts.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigFunctions.xsl", CppFilePath + "\\ConfigFunctions.cpp");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGlobals.xsl", CppFilePath + "\\AutoGlobals.h");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigResources.xsl", CppFilePath + "\\AutoResources.rc");
				 */
			}
		}

		private static void GenerateConfigGeneralCpp(string cppFilePath, ConfigurationModel model)
		{
			var fileContents = File.ReadAllText(Path.Combine(cppFilePath, "ConfigGeneral.cpp.template"));
			StampCppString("MAINTITLE", model.General.Title, ref fileContents);
			StampCppString("SUBTITLE", model.General.ListSubtitle, ref fileContents);
			StampCppInt("BGCOLOR_R", model.General.ListBackground.Red, ref fileContents);
			StampCppInt("BGCOLOR_G", model.General.ListBackground.Green, ref fileContents);
			StampCppInt("BGCOLOR_B", model.General.ListBackground.Blue, ref fileContents);
			StampCppInt("BITMAPOFFSET_X", model.General.ListBackground.OffsetX, ref fileContents);
			StampCppInt("BITMAPOFFSET_Y", model.General.ListBackground.OffsetY, ref fileContents);
			StampCppInt("BLENDBOTTOM", model.General.ListBackground.BlendBottom, ref fileContents);
			StampCppInt("BLENDTOP", model.General.ListBackground.BlendTop, ref fileContents);
			StampCppInt("BLENDRIGHT", model.General.ListBackground.BlendRight, ref fileContents);
			StampCppInt("BLENDLEFT", model.General.ListBackground.BlendLeft, ref fileContents);
			StampCppBool("LISTONEPRODUCT", model.General.ListEvenOne, ref fileContents);
			StampCppInt("LISTADJUST", model.General.ListSpacingAdjust, ref fileContents);
			StampCppInt("BUTTONADJUST", model.General.InfoButtonAdjust, ref fileContents);
			StampCppBool("HIDEKEYPROMPT", model.General.KeyPromptNeedsKeys, ref fileContents);
			StampCppString("KEYPROMPTTITLE", model.General.KeyTitle, ref fileContents);
			StampCppBool("STARTFROMANY", model.General.StartFromAnyCD, ref fileContents);
			StampCppString("HELPBUTTONTEXT", model.General.HelpButtonText, ref fileContents);
			StampCppString("HELPBUTTONFILE", model.General.HelpFile, ref fileContents);
			StampCppString("TOUFILE", model.General.TermsOfUseFile, ref fileContents);
			StampCppString("TOUBUTTONTEXT", model.General.TermsButtonText, ref fileContents);
			StampCppBool("SHOWINSTALLCOMPLETE", model.General.ShowInstallCompleteMessage, ref fileContents);
			File.WriteAllText(Path.Combine(cppFilePath, "ConfigGeneral.cpp"), fileContents);
		}

		private static void GenerateConfigDisksCpp(string cppFilePath, ConfigurationModel model)
		{
			var fileContents = File.ReadAllText(Path.Combine(cppFilePath, "ConfigDisks.cpp.template"));
			StampCppString("PACKAGETITLE", model.CDs.CD[0].Title, ref fileContents);
			StampCppString("VOLUMELABEL", model.CDs.CD[0].VolumeLabel, ref fileContents);
			File.WriteAllText(Path.Combine(cppFilePath, "ConfigDisks.cpp"), fileContents);
		}

		private static void StampCppString(string replaceString, string modelString, ref string fileContents)
		{
			var replacementText = "NULL";
			if (!string.IsNullOrEmpty(modelString))
			{
				replacementText = "_T(\"" + modelString + "\")";
			}
			fileContents = fileContents.Replace(replaceString, replacementText);
		}

		private static void StampCppInt(string replaceString, int modelInt, ref string fileContents)
		{
			fileContents = fileContents.Replace(replaceString, modelInt.ToString());
		}

		private static void StampCppBool(string replaceString, bool modelBool, ref string fileContents)
		{
			fileContents = fileContents.Replace(replaceString, modelBool.ToString().ToLower());
		}

		private static string CreateTempCompilationFolder(ConfigurationModel model)
		{
			var folderRoot = Path.GetDirectoryName(model.FileLocation);
			var tempFolder = Path.Combine(folderRoot, "setup.exe coming soon");
			Directory.CreateDirectory(tempFolder);
			return tempFolder;
		}

		private static bool CheckLegacyBitmapSetting(ConfigurationModel model, string bitmapFilePath, IConfigurationView view)
		{
			if (model.General.ListBackground.ImagePath.Contains(Path.DirectorySeparatorChar.ToString()))
			{
				if (!File.Exists(model.General.ListBackground.ImagePath))
				{
					view.LogErrorLine(string.Format("Bitmap path '{0}' specified in XML node /MasterInstaller/General/ListBackground does not exist. This may be legacy data. Bitmaps should be stored in the '{1}' folder and referenced by file name only.",
						model.General.ListBackground.ImagePath, bitmapFilePath));
					return false;
				}
			}
			return true;
		}
	}
}
