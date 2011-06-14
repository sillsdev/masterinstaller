using System;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using Microsoft.Win32;
using IWshRuntimeLibrary;

namespace InitUtils
{
	public partial class InitUtils : Form
	{
		private const string MasterInstaller = "MASTER_INSTALLER";
		private const string PackageProducts = "PACKAGE_PRODUCTS";
		private const string CdImages = "CD_IMAGES";
		private const string CertificateDrive = "DIGITAL_CERT_DRIVE";

		public InitUtils()
		{
			InitializeComponent();

			var exePath = System.Reflection.Assembly.GetExecutingAssembly().Location;
			if (exePath == null)
				throw new Exception("So sorry, don't know where we are!");
			var exeFolder = Path.GetDirectoryName(exePath);

			MasterInstallerFolderBox.Text = Environment.GetEnvironmentVariable(MasterInstaller, EnvironmentVariableTarget.Machine);
			if (MasterInstallerFolderBox.Text != null)
				if (MasterInstallerFolderBox.Text.Length == 0)
					MasterInstallerFolderBox.Text = Path.GetDirectoryName(exeFolder);
			ProductsFolderBox.Text = Environment.GetEnvironmentVariable(PackageProducts, EnvironmentVariableTarget.Machine);
			CdImagesFolderBox.Text = Environment.GetEnvironmentVariable(CdImages, EnvironmentVariableTarget.Machine);

			var allDrives = DriveInfo.GetDrives();
			foreach (var d in allDrives.Where(d => d.DriveType == DriveType.CDRom))
				CdDrivesComboBox.Items.Add(d.RootDirectory.FullName);
			CdDrivesComboBox.SelectedItem = Environment.GetEnvironmentVariable(CertificateDrive, EnvironmentVariableTarget.Machine);
			if (CdDrivesComboBox.SelectedItem == null)
				CdDrivesComboBox.SelectedIndex = 0;
		}

