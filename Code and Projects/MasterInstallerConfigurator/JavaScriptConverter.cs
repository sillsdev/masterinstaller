// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)
using System;
using System.Collections.Generic;
using System.IO;
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
				// If the model already has a version number then we must have converted it before, don't reconvert
				if (!string.IsNullOrEmpty(configurationModel.Version))
					return;
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
								if(!flavor.IncludedProductTitles.Contains(product.Title))
									flavor.IncludedProductTitles.Add(product.Title);
							}
							else if (currentLine.StartsWith("NextStage"))
							{
								currentState = ReadingState.Tasks;
							}
							break;
						case ReadingState.Tasks:
							// Handle lines that look like :
							// SetElement("OutputPath", "G:\\Software Package Builder\\Web Downloads\\FW 8.2.2 SE");
							// SelectElement("WriteXml", true);
							// SetElement("SfxStyle", "UseFwSfx");
							if (configurationModel.Tasks == null)
							{
								configurationModel.Tasks = new ConfigurationModel.TasksToExecuteSettings();
							}
							if (@currentLine.StartsWith("SetElement") || currentLine.StartsWith("SelectElement") && currentLine.Contains("true"))
							{
								var sections = currentLine.Split('"');
								var elementName = sections[1];
								switch (elementName)
								{
									case "OutputPath":
										configurationModel.Tasks.OutputFolder = sections[3];
										break;
									case "WriteXml":
										configurationModel.Tasks.WriteInstallerXml = true;
										break;
									case "WriteDownloadsXml":
										configurationModel.Tasks.WriteDownloadsXml = true;
										break;
									case "Compile" :
										configurationModel.Tasks.Compile = true;
										break;
									case "GatherFiles" :
										configurationModel.Tasks.GatherFiles = true;
										break;
									case "BuildSfx":
										configurationModel.Tasks.BuildSelfExtractingDownloadPackage = true;
										break;
									case "SaveSettings" :
										configurationModel.Tasks.RememberSettings = true;
										break;
									case "SfxStyle" :
										configurationModel.Tasks.SelfExtractingStyle = sections[3];
										break;
									default:
										throw new Exception("Invalid javascript file, unknown option: " + elementName);
								}
							}
							break;
					}
				} // end while state machine
				xmlConfigStream.Position = 0; // Reset the stream based on the safe assumption that we will always write at least as much as we read
				xmlConfigStream.SetLength(0);
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
