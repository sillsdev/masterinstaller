using System.IO;
using System.Xml.Serialization;
using MasterInstallerConfigurator;
using NUnit.Framework;

namespace MasterInstallerConfiguratorTests
{
	[TestFixture]
	public class ConfigurationModelTests
	{
		[Test]
		public void CanPersistBasicModel()
		{
			var model = new ConfigurationModel();
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textWriter = new StringWriter())
			{
				serializer.Serialize(textWriter, model);
				Assert.That(textWriter.ToString(), Is.StringContaining("<MasterInstaller"), "Did not write out the MasterInstaller element");
			}
		}

		[Test]
		public void CanReadBasicModel()
		{
			var basicModelXmlString =
				"<MasterInstaller><AutoConfigure/><General><Title>Test Installer</Title></General><Products><Product>" +
				"<AutoConfigure><Title>Test</Title></AutoConfigure></Product></Products></MasterInstaller>";
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StringReader(basicModelXmlString))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				Assert.NotNull(model);
				Assert.NotNull(model.AutoConfigure);
				Assert.NotNull(model.General);
				CollectionAssert.IsNotEmpty(model.Products);
			}
		}

		[Test]
		public void CanReadAutoConfigureSection()
		{
			var cppPath = @"F:\CD Builder\Master Installer";
			var productsPath = @"F:\CD Builder\Products";
			var cdPath = @"F:\CD Builder\CDs";
			var helpSource = @"Documents\SILSoftwareSupport";
			var helpDest = @"SILSoftwareSupport";
			var TOUSource = @"Documents\TermsOfUse\License SIL Freeware.rtf";
			var TOUDest = @"Docs";
			var cdImagePath = @"F:\CD Builder\CDs\WordSurv 6";
			var cdSize = @"700";
			var minFileSize = @"2048";
			var basicModelXmlString =
				string.Format(@"<MasterInstaller><AutoConfigure>
					<CppFilePath>{0}</CppFilePath>
					<ProductsPath>{1}</ProductsPath>
					<CDsPath>{2}</CDsPath>
					<ExternalHelpSource>{3}</ExternalHelpSource>
					<ExternalHelpDestination>{4}</ExternalHelpDestination>
					<TermsOfUseSource>{5}</TermsOfUseSource>
					<TermsOfUseDestination>{6}</TermsOfUseDestination>
					<CdImagePath>{7}</CdImagePath>
					<CdSize>{8}</CdSize>
					<MinFileSize>{9}</MinFileSize>
				</AutoConfigure></MasterInstaller>", cppPath, productsPath, cdPath, helpSource,
															  helpDest, TOUSource, TOUDest, cdImagePath, cdSize, minFileSize);
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StringReader(basicModelXmlString))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				Assert.That(model.AutoConfigure.CppPath, Is.StringEnding(cppPath));
				Assert.That(model.AutoConfigure.ProductsPath, Is.StringEnding(productsPath));
				Assert.That(model.AutoConfigure.CdPath, Is.StringEnding(cdPath));
				Assert.That(model.AutoConfigure.HelpSource, Is.StringEnding(helpSource));
				Assert.That(model.AutoConfigure.HelpDest, Is.StringEnding(helpDest));
				Assert.That(model.AutoConfigure.TermsSource, Is.StringEnding(TOUSource));
				Assert.That(model.AutoConfigure.TermsDest, Is.StringEnding(TOUDest));
				Assert.That(model.AutoConfigure.ImagePath, Is.StringEnding(cdImagePath));
				Assert.That(model.AutoConfigure.CdSize, Is.EqualTo(int.Parse(cdSize)));
				Assert.That(model.AutoConfigure.MinFileSize, Is.EqualTo(int.Parse(minFileSize)));
			}
		}
	}
}
