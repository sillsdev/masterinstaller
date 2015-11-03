using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MasterInstallerConfigurator
{
	public partial class ConfigurationWizard : Form
	{
		private ConfigurationController _controller;
		public ConfigurationWizard(ConfigurationController controller)
		{
			_controller = controller;
			InitializeComponent();
			controller.PopulateWithModelSettings(this);
		}
	}
}
