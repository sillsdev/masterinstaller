﻿// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.IO;
using System.Text;

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

		private static void GenerateProductsCpp(string cppFilePath, ConfigurationModel model)
		{
			var externalFunctions = new StringBuilder();
			externalFunctions.AppendLine("// External functions:");
			var productConfigurations = new StringBuilder();
			productConfigurations.AppendLine("// List of software products in this package.");
			productConfigurations.AppendLine("static SoftwareProduct Products[] =\n{");
			var preInstallationDependencies = new StringBuilder();
			preInstallationDependencies.AppendLine("// List of pre-installation dependencies.");
			preInstallationDependencies.AppendLine("static const Mapping Prerequisites[] =\n{");
			var postInstallationTests = new StringBuilder();
			postInstallationTests.AppendLine("// List of post-installation dependencies.");
			postInstallationTests.AppendLine("static const Mapping RunDependencies[] =\n{");
			foreach (var product in model.Products)
			{
				GenerateProductExternalFunctions(product, externalFunctions);
				productConfigurations.AppendLine("\t{");
				GenerateProductStringVariable(product.Tag, "Name used in dependency lists", productConfigurations);
				GenerateProductStringVariable(product.Title, "Title", productConfigurations);
				GenerateProductStringVariable(product.ProductCode, "MSI Product Code", productConfigurations);
				GenerateProductStringVariable(product.Version, "Version for Product Key test", productConfigurations);
				GenerateProductIntVariable(product.KeyId, "-1", "Product Key ID", productConfigurations);
				GenerateProductBoolVariable(product.List, "Flag saying if product should be offered to user (m_kOneOfOurs)",
					productConfigurations);
				// The IsContainer flag is obsolete and not present in any existing config files, but we will write it out as false for now
				GenerateProductBoolVariable(false, "IsContainer (Obsolete, always false)",
					productConfigurations);
				GenerateProductStringVariable(null, "Critical file condition flag (Obsolete)", productConfigurations);
				GenerateProductStringVariable(product.CriticalFile, "Critical file (was critical file condition true)", productConfigurations);
				GenerateProductStringVariable(null, "Critical file condition false (Obsolete)", productConfigurations);
				GenerateProductIntVariable(product.CDNumber, "CD index (zero-based)", productConfigurations);
				GenerateProductStringVariable(null, "MinOS (Obsolete)", productConfigurations);
				GenerateProductStringVariable(null, "MaxOS (Obsolete)", productConfigurations);
				GenerateProductBoolVariable(product.MustBeAdmin, "Must be admin to install", productConfigurations);
				GenerateProductBoolVariable(product.FlushReboot, "Flag to say if pending reboot must be executed first", productConfigurations);
				GenerateProductBoolVariable(false, "flag related to reboot and PendingFileRenameOperations reg setting (Obsolete)", productConfigurations);
				GenerateProductBoolVariable(false, "flag related to reboot and wininit.ini (Obsolete)", productConfigurations);
				GenerateProductBoolVariable(false, "flag related to killing hanging windows before installation (Obsolete)", productConfigurations);
				GenerateProductFunctionVariable(product.Preinstall, "Preinstallation function", productConfigurations);
				GenerateProductStringVariable(null, "No External Preinstallation function (Obsolete)", productConfigurations);
				GenerateProductBoolVariable(false, "Flag to say if we may ignore returned error code from Preinstall function (Obsolete)", productConfigurations);
				GenerateProductInstallVariables(product, productConfigurations);
				GenerateProductIntVariable(0, "Success code override (Obsolete)", productConfigurations);
				GenerateProductBoolVariable(product.MustNotDelayReboot, "Flag to say if pending reboot must be executed right after installation", productConfigurations);
				GenerateProductStringVariable(product.DownloadURL, "Download Web URL", productConfigurations);
				GenerateProductStringVariable(product.Commentary, "Commentary during installation", productConfigurations);
				GenerateProductStringVariable(product.StatusWindow, "What to do with status window during installation", productConfigurations);
				GenerateProductBoolVariable(false, "StatusPauseWin98 flag (Obsolete)", productConfigurations);
			}
		}

		private static void GenerateProductInstallVariables(ConfigurationModel.Product product, StringBuilder builder)
		{
			if (product.Install.Type == "Internal")
			{
				GenerateProductStringVariable(null, "No Install Cmd condition", builder);
				GenerateProductStringVariable(null, "No Install Cmd for true condition", builder);
				GenerateProductStringVariable(null, "No Install Cmd for false condition", builder);
				GenerateProductStringVariable(null, "No MsiFlags", builder);
				GenerateProductBoolVariable(false, "Don't test ANSI conversion", builder);
				GenerateProductStringVariable(null, "No MsiUpgradeFrom", builder);
				GenerateProductStringVariable(null, "No MsiUpgradeTo", builder);
				GenerateProductStringVariable(null, "No MsiUpgradeFlags", builder);
				GenerateProductFunctionVariable(product.Install.InstallFunction, "Internal install function", builder);
			}
			else
			{
				if (!string.IsNullOrEmpty(product.Install.Flag))
				{
					GenerateProductFunctionVariable(product.Install.Flag, "Install Cmd condition flag", builder);
					GenerateProductPathVariable(product.Install.True, "Install Cmd condition true", builder);
					GenerateProductPathVariable(product.Install.False, "Install Cmd condition true", builder);
				}
				else
				{
					GenerateProductStringVariable(null, "No Install Cmd condition", builder);
					GenerateProductStringVariable(product.Install.InstallFunction, "External install Cmd", builder);
					GenerateProductStringVariable(null, "No Install Cmd for false condition", builder);
				}
				GenerateProductStringVariable(product.Install.MsiFlags, "No MsiFlags", builder);
				GenerateProductBoolVariable(false, "Don't test ANSI conversion (Obsolete)", builder);
				GenerateProductStringVariable(product.Install.MsiUpgradeFrom, "MsiUpgradeFrom", builder);
				GenerateProductStringVariable(product.Install.MsiUpgradeTo, "No MsiUpgradeTo", builder);
				GenerateProductStringVariable(null, "No MsiUpgradeFlags (Obsolete)", builder);
				GenerateProductFunctionVariable(null, "No internal install function", builder);
			}
		}

		private static void GenerateProductPathVariable(string path, string comment, StringBuilder builder)
		{
			var pathValue = "NULL";
			if (!string.IsNullOrEmpty(path))
			{
				pathValue = string.Format("_T(\"{0}\")", path.Replace("\\", "\\\\"));
			}
			builder.AppendLine(string.Format("{0}, // {1}", pathValue, comment));
		}

		private static void GenerateProductFunctionVariable(string preInstall, string comment, StringBuilder builder)
		{
			builder.AppendLine(string.Format("\t\t{0}, // {1}", string.IsNullOrEmpty(preInstall) ? "NULL" : preInstall, comment));
		}

		private static void GenerateProductIntVariable(int integerValue, string comment, StringBuilder productConfigurations)
		{
			GenerateProductIntVariable(integerValue.ToString(), "", comment, productConfigurations);
		}

		private static void GenerateProductBoolVariable(bool boolValue, string comment, StringBuilder productConfigurations)
		{
			productConfigurations.AppendLine(string.Format("\t\t{0}, {1}", boolValue.ToString().ToLower(), comment));
		}

		private static void GenerateProductIntVariable(string integerString, string valueIfEmpty, string comment, StringBuilder stringBuilder)
		{
			var intValue = valueIfEmpty;
			if (!string.IsNullOrEmpty(integerString))
				intValue = integerString;
			stringBuilder.AppendLine(string.Format("\t\t{0}, {1}", intValue, comment));
		}

		private static void GenerateProductStringVariable(string variableValue, string comment, StringBuilder builder)
		{
			if (string.IsNullOrEmpty(variableValue))
				variableValue = "NULL";
			builder.AppendLine(string.Format("\t\t_T(\"{0}\"), // {1}", variableValue, comment));
		}

		private static void GenerateProductExternalFunctions(ConfigurationModel.Product product, StringBuilder externalFunctions)
		{
			if (product.TestPresence != null)
			{
				var productTestFunction =
					string.Format("extern bool {0}(const _TCHAR * pszMinVersion, const _TCHAR * pszMaxVersion, SoftwareProduct * Product);",
						product.TestPresence.TestFunction);
				externalFunctions.AppendLine(productTestFunction);
			}
			if (!string.IsNullOrEmpty(product.Preinstall))
			{
				externalFunctions.AppendLine(string.Format("extern int {0}(SoftwareProduct * Product);", product.Preinstall));
			}
			if (product.Install != null && product.Install.Type == "Internal")
			{
				externalFunctions.AppendLine(string.Format("extern DWORD {0}(bool fFlag, SoftwareProduct * Product);",
					product.Install.InstallFunction));
			}
			if (product.PostInstall != null)
			{
				externalFunctions.AppendLine(string.Format("extern int {0}(SoftwareProduct * Product);",
					product.PostInstall.PostInstallFunction));
			}
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
			var tempFolder = Path.Combine(folderRoot, "setup.exeComingSoon");
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
