﻿// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml.Serialization;

namespace MasterInstallerConfigurator
{
	[XmlRoot("MasterInstaller", IsNullable = false)]
	public class ConfigurationModel
	{
		internal static string CurrentModelVersion = "1.0";
		[XmlAttribute]
		public string Version { get; set; }

		[XmlIgnore]
		public string FileLocation { get; set; }

#region These properties are used in the build web Packages wizard
		public List<FlavorOptions> Flavors { get; set; }

		public class FlavorOptions
		{
			public string DownloadURL { get; set; }
			public string FlavorName { get; set; }

			public List<string> IncludedProductTitles { get; set; }
		}

		public TasksToExecuteSettings Tasks { get; set; }

		public class TasksToExecuteSettings
		{
			public string OutputFolder { get; set; }
			public bool WriteInstallerXml { get; set; }
			public bool WriteDownloadsXml { get; set; }
			public bool Compile { get; set; }
			public bool GatherFiles { get; set; }
			public bool BuildSelfExtractingDownloadPackage { get; set; }
			public bool RememberSettings { get; set; }
			public string SelfExtractingStyle { get; set; }
		}

#endregion
		public SetupOptions AutoConfigure { get; set; }

		public GeneralOptions General { get; set; }

		public List<Product> Products { get; set; }

		public CDOptionContianer CDs { get; set; }

		public class CDOptionContianer
		{
			[XmlElement]
			public List<CDOptions> CD;

			public class CDOptions
			{
				[XmlElement]
				public string Title;

				[XmlElement]
				public string VolumeLabel;
			}
		}

		public class SetupOptions
		{
			[XmlElement(ElementName = "CppFilePath")]
			public string CppPath { get; set; }

			[XmlElement(ElementName = "ProductsPath")]
			public string ProductsPath { get; set; }

			[XmlElement(ElementName = "CDsPath")]
			public string CdPath { get; set; }

			[XmlElement(ElementName = "ExternalHelpSource")]
			public string HelpSource { get; set; }

			[XmlElement(ElementName = "ExternalHelpDestination")]
			public string HelpDest { get; set; }

			[XmlElement(ElementName = "TermsOfUseSource")]
			public string TermsSource { get; set; }

			[XmlElement(ElementName = "TermsOfUseDestination")]
			public string TermsDest { get; set; }

			[XmlElement(ElementName = "CdImagePath")]
			public string ImagePath { get; set; }

			[XmlElement(ElementName = "CdSize")]
			public int CdSize { get; set; }

			[XmlElement(ElementName = "MinFileSize")]
			public int MinFileSize { get; set; }
		}

		public class Product
		{
			public object AutoConfigure { get; set; }

			public string Title { get; set; }

			public string Tag { get; set; }

			public bool MustBeAdmin { get; set; }

			public bool MustHaveWin2kOrBetter { get; set; }

			public string CriticalFile { get; set; }

			public InstallOptions Install { get; set; }

			public TestPresenceOptions TestPresence { get; set; }

			public string Preinstall { get; set; }

			public PostInstallOptions PostInstall { get; set; }

			public string DownloadURL { get; set; }

			public string Commentary { get; set; }

			public string StatusWindow { get; set; }

			public bool FlushReboot { get; set; }

			[XmlElement(ElementName = "Prerequisite")]
			public List<RequirementOption> Prerequisite { get; set; }

			[XmlElement(ElementName = "Requires")]
			public List<RequirementOption> Requires { get; set; }

			public string ProductCode { get; set; }

			public HelpOptions Help { get; set; }

			public int CDNumber { get; set; }

			[XmlAttribute]
			public string Version { get; set; }

			[XmlAttribute]
			public string KeyId { get; set; }

			[XmlAttribute]
			public bool List { get; set; }

			[XmlElement]
			public bool MustNotDelayReboot { get; set; }
		}

		public class InstallOptions
		{
			[XmlText]
			public string InstallFunction { get; set; }

			[XmlAttribute]
			public string Type { get; set; }

			[XmlAttribute]
			public string MsiFlags { get; set; }

			[XmlAttribute]
			public string Flag { get; set; }

			[XmlElement]
			public string False { get; set; }

