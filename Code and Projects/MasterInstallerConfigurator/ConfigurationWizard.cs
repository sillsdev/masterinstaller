// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.Drawing;
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
			var flavorNameHeader = new Label { Text = @"Flavor Name", Font = new Font(FontFamily.GenericSerif, 12.0f) };
			var flavorUrlHeader = new Label { Text = @"Flavor Url", Font = new Font(FontFamily.GenericSerif, 12.0f) };
			_flavorPanel.RowStyles[0].SizeType = SizeType.AutoSize;
			_flavorPanel.Controls.Add(flavorNameHeader, 0, 0);
			_flavorPanel.Controls.Add(flavorUrlHeader, 1, 0);
			controller.PopulateWithModelSettings(this);
		}

		public void AddFlavor(string flavorName, string flavorUrl)
		{
			_flavorConfigTab.SuspendLayout();
			var name = new TextBox
			{
				Text = flavorName,
				Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top,
				Font = new Font(FontFamily.GenericSerif, 12.0f)
			};
			var url = new TextBox
			{
				Text = flavorUrl,
				Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top,
				Font = new Font(FontFamily.GenericSerif, 12.0f)
			};
			var row = _flavorPanel.RowCount++;
			_flavorPanel.Controls.Add(name, 0, row);
			_flavorPanel.Controls.Add(url, 1, row);
			if(_flavorPanel.RowStyles.Count <= row) // This is a hack for a windows forms defect where styles don't grow in proportion to rows
				_flavorPanel.RowStyles.Add(new RowStyle(SizeType.AutoSize));
			_flavorPanel.RowStyles[row].Height = _flavorPanel.RowStyles[row - 1].Height;
			_flavorConfigTab.ResumeLayout();
		}
	}
}