		private void OnClickOk(object sender, EventArgs e)
		{
			Environment.SetEnvironmentVariable(MasterInstaller, MasterInstallerFolderBox.Text,
				EnvironmentVariableTarget.Machine);
			Environment.SetEnvironmentVariable(PackageProducts, ProductsFolderBox.Text,
				EnvironmentVariableTarget.Machine);
			Environment.SetEnvironmentVariable(CdImages, CdImagesFolderBox.Text,
				EnvironmentVariableTarget.Machine);
			Environment.SetEnvironmentVariable(CertificateDrive, CdDrivesComboBox.SelectedItem.ToString(),
				EnvironmentVariableTarget.Machine);

			var utilsFolder = Path.Combine(MasterInstallerFolderBox.Text, "Utils");

			// Set registry values for the CdImage utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\CD_Image", null, "Make CD image with this label");
			var b = new Byte[] { 1, 0, 0, 0 };
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\CD_Image", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\CD_Image\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CdImage.js") + "\" \"%1\"");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\DVD_Image", null, "Make DVD image (using UDF) with this label");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\DVD_Image", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\DVD_Image\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CdImage.js") + "\" \"%1\" -UDF");

			// Set registry values for the CompileXmlMasterInstaller utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\CompileMasterInstaller", null, "Compile Master Installer from this XML file");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\CompileMasterInstaller\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CompileXmlMasterInstaller.js") + "\" \"%1\"");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\CompileMasterInstallerEx", null, "Compile Master Installer with Easter Eggs from this XML file");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\CompileMasterInstallerEx\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CompileXmlMasterInstaller.js") + "\" \"%1\" -E");

			// Set registry values for the CompressFolder utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\7-zip", null, "Compress contents to maximum extent");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\7-zip", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\7-zip\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CompressFolder.js") + "\" \"%1\"");

			// Set registry values for the CreateSfxSetup utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilFwSfxSetup", null, "Create SIL FieldWorks Software Package from this folder's contents");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilFwSfxSetup", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilFwSfxSetup\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CreateSfxSetup.js") + "\" \"%1\" -InstallFw");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilSfxSetup", null, "Create Standard SIL Software Package from this folder's contents");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilSfxSetup", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilSfxSetup\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CreateSfxSetup.js") + "\" \"%1\" -InstallThenDelete");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilCustomSfxSetup", null, "Create Custom Software Package from this folder's contents");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilCustomSfxSetup", "EditFlags", b, RegistryValueKind.Binary);
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Folder\shell\SilCustomSfxSetup\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "CreateSfxSetup.js") + "\" \"%1\" -Custom");

			// Set registry values for the RebuildWebDownload utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\RebuildWebDownload", null, "(Re)build web download package from this XML file");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\RebuildWebDownload\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "RebuildWebDownload.js") + "\" \"%1\"");

			// Set registry values for the WriteDownloadReport utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\WriteDownloadReport", null, "Compose downloads report from this XML file");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\xmlfile\shell\WriteDownloadReport\command", null, "wscript.exe \"" + Path.Combine(utilsFolder, "WriteDownloadReport.js") + "\" \"%1\"");

			// Set registry values for the SignMaster utility:
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Msi.Package\shell\Sign", null, "Sign with digital certificate");
			Registry.SetValue(@"HKEY_CLASSES_ROOT\Msi.Package\shell\Sign\command", null, "\"" + Path.Combine(utilsFolder, "SignMaster.exe") + "\" -d \"SIL Software Installer\" \"%1\"");

			// Add desktop shortcuts for the SignMaster utility:
			var wshShell = new WshShell();

			var masterInstallerShortcut = (IWshShortcut)wshShell.CreateShortcut(Path.Combine(
				Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
				"Drop (Master) Installer here to sign it.lnk"));

			masterInstallerShortcut.TargetPath = Path.Combine(utilsFolder, "SignMaster.exe");
			masterInstallerShortcut.Arguments = "-d \"SIL Software Installer\"";
			masterInstallerShortcut.Description =
				"Launches the SignMaster utility to sign the dropped file(s) as SIL Software Installer(s).";
			masterInstallerShortcut.IconLocation = "SHELL32.dll,216";
			masterInstallerShortcut.Save();

			var downloadPackageShortcut = (IWshShortcut)wshShell.CreateShortcut(Path.Combine(
				Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
				"Drop Software Package here to sign it.lnk"));

			downloadPackageShortcut.TargetPath = Path.Combine(utilsFolder, "SignMaster.exe");
			downloadPackageShortcut.Arguments = "-d \"SIL Software Package\"";
			downloadPackageShortcut.Description =
				"Launches the SignMaster utility to sign the dropped file(s) as SIL Software Package(s).";
			downloadPackageShortcut.IconLocation = "SHELL32.dll,216";
			downloadPackageShortcut.Save();

			MessageBox.Show("Done. You can now use the Master Installer utilities.");
			Close();
		}

		private void OnClickCancel(object sender, EventArgs e)
		{
			Close();
		}

		private void OnClickMasterInstallerBrowse(object sender, EventArgs e)
		{
			var d = new FolderBrowserDialog();
			d.Description = "Select the folder that contains your local copy of the Master Installer source files:";
			d.SelectedPath = MasterInstallerFolderBox.Text;
			d.ShowNewFolderButton = false;
			d.ShowDialog();
			MasterInstallerFolderBox.Text = d.SelectedPath;
		}

		private void OnClickProductsBrowse(object sender, EventArgs e)
		{
			var d = new FolderBrowserDialog();
			d.Description = "Select the folder that contains your local copy of the Products library:";
			d.SelectedPath = ProductsFolderBox.Text;
			d.ShowNewFolderButton = false;
			d.ShowDialog();
			ProductsFolderBox.Text = d.SelectedPath;
		}

		private void OnClickCDsBrowse(object sender, EventArgs e)
		{
			var d = new FolderBrowserDialog();
			d.Description = "Select the folder that does (or will) contain your local copy of the CD images archive:";
			d.SelectedPath = CdImagesFolderBox.Text;
			d.ShowNewFolderButton = true;
			d.ShowDialog();
			CdImagesFolderBox.Text = d.SelectedPath;
		}
	}
}
