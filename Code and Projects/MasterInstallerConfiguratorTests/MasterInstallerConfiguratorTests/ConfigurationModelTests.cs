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
			var cdSize = 700;
			var minFileSize = 2048;
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
				Assert.That(model.AutoConfigure.CdSize, Is.EqualTo(cdSize));
				Assert.That(model.AutoConfigure.MinFileSize, Is.EqualTo(minFileSize));
			}
		}

		[Test]
		public void CanReadGeneralSection()
		{
			var title = @"Title";
			var listSubtitle = @"Subtitle";
			var listBgPath = @"D:\CDs\bg.png";
			var listEvenOne = true;
			var listSpaceAdj = 3;
			var infoButtonAdj = 4;
			var startFromAny = true;
			var keyPromptNeedsKeys = true;
			var keyTitle = @"SIL Software Product Key";
			var extHelpFile = @"help.chm";
			var helpBtnTxt = "Help me Rhonda";
			var TOUFile = @"D:\Terms.txt";
			var TOUBtnTxt = @"Can I use this?";
			var basicModelXmlString =
				string.Format(@"<MasterInstaller><General>
					<Title>{0}</Title>
					<ListSubtitle>{1}</ListSubtitle>
					<ListBackground DefaultFolder='Bitmaps' OffsetX='30' OffsetY='30' BlendLeft='30' BlendRight='30' BlendTop='30' BlendBottom='30'>{2}</ListBackground>
					<ListEvenOneProduct>{3}</ListEvenOneProduct>
					<ListSpacingAdjust>{4}</ListSpacingAdjust>
					<InfoButtonAdjust>{5}</InfoButtonAdjust>
					<StartFromAnyCD>{6}</StartFromAnyCD>
					<KeyPromptNeedsShiftCtrl>{7}</KeyPromptNeedsShiftCtrl>
					<GetKeyTitle>{8}</GetKeyTitle>
					<ExternalHelpFile>{9}</ExternalHelpFile>
					<HelpButtonText>{10}</HelpButtonText>
					<TermsOfUseFile>{11}</TermsOfUseFile>
					<TermsButtonText>{12}</TermsButtonText>
				</General></MasterInstaller>", title, listSubtitle, listBgPath, listEvenOne.ToString().ToLower(), listSpaceAdj, infoButtonAdj,
													  startFromAny.ToString().ToLower(), keyPromptNeedsKeys.ToString().ToLower(), keyTitle, extHelpFile, helpBtnTxt, TOUFile, TOUBtnTxt);
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StringReader(basicModelXmlString))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				Assert.That(model.General.Title, Is.StringEnding(title), "Did not read Title");
				Assert.That(model.General.ListSubtitle, Is.StringEnding(listSubtitle), "Did not read ListSubtitle");
				Assert.That(model.General.ListBackground.ImagePath, Is.StringEnding(listBgPath), "Did not read ListBackground");
				Assert.That(model.General.ListEvenOne, Is.True, "Did not read ListEvenOne");
				Assert.That(model.General.ListSpacingAdjust, Is.EqualTo(listSpaceAdj), "Did not read ListSpacingAdjust");
				Assert.That(model.General.InfoButtonAdjust, Is.EqualTo(infoButtonAdj), "Did not read InfoButtonAdjust");
				Assert.That(model.General.StartFromAnyCD, Is.True, "Did not read StartFromAnyCD");
				Assert.That(model.General.KeyPromptNeedsKeys, Is.True, "Did not read KeyPromptNeedsKeys");
				Assert.That(model.General.KeyTitle, Is.StringEnding(keyTitle), "Did not read KeyTitle");
				Assert.That(model.General.HelpFile, Is.EqualTo(extHelpFile), "Did not read HelpFile");
				Assert.That(model.General.HelpButtonText, Is.EqualTo(helpBtnTxt), "Did not read HelpFile");
				Assert.That(model.General.TermsOfUseFile, Is.EqualTo(TOUFile), "Did not read TermsOfUseFile");
				Assert.That(model.General.TermsOfUseButtonText, Is.EqualTo(TOUBtnTxt), "Did not read TermsOfUseButtonText");
			}
		}
	}
}
