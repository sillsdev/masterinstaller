// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace MasterInstallerConfigurator
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			var model = ConfigurationModel.Load("C:\\Repositories\\masterinstaller\\Installer Definitions\\FW 8.2.1 BTE\\AutoBuildDownloadPackages.js",
				"C:\\Repositories\\masterinstaller\\Installer Definitions\\FW 8.2.1 BTE\\FW 8.2.1 BTE.xml");
			var controller = new ConfigurationController(model);
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new ConfigurationWizard(controller));
		}
	}
}