			[XmlElement]
			public string True { get; set; }

			[XmlAttribute]
			public string MsiUpgradeFrom { get; set; }

			[XmlAttribute]
			public string MsiUpgradeTo { get; set; }
		}

		public class PostInstallOptions
		{
			[XmlAttribute]
			public bool IncludeResourcesFile { get; set; }

			[XmlText]
			public string PostInstallFunction { get; set; }
		}

		public class GeneralOptions
		{
			public string Title { get; set; }

			public string ListSubtitle { get; set; }

			public ListBackgroundOptions ListBackground { get; set; }

			[XmlElement(ElementName = "ListEvenOneProduct")]
			public bool ListEvenOne { get; set; }

			public SerializeableNullableInt ListSpacingAdjust { get; set; }

			public SerializeableNullableInt InfoButtonAdjust { get; set; }

			public bool StartFromAnyCD { get; set; }

			[XmlElement(ElementName = "KeyPromptNeedsShiftCtrl")]
			public bool KeyPromptNeedsKeys { get; set; }

			[XmlElement(ElementName = "GetKeyTitle")]
			public string KeyTitle { get; set; }

			[XmlElement(ElementName = "ExternalHelpFile")]
			public string HelpFile { get; set; }

			public string HelpButtonText { get; set; }

			public string TermsOfUseFile { get; set; }

			public string TermsButtonText { get; set; }

			public bool ShowInstallCompleteMessage { get; set; }
		}

		public class ListBackgroundOptions
		{
			[XmlText]
			public string ImagePath { get; set; }

			[XmlAttribute]
			public int Red { get; set; }

			[XmlAttribute]
			public int Green { get; set; }

			[XmlAttribute]
			public int Blue { get; set; }

			[XmlAttribute]
			public int OffsetX { get; set; }

			[XmlAttribute]
			public int OffsetY { get; set; }

			[XmlAttribute]
			public int BlendLeft { get; set; }

			[XmlAttribute]
			public int BlendRight { get; set; }

			[XmlAttribute]
			public int BlendTop { get; set; }

			[XmlAttribute]
			public int BlendBottom { get; set; }
		}

		public class RequirementOption
		{
			[XmlAttribute]
			public string Tag { get; set; }

			[XmlAttribute]
			public string Version { get; set; }

			[XmlAttribute]
			public string MinVersion { get; set; }

			[XmlAttribute]
			public string MaxVersion { get; set; }

			[XmlAttribute]
			public string MinOS { get; set; }

			[XmlAttribute]
			public string MaxOS { get; set; }

			[XmlAttribute]
			public string FailMsg { get; set; }
		}

		public class TestPresenceOptions
		{
			[XmlText]
			public string TestFunction { get; set; }

			[XmlAttribute]
			public string Version { get; set; }
		}

		public class RequiresOption
		{
			[XmlAttribute]
			public string Tag { get; set; }

			[XmlAttribute]
			public string MinVersion { get; set; }

			[XmlAttribute(AttributeName = "FailMsg")]
			public string FailMessage { get; set; }
		}

		public void Save()
		{
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textWriter = new StreamWriter(FileLocation))
			{
				serializer.Serialize(textWriter, this);
			}
		}

		public static ConfigurationModel Load(string javaScriptFile, string xmlConfigFile)
		{
			JavaScriptConverter.ConvertJsToXml(javaScriptFile, xmlConfigFile);
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textWriter = new StreamReader(xmlConfigFile))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textWriter);
				model.FileLocation = xmlConfigFile;
				return model;
			}
		}
		public static ConfigurationModel Load(string xmlConfigFile)
		{
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textWriter = new StreamReader(xmlConfigFile))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textWriter);
				model.FileLocation = xmlConfigFile;
				return model;
			}
		}
	}

	public class HelpOptions
	{
		[XmlText]
		public string HelpFile { get; set; }

		[XmlAttribute]
		public bool Internal { get; set; }
	}

	public class SerializeableNullableInt
	{
		[XmlText]
		public string ValueString { get; set; }

		public static implicit operator int(SerializeableNullableInt value)
		{
			return string.IsNullOrEmpty(value.ValueString) ? 0 : int.Parse(value.ValueString);
		}
	}
}
