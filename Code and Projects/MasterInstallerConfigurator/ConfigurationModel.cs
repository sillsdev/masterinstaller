using System.Collections.Generic;
using System.Xml.Serialization;

namespace MasterInstallerConfigurator
{
	[XmlRootAttribute("MasterInstaller", IsNullable = false)]
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
		}

		public class GeneralOptions
		{
			public string Title { get; set; }
		}
	}
}
