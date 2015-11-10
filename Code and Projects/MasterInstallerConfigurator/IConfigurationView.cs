// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.Collections.Generic;

namespace MasterInstallerConfigurator
{
	public interface IConfigurationView
	{
		void AddFlavor(string flavorName, string flavorUrl);
		void AddProductConfigurationRows(Dictionary<string, IEnumerable<string>> products, IList<string> flavors);
		void ClearAll();
		bool GatherFiles { get; set; }
		bool Compile { get; set; }
		bool BuildSelfExtractingDownloadPackage { get; set; }
		bool RememberSettings { get; set; }
		bool WriteInstallerXml { get; set; }
		bool WriteDownloadsXml { get; set; }
		string OutputFolder { get; set; }
		string SelfExtractingStyle { get; set; }
	}
}