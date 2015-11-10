// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System;
using System.Collections.Generic;
using System.Linq;

namespace MasterInstallerConfigurator
{
	public class ConfigurationController
	{
		private ConfigurationModel _model;
		public ConfigurationController(ConfigurationModel model)
		{
			_model = model;
		}

		public void PopulateWithModelSettings(IConfigurationView configurationWizard)
		{
			if (_model.Flavors != null)
			{
				foreach (var flavor in _model.Flavors)
				{
					configurationWizard.AddFlavor(flavor.FlavorName, flavor.DownloadURL);
				}
			}
			var flavorProducts = new Dictionary<string, IEnumerable<string>>();
			if (_model.Products != null)
			{
				foreach (var product in _model.Products)
				{
					flavorProducts[product.Title] =
						_model.Flavors.Where(flavor => flavor.IncludedProductTitles.Contains(product.Title)).Select(f => f.FlavorName);
				}
				configurationWizard.AddProductConfigurationRows(flavorProducts, new List<string>(_model.Flavors.Select(f => f.FlavorName)));
			}
			if (_model.Tasks != null)
			{
				configurationWizard.OutputFolder = _model.Tasks.OutputFolder;
				configurationWizard.Compile = _model.Tasks.Compile;
				configurationWizard.BuildSelfExtractingDownloadPackage = _model.Tasks.BuildSelfExtractingDownloadPackage;
				configurationWizard.GatherFiles = _model.Tasks.GatherFiles;
				configurationWizard.RememberSettings = _model.Tasks.RememberSettings;
				configurationWizard.SelfExtractingStyle = _model.Tasks.SelfExtractingStyle;
				configurationWizard.WriteInstallerXml = _model.Tasks.WriteInstallerXml;
				configurationWizard.WriteDownloadsXml = _model.Tasks.WriteDownloadsXml;
			}
		}

		public void ExecuteTasks(ConfigurationWizard configurationWizard)
		{
			if (configurationWizard.RememberSettings)
			{
				_model.Flavors.Clear();
				foreach (var flavor in configurationWizard.GetFlavors())
				{
					_model.Flavors.Add(new ConfigurationModel.FlavorOptions { FlavorName = flavor.Item1, DownloadURL = flavor.Item2,
						IncludedProductTitles = configurationWizard.GetIncludedProducts(flavor.Item1)});
				}
				_model.Tasks.OutputFolder = configurationWizard.OutputFolder;
				_model.Tasks.Compile = configurationWizard.Compile;
				_model.Tasks.BuildSelfExtractingDownloadPackage = configurationWizard.BuildSelfExtractingDownloadPackage;
				_model.Tasks.GatherFiles = configurationWizard.GatherFiles;
				_model.Tasks.RememberSettings = configurationWizard.RememberSettings;
				_model.Tasks.SelfExtractingStyle = configurationWizard.SelfExtractingStyle;
				_model.Tasks.WriteInstallerXml = configurationWizard.WriteInstallerXml;
				_model.Tasks.WriteDownloadsXml = configurationWizard.WriteDownloadsXml;
			}
			_model.Save();
		}

		public void AddFlavor(IConfigurationView wizard, string flavorName, string flavorUrl)
		{
			_model.Flavors.Add(new ConfigurationModel.FlavorOptions { FlavorName = flavorName, DownloadURL = flavorUrl, IncludedProductTitles = new List<string>()});
			wizard.ClearAll();
			PopulateWithModelSettings(wizard);
		}
	}
}
