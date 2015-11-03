// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)
namespace MasterInstallerConfigurator
{
	public class ConfigurationController
	{
		private ConfigurationModel _model;
		public ConfigurationController(ConfigurationModel model)
		{
			_model = model;
		}

		public void PopulateWithModelSettings(ConfigurationWizard configurationWizard)
		{
			foreach (var flavor in _model.Flavors)
			{
				configurationWizard.AddFlavor(flavor.FlavorName, flavor.DownloadURL);
			}
		}
	}
}
