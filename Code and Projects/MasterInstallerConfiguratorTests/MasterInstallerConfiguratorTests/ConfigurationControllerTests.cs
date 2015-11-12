using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MasterInstallerConfigurator;
using NUnit.Framework;

namespace MasterInstallerConfiguratorTests
{
	/// <summary>
	/// Summary description for ConfigurationControllerTests
	/// </summary>
	[TestFixture]
	public class ConfigurationControllerTests
	{
		[Test]
		public void AddFlavorWorks()
		{
			var model = new ConfigurationModel();
			var flavorName = "TestFlavor";
			model.Flavors = new List<ConfigurationModel.FlavorOptions> { new ConfigurationModel.FlavorOptions { FlavorName = flavorName}};
			var controller = new ConfigurationController(model);
			var testView = new ConfigViewForTests();
			controller.PopulateWithModelSettings(testView);
			TestThatViewHasFlavor(testView, model.Flavors[0]);
		}

		[Test]
		public void DeleteFlavorWorks()
		{
			var deleteFlavor = new ConfigurationModel.FlavorOptions { FlavorName = "DeletedFlavor", DownloadURL = "localhost" };
			var model = new ConfigurationModel();
			var flavorName = "TestFlavor";
			model.Flavors = new List<ConfigurationModel.FlavorOptions>
			{
				new ConfigurationModel.FlavorOptions { FlavorName = flavorName },
				deleteFlavor
			};
			var controller = new ConfigurationController(model);
			var testView = new ConfigViewForTests();
			controller.PopulateWithModelSettings(testView);
			TestThatViewHasFlavor(testView, model.Flavors[0]);
			TestThatViewHasFlavor(testView, model.Flavors[1]);
			// SUT
			controller.DeleteLastFlavor(testView);
			TestThatViewHasFlavor(testView, model.Flavors[0]);
			TestThatViewDoesNotHaveFlavor(testView, deleteFlavor);
		}

		[Test]
		public void AddProductConfigurationRows()
		{
			var model = new ConfigurationModel();
			var flavorNameA = "TestFlavor";
			var flavorNameB = "TestFlavor2";
			var productA = "ProductA";
			var productB = "ProductB";
			model.Flavors = new List<ConfigurationModel.FlavorOptions>
			{
				new ConfigurationModel.FlavorOptions
				{
					FlavorName = flavorNameA, IncludedProductTitles = new List<string> { productA }
				},
				new ConfigurationModel.FlavorOptions
				{
					FlavorName = flavorNameB, IncludedProductTitles = new List<string> { productB }
				}
			};
			var product = new ConfigurationModel.Product {Title = productB};
			model.Products = new List<ConfigurationModel.Product>
			{
				new ConfigurationModel.Product {Title = productA},
				product
			};
			var controller = new ConfigurationController(model);
			var testView = new ConfigViewForTests();
			controller.PopulateWithModelSettings(testView);
			TestThatViewHasProduct(testView, model.Products[0]);
			TestThatViewHasProduct(testView, model.Products[1]);
			TestThatProductIsSelectedForTheseFlavors(testView, model.Products[0], new List<string> { flavorNameA });
			TestThatProductIsSelectedForTheseFlavors(testView, model.Products[1], new List<string> { flavorNameB });
		}

		[Test]
		public void AddTaskSelections()
		{
			var model = new ConfigurationModel();
			model.Tasks = new ConfigurationModel.TasksToExecuteSettings
			{
				GatherFiles = true,
				BuildSelfExtractingDownloadPackage = true,
				Compile = true,
				OutputFolder = "C:/temp",
				RememberSettings = true,
				SelfExtractingStyle = "SfxFw",
				WriteInstallerXml = true,
				WriteDownloadsXml = true
			};
			var controller = new ConfigurationController(model);
			var testView = new ConfigViewForTests();
			// Assert defaults to ensure a valid test
			Assert.False(testView.GatherFiles || testView.Compile || testView.BuildSelfExtractingDownloadPackage ||
			             testView.RememberSettings || testView.WriteInstallerXml || testView.WriteDownloadsXml, "Expected all booleans to default to false, test invalid");
			controller.PopulateWithModelSettings(testView);
			Assert.True(testView.GatherFiles && testView.Compile && testView.BuildSelfExtractingDownloadPackage &&
							 testView.RememberSettings && testView.WriteInstallerXml && testView.WriteDownloadsXml, "Expected all booleans to be set to true");
			Assert.That(testView.OutputFolder, Is.EqualTo(model.Tasks.OutputFolder));
			Assert.That(testView.SelfExtractingStyle, Is.EqualTo(model.Tasks.SelfExtractingStyle));
		}

		private void TestThatProductIsSelectedForTheseFlavors(ConfigViewForTests testView, ConfigurationModel.Product product, List<string> list)
		{
			CollectionAssert.AreEquivalent(testView.enabledFlavorsForProduct[product.Title], list);
		}

		private void TestThatViewHasFlavor(ConfigViewForTests testView, ConfigurationModel.FlavorOptions flavorOptions)
		{
			CollectionAssert.Contains(testView.flavorNames, flavorOptions.FlavorName);
			CollectionAssert.Contains(testView.flavorUrls, flavorOptions.DownloadURL);
		}

		private void TestThatViewDoesNotHaveFlavor(ConfigViewForTests testView, ConfigurationModel.FlavorOptions flavorOptions)
		{
			CollectionAssert.DoesNotContain(testView.flavorNames, flavorOptions.FlavorName);
			CollectionAssert.DoesNotContain(testView.flavorUrls, flavorOptions.DownloadURL);
		}

		private void TestThatViewHasProduct(ConfigViewForTests testView, ConfigurationModel.Product product)
		{
			CollectionAssert.Contains(testView.productNames, product.Title);
		}

		private void TestMessageLogged(ConfigViewForTests testView, string error)
		{
			testView.loggedText.ToString().Contains(error);
		}

		public class ConfigViewForTests : IConfigurationView
		{
			public List<string> flavorNames = new List<string>();
			public List<string> flavorUrls = new List<string>();

			public List<string> productNames = new List<string>();
			public Dictionary<string, List<string>> enabledFlavorsForProduct = new Dictionary<string, List<string>>();

			public StringBuilder loggedText = new StringBuilder();

			public void AddFlavor(string flavorName, string flavorUrl)
			{
				flavorNames.Add(flavorName);
				flavorUrls.Add(flavorUrl);
			}

			public void AddProductConfigurationRows(Dictionary<string, IEnumerable<string>> products, IList<string> flavors)
			{
				foreach (var productName in products.Keys)
				{
					productNames.Add(productName);
					var flavorsForProduct = products[productName].ToList();
					enabledFlavorsForProduct[productName] = flavorsForProduct;
				}
			}

			public void ClearAll()
			{
				flavorNames.Clear();
				flavorUrls.Clear();
				productNames.Clear();
				enabledFlavorsForProduct.Clear();
			}

			public bool GatherFiles { get; set; }
			public bool Compile { get; set; }
			public bool BuildSelfExtractingDownloadPackage { get; set; }
			public bool RememberSettings { get; set; }
			public bool WriteInstallerXml { get; set; }
			public bool WriteDownloadsXml { get; set; }
			public string OutputFolder { get; set; }
			public string SelfExtractingStyle { get; set; }

			public void LogErrorLine(string errorMessage)
			{
				loggedText.AppendLine(errorMessage);
			}

			public void LogProgressLine(string errorMessage)
			{
				loggedText.AppendLine(errorMessage);
			}
		}
	}
}
