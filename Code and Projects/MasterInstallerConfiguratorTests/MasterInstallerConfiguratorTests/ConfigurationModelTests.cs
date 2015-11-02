﻿using System.IO;
using System.Linq;
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
				Assert.That((int)model.General.ListSpacingAdjust, Is.EqualTo(listSpaceAdj), "Did not read ListSpacingAdjust");
				Assert.That((int)model.General.InfoButtonAdjust, Is.EqualTo(infoButtonAdj), "Did not read InfoButtonAdjust");
				Assert.That(model.General.StartFromAnyCD, Is.True, "Did not read StartFromAnyCD");
				Assert.That(model.General.KeyPromptNeedsKeys, Is.True, "Did not read KeyPromptNeedsKeys");
				Assert.That(model.General.KeyTitle, Is.StringEnding(keyTitle), "Did not read KeyTitle");
				Assert.That(model.General.HelpFile, Is.EqualTo(extHelpFile), "Did not read HelpFile");
				Assert.That(model.General.HelpButtonText, Is.EqualTo(helpBtnTxt), "Did not read HelpFile");
				Assert.That(model.General.TermsOfUseFile, Is.EqualTo(TOUFile), "Did not read TermsOfUseFile");
				Assert.That(model.General.TermsOfUseButtonText, Is.EqualTo(TOUBtnTxt), "Did not read TermsOfUseButtonText");
			}
		}

		[Test]
		public void CanReadProductSection()
		{
			var title = @"Title";
			var tag = @"Subtitle";
			var mustHaveWin2k = true;
			var admin = true;
			var criticalFile = "Utilities\\DotNet\\NDP1.1sp1-KB867460-X86.exe";
			var install = "$Magic$";
			var testPresence = "TestPresenceMagicWords";
			var statusWindow = "Status";
			var productCode = @"SIL Software Product Key";
			var help = @"help.chm";
			var tagAttribute = @"needstag";
			var tagAttribute2 = @"needstag2";
			var TOUBtnTxt = @"Can I use this?";
			var version = "14.2";
			var CDNumber = 0;
			var downloadUrl = "http://test.com/";
			var flushReboot = true;
			var testVersion = "2.2.2";
			var failMessage = "fail Message";
			var basicModelXmlString =
				string.Format(@"<MasterInstaller><Products><Product List='true'>
					<AutoConfigure><Title>Test</Title></AutoConfigure>
					<Title>{0}</Title>
					<Tag>{1}</Tag>
					<MustHaveWin2kOrBetter>{2}</MustHaveWin2kOrBetter>
					<MustBeAdmin>{3}</MustBeAdmin>
					<CriticalFile>{4}</CriticalFile>
					<Install>{5}</Install>
					<TestPresence Version='1.1'>{6}</TestPresence>
					<StatusWindow>{7}</StatusWindow>
					<ProductCode>{8}</ProductCode>
					<Help>{9}</Help>
					<Prerequisite Tag='{10}' Version='{15}'/>
					<Prerequisite Tag='{11}' Version='{15}'/>
					<CD>{12}</CD>
					<DownloadURL>{13}</DownloadURL>
					<FlushReboot>{14}</FlushReboot>
					<Requires Tag='{10}' MinVersion='{15}' FailMsg='{16}'/>
				</Product></Products></MasterInstaller>", title, tag, mustHaveWin2k.ToString().ToLower(), admin.ToString().ToLower(), criticalFile, install,
													  testPresence, statusWindow, productCode, help, tagAttribute, tagAttribute2, CDNumber, downloadUrl, flushReboot.ToString().ToLower(), testVersion, failMessage);
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StringReader(basicModelXmlString))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				var testProduct = model.Products[0];
				Assert.That(testProduct.Title, Is.StringEnding(title), "Did not read Title");
				Assert.That(testProduct.Tag, Is.StringEnding(tag), "Did not read Tag");
				Assert.That(testProduct.MustHaveWin2kOrBetter, Is.True, "Did not read MustHaveWin2kOrBetter");
				Assert.That(testProduct.MustBeAdmin, Is.True, "Did not read MustBeAdmin");
				Assert.That(testProduct.CriticalFile, Is.StringEnding(criticalFile), "Did not read CriticalFile");
				Assert.That(testProduct.Install, Is.StringEnding(install), "Did not read Install");
				Assert.That(testProduct.TestPresence.TestValue, Is.StringEnding(testPresence), "Did not read TestPresence");
				Assert.That(testProduct.StatusWindow, Is.StringEnding(statusWindow), "Did not read StatusWindow");
				Assert.That(testProduct.ProductCode, Is.StringEnding(productCode), "Did not read ProductCode");
				Assert.That(testProduct.Help, Is.EqualTo(help), "Did not read HelpFile");
				Assert.That(testProduct.Prerequisite.Count, Is.EqualTo(2), "Not enough prerequisite elements");
				Assert.That(testProduct.Prerequisite[0].Tag, Is.StringEnding(tagAttribute), "Did not read first prereq Tag");
				Assert.That(testProduct.Prerequisite[0].Version, Is.StringEnding(testVersion), "Did not read first prereq version");
				Assert.That(testProduct.Prerequisite[1].Tag, Is.StringEnding(tagAttribute2), "Did not read second prereq Tag");
				Assert.That(testProduct.CDNumber, Is.EqualTo(CDNumber), "Did not read CD Number");
				Assert.That(testProduct.DownloadURL, Is.StringEnding(downloadUrl), "Did not read DownloadURL");
				Assert.That(testProduct.FlushReboot, Is.True, "Did not read flushreboot");
				Assert.That(testProduct.Requires.Tag, Is.StringEnding(tagAttribute), "Did not read Requires Tag attribute");
				Assert.That(testProduct.Requires.MinVersion, Is.StringEnding(testVersion), "Did not read Requires testVersion");
				Assert.That(testProduct.Requires.FailMessage, Is.StringEnding(failMessage), "Did not read Requires failMessage");
			}
		}

		[Test]
		public void CanHandleEmptyAdjustElements()
		{
			var basicModelXmlString =
				@"<MasterInstaller><General>
					<Title>Title</Title>
					<ListSpacingAdjust></ListSpacingAdjust>
					<InfoButtonAdjust></InfoButtonAdjust>
				</General></MasterInstaller>";
			var serializer = new XmlSerializer(typeof(ConfigurationModel));
			using (var textReader = new StringReader(basicModelXmlString))
			{
				var model = (ConfigurationModel)serializer.Deserialize(textReader);
				// Make sure that the implicit cast to int is working when nothing is present in the element
				Assert.True(model.General.ListSpacingAdjust == 0, "Did not read default ListSpacingAdjust");
				Assert.True(model.General.ListSpacingAdjust == 0, "Did not read default InfoButtonAdjust");
			}
		}

		[Test]
		[Category("ByHand")]
		public void CanReadExistingConfigurations()
		{
			var installerDefinitions = Directory.EnumerateDirectories("../../../../../Installer Definitions");
			foreach (var dir in installerDefinitions)
			{
				var xmlFiles = Directory.EnumerateFiles(dir, "*.xml");

				var serializer = new XmlSerializer(typeof(ConfigurationModel));
				using (var textReader = new FileStream(xmlFiles.First(), FileMode.Open))
				{
					var result = serializer.Deserialize(textReader);
				}
			}
		}
	}
}
