// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.Windows.Forms;

namespace MasterInstallerConfigurator
{
	public partial class AddFlavorDialog : Form
	{
		public AddFlavorDialog()
		{
			InitializeComponent();
		}

		public string FlavorName { get { return _flavorName.Text; } }
		public string FlavorUrl { get { return _flavorUrl.Text; } }

		private void _okBtn_Click(object sender, System.EventArgs e)
		{
			DialogResult = DialogResult.OK;
			Close();
		}

		private void _cancelBtn_Click(object sender, System.EventArgs e)
		{
			Close();
		}
	}
}
