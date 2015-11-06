// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System;
using System.Collections.Generic;

namespace MasterInstallerConfigurator
{
	public interface IConfigurationView
	{
		void AddFlavor(string flavorName, string flavorUrl);
		void AddProductConfigurationRows(Dictionary<string, IEnumerable<string>> products, IList<string> flavors);
	}
}