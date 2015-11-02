﻿using System.Collections.Generic;
using System.Security.Policy;
using System.Xml.Serialization;

namespace MasterInstallerConfigurator
{
	[XmlRoot("MasterInstaller", IsNullable = false)]
	public class ConfigurationModel
	{
		public SetupOptions AutoConfigure { get; set; }

		public GeneralOptions General { get; set; }

		public List<Product> Products { get; set; }


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

			public string Install { get; set; }

			public TestPresenceOptions TestPresence { get; set; }

			public string DownloadURL { get; set; }

			public string Commentary { get; set; }

			public string StatusWindow { get; set; }

			public bool FlushReboot { get; set; }

			[XmlElement(ElementName = "Prerequisite")]
			public List<PrerequisiteOptions> Prerequisite { get; set; }

			public RequiresOption Requires { get; set; }

			public string ProductCode { get; set; }

			public string Help { get; set; }

			public int CDNumber { get; set; }
		}

		public class GeneralOptions
		{
			public string Title { get; set; }

			public string ListSubtitle { get; set; }

			public ListBackgroundOptions ListBackground { get; set; }

			[XmlElement(ElementName = "ListEvenOneProduct")]
			public bool ListEvenOne { get; set; }

			public int ListSpacingAdjust { get; set; }

			public int InfoButtonAdjust { get; set; }

			public bool StartFromAnyCD { get; set; }

			[XmlElement(ElementName = "KeyPromptNeedsShiftCtrl")]
			public bool KeyPromptNeedsKeys { get; set; }

			[XmlElement(ElementName = "GetKeyTitle")]
			public string KeyTitle { get; set; }

			[XmlElement(ElementName = "ExternalHelpFile")]
			public string HelpFile { get; set; }

			public string HelpButtonText { get; set; }

			public string TermsOfUseFile { get; set; }

			[XmlElement(ElementName = "TermsButtonText")]
			public string TermsOfUseButtonText { get; set; }
		}

		public class ListBackgroundOptions
		{
			[XmlText]
			public string ImagePath { get; set; }
		}

		public class PrerequisiteOptions
		{
			[XmlAttribute]
			public string Tag { get; set; }

			[XmlAttribute]
			public string Version { get; set; }
		}

		public class TestPresenceOptions
		{
			[XmlText]
			public string TestValue { get; set; }

			[XmlAttribute]
			string Version { get; set; }
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
	}
}
