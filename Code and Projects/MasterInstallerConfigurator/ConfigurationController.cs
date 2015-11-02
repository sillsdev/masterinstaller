using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

		}
	}
}
