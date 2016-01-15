// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace MasterInstallerConfigurator
{
	class MasterInstallerCompiler
	{
		private const string GeneratedCppHeaderString = "/*\tThis file is ALWAYS PRODUCED AUTOMATICALLY.\n\tDo not edit it, as it may be overwritten without warning.\n\tThe source for producing this file is an XML document.\n*/\n";

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
				GenerateProductsCpp(cppFilePath, model);
				GenerateConfigFunctionsCpp(cppFilePath, model);
				GenerateGlobalFlags(cppFilePath, model);
				/*
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigGlobals.xsl", CppFilePath + "\\AutoGlobals.h");
	ProcessConfigFile(xmlDoc, CppFilePath + "\\ConfigResources.xsl", CppFilePath + "\\AutoResources.rc");
				 */
			}
		}

		private static void GenerateGlobalFlags(string cppFilePath, ConfigurationModel model)
		{
			var globalFlags = new StringBuilder();
			globalFlags.AppendLine("// External global flags:");
			foreach (var product in model.Products)
			{
				if (!string.IsNullOrEmpty(product.CriticalFile))
				{
					globalFlags.AppendLine(string.Format("extern bool {0};", product.CriticalFile));
				}
				if (product.Install != null && !string.IsNullOrEmpty(product.Install.Flag))
				{
					globalFlags.AppendLine(string.Format("extern bool {0};", product.Install.Flag));
				}
			}
			File.WriteAllText(Path.Combine(cppFilePath, "AutoGlobals.h"), GeneratedCppHeaderString + globalFlags);
		}

		private static void GenerateConfigFunctionsCpp(string cppFilePath, ConfigurationModel model)
		{
			var testPresence = new StringBuilder();
			var preInstall = new StringBuilder();
			var install = new StringBuilder();
			var postInstall = new StringBuilder();
			testPresence.AppendLine("// TestPresence functions:");
			install.AppendLine("// Installation functions:");
			preInstall.AppendLine("// Preinstallation functions:");
			postInstall.AppendLine("// Postinstallation functions:");
			foreach (var product in model.Products)
			{
				if (product.TestPresence != null) // The xslt used to test not(@Type='External') but Type attribute was not present in any Installer Definitions
				{
					testPresence.AppendLine(string.Format("#include {0}.cpp", product.TestPresence.TestFunction));
				}
				if (!string.IsNullOrEmpty(product.Preinstall)) // the xslt used to test not(@Type='External') and not(@ignoreError='true') but these are obsolete
				{
					preInstall.AppendLine(string.Format("#include {0}.cpp", product.Preinstall));
				}
				if (product.Install != null && product.Install.Type != null && product.Install.Type.ToLower() == "internal")
				{
					install.AppendLine(string.Format("#include {0}.cpp", product.Install.InstallFunction));
				}
				if (product.PostInstall != null) // The xslt used to test not(@Type='External') but Type attribute was not present in any Installer Definitions
				{
					install.AppendLine(string.Format("#include {0}.cpp", product.PostInstall.PostInstallFunction));
				}
			}
			File.WriteAllText(Path.Combine(cppFilePath, "ConfigFunctions.cpp"), GeneratedCppHeaderString + testPresence + preInstall + install + postInstall);
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
			var preInstallDeps = new StringBuilder();
			var postInstallationTests = new StringBuilder();
			foreach (var product in model.Products)
			{
				GenerateProductExternalFunctions(product, externalFunctions);
				GenerateProductPrerequisites(product, preInstallDeps);
				GenerateProductRequires(product, postInstallationTests);
				productConfigurations.AppendLine("\t{");
				GenerateProductStringVariable(product.Tag, "Name used in dependency lists", productConfigurations);
				GenerateProductStringVariable(product.Title, "Title", productConfigurations);
				GenerateProductStringVariable(product.ProductCode, "MSI Product Code", productConfigurations);
				GenerateProductStringVariable(product.Version, "Version for Product Key test", productConfigurations);
				GenerateProductStringVariable(null, "Feature (Obsolete)", productConfigurations);
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
				GenerateProductTestPresenceVariables(product.TestPresence, productConfigurations);
				GenerateProductPostInstallationVariables(product.PostInstall, productConfigurations);
				GenerateProductPathVariable(product.Help != null ? product.Help.HelpFile : null, "Help tag", productConfigurations);
				GenerateProductBoolVariable(product.Help != null && product.Help.Internal, "Help tag", productConfigurations);
				productConfigurations.AppendLine("\t},");
			}
			productConfigurations.AppendLine("};");
			CompleteDependencyArrays(preInstallDeps, postInstallationTests);
			File.WriteAllText(Path.Combine(cppFilePath, "ConfigProducts.cpp"), GeneratedCppHeaderString + externalFunctions + productConfigurations + preInstallDeps + postInstallationTests);
		}

		private static void CompleteDependencyArrays(StringBuilder preInstallDeps, StringBuilder postInstallationTests)
		{
			if (preInstallDeps.Length > 0)
			{
				var deps = preInstallDeps.ToString();
				var itemCount = deps.Count(x => x == '{');
				preInstallDeps.Clear();
				preInstallDeps.AppendLine("// List of pre-installation dependencies.");
				preInstallDeps.AppendLine("static const Mapping Prerequisites[] =\n{");
				preInstallDeps.AppendLine(deps);
				preInstallDeps.AppendLine("};");
				preInstallDeps.AppendLine(string.Format("static const int kctPrerequisites = {0};", itemCount));
			}
			else
			{
				preInstallDeps.AppendLine("// No pre-installation dependencies.");
				preInstallDeps.AppendLine("static const Mapping * Prerequisites = NULL;");
				preInstallDeps.AppendLine("static const int kctPrerequisites = 0;");
			}
			if (postInstallationTests.Length > 0)
			{
				var deps = postInstallationTests.ToString();
				var itemCount = deps.Count(x => x == '{');
				postInstallationTests.Clear();
				postInstallationTests.AppendLine("// List of post-installation dependencies.");
				postInstallationTests.AppendLine("static const Mapping RunDependencies[] =\n{");
				postInstallationTests.AppendLine(deps);
				postInstallationTests.AppendLine("};");
				postInstallationTests.AppendLine(string.Format("static const int kctRunDependencies = {0};", itemCount));
			}
			else
			{
				postInstallationTests.AppendLine("// No post-installation dependencies.");
				postInstallationTests.AppendLine("static const Mapping * RunDependencies = NULL;");
				postInstallationTests.AppendLine("static const int kctRunDependencies = 0;");
			}
		}

		private static void GenerateProductRequires(ConfigurationModel.Product product, StringBuilder postInstallTests)
		{
			if (product.Requires != null)
			{
				GenerateRequirementArrayEntries(product.Tag, product.Requires, postInstallTests);
			}
		}

		private static void GenerateProductPrerequisites(ConfigurationModel.Product product, StringBuilder preInstallDeps)
		{
			if(product.Prerequisite != null)
			{
				GenerateRequirementArrayEntries(product.Tag, product.Prerequisite, preInstallDeps);
			}
		}

		private static void GenerateRequirementArrayEntries(string productTag, List<ConfigurationModel.RequirementOption> requirements, StringBuilder builder)
		{
			foreach (var preReq in requirements)
			{
				builder.AppendLine("{");
				GenerateProductStringVariable(productTag, "Product tag", builder);
				GenerateProductStringVariable(preReq.Tag, "Requirement tag", builder);
				if (!string.IsNullOrEmpty(preReq.Version))
				{
					GenerateProductStringVariable(preReq.Version, "Version (min)", builder);
					GenerateProductStringVariable(preReq.Version, "Version (max)", builder);
				}
				else
				{
					GenerateProductStringVariable(preReq.MinVersion, "MinVersion", builder);
					GenerateProductStringVariable(preReq.MaxVersion, "MinVersion", builder);
				}
				GenerateProductStringVariable(preReq.FailMsg, "Fail message (for post requirements)", builder);
				GenerateProductStringVariable(preReq.MinOS, "Min OS version", builder);
				GenerateProductStringVariable(preReq.MaxOS, "Max OS version", builder);
				builder.AppendLine("},");
			}
		}

		private static void GenerateProductPostInstallationVariables(ConfigurationModel.PostInstallOptions postInstall, StringBuilder productConfigurations)
		{
			if (postInstall != null)
			{
				GenerateProductFunctionVariable(postInstall.PostInstallFunction, "Postinstallation function", productConfigurations);
			}
			else
			{
				GenerateProductFunctionVariable(null, "No Postinstallation function", productConfigurations);
			}
			GenerateProductFunctionVariable(null, "No Postinstallation command (Obsolete)", productConfigurations);
		}

		private static void GenerateProductTestPresenceVariables(ConfigurationModel.TestPresenceOptions testPresenceOptions, StringBuilder productConfigurations)
		{
			if (testPresenceOptions != null)
			{
				GenerateProductFunctionVariable(testPresenceOptions.TestFunction, "TestPresence function", productConfigurations);
				GenerateProductFunctionVariable(null, "No TestPresence command (Obsolete)", productConfigurations);
				GenerateProductStringVariable(testPresenceOptions.Version, "TestPresence version parameter", productConfigurations);
			}
			else
			{
				GenerateProductFunctionVariable(null, "No TestPresence function", productConfigurations);
				GenerateProductFunctionVariable(null, "No TestPresence command (Obsolete)", productConfigurations);
				GenerateProductStringVariable(null, "No TestPresence version parameter", productConfigurations);
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
			GenerateProductStringVariable(!string.IsNullOrEmpty(path) ? path.Replace(@"\", @"\\") : path, comment, builder);
		}

		private static void GenerateProductStringVariable(string variableValue, string comment, StringBuilder builder)
		{
			var stringValue = "NULL";
			if (!string.IsNullOrEmpty(variableValue))
			{
				stringValue = string.Format("_T(\"{0}\")", variableValue);
			}
			builder.AppendLine(string.Format("\t\t{0}, // {1}", stringValue, comment));
		}

		private static void GenerateProductFunctionVariable(string preInstall, string comment, StringBuilder builder)
		{
			builder.AppendLine(string.Format("\t\t{0}, // {1}", string.IsNullOrEmpty(preInstall) ? "NULL" : preInstall, comment));
		}

		private static void GenerateProductIntVariable(int integerValue, string comment, StringBuilder productConfigurations)
		{
			GenerateProductIntVariable(integerValue.ToString(), "", comment, productConfigurations);
		}

		private static void GenerateProductIntVariable(string integerString, string valueIfEmpty, string comment, StringBuilder stringBuilder)
		{
			var intValue = valueIfEmpty;
			if (!string.IsNullOrEmpty(integerString))
				intValue = integerString;
			stringBuilder.AppendLine(string.Format("\t\t{0}, // {1}", intValue, comment));
		}

		private static void GenerateProductBoolVariable(bool boolValue, string comment, StringBuilder productConfigurations)
		{
			productConfigurations.AppendLine(string.Format("\t\t{0}, // {1}", boolValue.ToString().ToLower(), comment));
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
