﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml.Serialization;

namespace MasterInstallerConfigurator
{
	/// <summary>
	/// Historically certain settings were saved by writing out a javascript file that would set checkmarks in the configuration dialog.
	/// This class will convert those javascript files into xml settings in a unified Installer Definition xml file.
	/// </summary>
	public class JavaScriptConverter
	{
		public static void ConvertJsToXml(string javaScriptFile, string xmlSettingsFile)
		{
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var xmlConfigStream = new FileStream(xmlSettingsFile, FileMode.Open, FileAccess.ReadWrite))
			using (var jsReader = new StreamReader(javaScriptFile))
			{
				var configurationModel = (ConfigurationModel)serializer.Deserialize(xmlConfigStream);
				string jsLine;
				var currentState = ReadingState.Header;
				ConfigurationModel.FlavorOptions currentFlavor = null;
				while ((jsLine = jsReader.ReadLine()) != null)
				{
					var currentLine = jsLine.Trim();
					switch (currentState)
					{
						case ReadingState.Header:
							// Ignore all text until the actual function starts
							if (currentLine.StartsWith("{"))
							{
								currentFlavor = new ConfigurationModel.FlavorOptions();
								configurationModel.Flavors.Add(currentFlavor);
								currentState = ReadingState.FlavorState;
							}
							break;
						case ReadingState.FlavorState:
							// Handle lines that look like : 
							// SetElement("FlavorName1", "FW822_SE_A");
							// SetElement("FlavorUrl1", "http://downloads.sil.org/FieldWorks/8.2.2/SE/FW822_SE_A.exe");
							if (currentLine.StartsWith("SetElement"))
							{
								var sections = currentLine.Split('"');
								if (sections[1].StartsWith("FlavorName"))
								{
									// ReSharper disable once PossibleNullReferenceException - Only invalid js can cause this
									currentFlavor.FlavorName = sections[3];
								}
								else if (sections[1].StartsWith("FlavorUrl"))
								{
									// ReSharper disable once PossibleNullReferenceException - Only invalid js can cause this
									currentFlavor.DownloadURL = sections[3];
								}
								else
								{
									throw new Exception("Unsupported Javascript FileFormat");
								}
							}
							else if (currentLine.StartsWith("AddFlavor"))
							{
								currentFlavor = new ConfigurationModel.FlavorOptions();
								configurationModel.Flavors.Add(currentFlavor);
							}
							else if (currentLine.StartsWith("NextStage"))
							{
								currentState = ReadingState.FlavorProducts;
							}
							break;
						case ReadingState.FlavorProducts:
							// Handle lines that look like :
							// SelectElement("IncludedF1P1", true);
							// Note, these are 1 indexed Flavor and Product numbers
							if (currentLine.StartsWith("SelectElement") && currentLine.Contains("true"))
							{
								var flavorProductStringStart = currentLine.IndexOf("F", StringComparison.Ordinal);
								var flavorProductStringEnd = currentLine.IndexOf("\",", StringComparison.Ordinal);
								var flavorProductIndexes = currentLine.Substring(flavorProductStringStart + 1,
									flavorProductStringEnd - flavorProductStringStart - 1).Split('P');
								var flavor = configurationModel.Flavors[int.Parse(flavorProductIndexes[0]) - 1];
								var product = configurationModel.Products[int.Parse(flavorProductIndexes[1]) - 1];
								if(flavor.IncludedProductTitles == null)
									flavor.IncludedProductTitles = new List<string>();
								flavor.IncludedProductTitles.Add(product.Title);
							}
							else if (currentLine.StartsWith("NextStage"))
							{
								currentState = ReadingState.Tasks;
							}
							break;
						case ReadingState.Tasks:
							if (currentLine.StartsWith("SetElement"))
							{

							}
							else if (currentLine.StartsWith("SelectElement") && currentLine.Contains("true"))
							{

							}
							break;
					}
				} // end while state machine
				xmlConfigStream.Position = 0; // Reset the stream based on the safe assumption that we will always write at least as much as we read
				serializer.Serialize(xmlConfigStream, configurationModel);
			} // end stream usings
		}

		enum ReadingState
		{
			Header,
			FlavorState,
			FlavorProducts,
			Tasks
		}
	}
}
