using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Xml.Serialization;
using MasterInstallerConfigurator;
using NUnit.Framework;

namespace MasterInstallerConfiguratorTests
{
	[TestFixture]
	internal class JavaScriptConverterTests
	{
		private string TestFolder { get; set; }

		[SetUp]
		public void SetUp()
		{
			TestFolder = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
			Directory.CreateDirectory(TestFolder);
		}

		[TearDown]
		public void TearDown()
		{
			Directory.Delete(TestFolder, true);
		}

		[Test]
		public void SingleFeatureNoOptionsAddsFeature()
		{
			var simpleScript = 
				@"// Fills in details for download packages automatically.
				// This instance created AUTOMATICALLY during a previous run.
				function AutomatePackages()
				{
					SetElement(""FlavorName1"", ""NAME"");
					SetElement(""FlavorUrl1"", ""URL"");
					NextStage();
					NextStage();
				}";
			var configuration = new ConfigurationModel();
			var scriptFile = Path.Combine(TestFolder, "test.js");
			var installerFile = Path.Combine(TestFolder, "installer.xml");
			File.WriteAllText(scriptFile, simpleScript);
			configuration.Save(installerFile);
			JavaScriptConverter.ConvertJsToXml(scriptFile, installerFile);
			var serializer = new XmlSerializer(typeof (ConfigurationModel));
			using (var textReader = new StreamReader(installerFile))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				Assert.That(model.Flavors, Is.Not.Null);
				Assert.That(model.Flavors.Count, Is.EqualTo(1));
				Assert.That(model.Flavors.First().FlavorName, Is.EqualTo("NAME"));
				Assert.That(model.Flavors.First().DownloadURL, Is.EqualTo("URL"));
			}
		}

		[Test]
		public void TestFeatureProductSelection()
		{
			var flavorName1 = "flavorA";
			var flavorName2 = "flavorB";
			var url1 = "http://a.com";
			var url2 = "http://b.com";
			var outputPath = "D:/Test";
			var sfxStyle = "Hmm";
			var complexScript =
				@"// Fills in details for download packages automatically.
				// This instance created AUTOMATICALLY during a previous run.
				function AutomatePackages()
				{
				" +
				string.Format(@"SetElement(""FlavorName1"", ""{0}"");
					SetElement(""FlavorUrl1"", ""{1}"");
					AddFlavor();
					SetElement(""FlavorName2"", ""{2}"");
					SetElement(""FlavorUrl2"", ""{3}"");

					NextStage();

					SelectElement(""IncludedF1P1"", true);
					SelectElement(""IncludedF1P2"", false);

					SelectElement(""IncludedF2P1"", false);
					SelectElement(""IncludedF2P2"", true);

					NextStage();", flavorName1, url1, flavorName2, url2) +
					@"
				}";

			var configuration = new ConfigurationModel();
			// The configuration needs to have 2 Products for the given JavaScript
			configuration.Products = new List<ConfigurationModel.Product>();
			var mainProduct = "Main Product";
			configuration.Products.Add(new ConfigurationModel.Product { Title = mainProduct });
			var dependency = "Dependency";
			configuration.Products.Add(new ConfigurationModel.Product { Title = dependency });
			var scriptFile = Path.Combine(TestFolder, "test.js");
			var installerFile = Path.Combine(TestFolder, "installer.xml");
			File.WriteAllText(scriptFile, complexScript);
			configuration.Save(installerFile);
			JavaScriptConverter.ConvertJsToXml(scriptFile, installerFile);
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StreamReader(installerFile))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				Assert.That(model.Flavors, Is.Not.Null);
				Assert.That(model.Flavors.Count, Is.EqualTo(2));
				var firstFlavor = model.Flavors[0];
				var secondFlavor = model.Flavors[1];
				Assert.That(firstFlavor.FlavorName, Is.EqualTo(flavorName1));
				Assert.That(firstFlavor.DownloadURL, Is.EqualTo(url1));
				Assert.That(firstFlavor.IncludedProductTitles[0], Is.EqualTo(mainProduct));
				Assert.That(secondFlavor.FlavorName, Is.EqualTo(flavorName2));
				Assert.That(secondFlavor.DownloadURL, Is.EqualTo(url2));
				Assert.That(secondFlavor.IncludedProductTitles[0], Is.EqualTo(dependency));
			}
		}

		[Test]
		public void TestTasksConversion()
		{
			var flavorName1 = "flavorA";
			var url1 = "http://a.com";
			var outputPath = "D:/Test";
			var sfxStyle = "Hmm";
			var complexScript =
				@"// Fills in details for download packages automatically.
				// This instance created AUTOMATICALLY during a previous run.
				function AutomatePackages()
				{
				" +
				string.Format(@"SetElement(""FlavorName1"", ""{0}"");
					SetElement(""FlavorUrl1"", ""{1}"");
					NextStage();
					SelectElement(""IncludedF1P1"", true);
					NextStage();

					SetElement(""OutputPath"", ""{2}"");
					SelectElement(""WriteXml"", true);
					SelectElement(""WriteDownloadsXml"", true);
					SelectElement(""Compile"", true);
					SelectElement(""GatherFiles"", true);
					SelectElement(""BuildSfx"", true);
					SetElement(""SfxStyle"", ""{3}"");
					SetElement(""SaveSettings"", true);", flavorName1, url1, outputPath, sfxStyle) +
				@"
				}";

			var configuration = new ConfigurationModel();
			configuration.Products = new List<ConfigurationModel.Product>();
			var mainProduct = "Main Product";
			configuration.Products.Add(new ConfigurationModel.Product {Title = mainProduct});
			var scriptFile = Path.Combine(TestFolder, "test.js");
			var installerFile = Path.Combine(TestFolder, "installer.xml");
			File.WriteAllText(scriptFile, complexScript);
			configuration.Tasks = new ConfigurationModel.TasksToExecuteSettings();
			configuration.Save(installerFile);
			// Prove that values started as false
			Assert.That(configuration.Tasks.BuildSelfExtractingDownloadPackage, Is.False, "Setup did not initiate values to false, test invalid");
			Assert.That(configuration.Tasks.Compile, Is.False, "Setup did not initiate values to false, test invalid");
			Assert.That(configuration.Tasks.GatherFiles, Is.False, "Setup did not initiate values to false, test invalid");
			Assert.That(configuration.Tasks.WriteDownloadsXml, Is.False, "Setup did not initiate values to false, test invalid");
			Assert.That(configuration.Tasks.WriteInstallerXml, Is.False, "Setup did not initiate values to false, test invalid");
			Assert.That(configuration.Tasks.RememberSettings, Is.False, "Setup did not initiate values to false, test invalid");
			// SUT
			JavaScriptConverter.ConvertJsToXml(scriptFile, installerFile);
			var serializer = new XmlSerializer(typeof (ConfigurationModel));
			using (var textReader = new StreamReader(installerFile))
			{
				var model = (ConfigurationModel) serializer.Deserialize(textReader);
				Assert.That(model.Tasks.OutputFolder, Is.EqualTo(outputPath));
				Assert.That(model.Tasks.BuildSelfExtractingDownloadPackage, Is.True);
				Assert.That(model.Tasks.Compile, Is.True);
				Assert.That(model.Tasks.GatherFiles, Is.True);
				Assert.That(model.Tasks.WriteDownloadsXml, Is.True);
				Assert.That(model.Tasks.WriteInstallerXml, Is.True);
				Assert.That(model.Tasks.RememberSettings, Is.True);
			}
		}
	}
}
