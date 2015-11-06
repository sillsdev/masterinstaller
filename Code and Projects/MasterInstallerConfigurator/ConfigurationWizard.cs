// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)

using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace MasterInstallerConfigurator
{
	public partial class ConfigurationWizard : Form, IConfigurationView
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

		public void AddProductConfigurationRows(Dictionary<string, IEnumerable<string>> products, IList<string> flavors)
		{
			_flavorOptionTable.SuspendLayout();
			_flavorOptionTable.Controls.Clear();
			_flavorOptionTable.RowStyles.Clear();
			_flavorOptionTable.ColumnCount = 1;
			_flavorOptionTable.RowCount = 1;
			var columnCount = 1;
			// Add the flavors in a 'Header' row
			foreach (var flavor in flavors)
			{
				_flavorOptionTable.Controls.Add(new Label { Text = flavor, Anchor = AnchorStyles.Left | AnchorStyles.Top,
						Font = new Font(FontFamily.GenericSansSerif, 12.0f) },
						columnCount, 0);
				columnCount++;
				_flavorOptionTable.ColumnCount++;
			}
			var rowCount = 1;
			foreach (var product in products.Keys)
			{
				// Add the product names along the left
				_flavorOptionTable.Controls.Add(new Label { Text = product, Anchor = AnchorStyles.Left | AnchorStyles.Top,
						Font = new Font(FontFamily.GenericSansSerif, 12.0f) }, 
						0, rowCount);
				// Add the checkboxes for this product checking the ones which each flavor has selected
				for (var i = 1; i < _flavorOptionTable.ColumnCount; ++i)
				{
					_flavorOptionTable.Controls.Add(new CheckBox { Checked = products[product].Contains(flavors[i - 1]),
						Anchor = AnchorStyles.None},
						i, rowCount);
				}
				++rowCount;
				_flavorOptionTable.RowCount++;
			}
			_flavorOptionTable.ResumeLayout();
		}
	}
}
