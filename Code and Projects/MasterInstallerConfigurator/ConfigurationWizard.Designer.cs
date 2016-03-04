// Copyright (c) 2015 SIL International
// This software is licensed under the MIT License (http://opensource.org/licenses/MIT)
namespace MasterInstallerConfigurator
{
	partial class ConfigurationWizard
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ConfigTabControl = new System.Windows.Forms.TabControl();
			this._flavorSetupPage = new System.Windows.Forms.TabPage();
			this._deleteFlavor = new System.Windows.Forms.Button();
			this._addFlavorBtn = new System.Windows.Forms.Button();
			this._flavorSetupLabel = new System.Windows.Forms.Label();
			this._flavorPanel = new System.Windows.Forms.TableLayoutPanel();
			this._flavorConfigTab = new System.Windows.Forms.TabPage();
			this._flavorConfigLabel = new System.Windows.Forms.Label();
			this._flavorOptionTable = new System.Windows.Forms.TableLayoutPanel();
			this._webDownloadTab = new System.Windows.Forms.TabPage();
			this._goButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this._packageStyle = new System.Windows.Forms.ComboBox();
			this._certificatePassPhrase = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this._saveSettings = new System.Windows.Forms.CheckBox();
			this._buildDLPackages = new System.Windows.Forms.CheckBox();
			this._gatherFiles = new System.Windows.Forms.CheckBox();
			this._writeDownloadsXml = new System.Windows.Forms.CheckBox();
			this._writeInstallerXml = new System.Windows.Forms.CheckBox();
			this._compile = new System.Windows.Forms.CheckBox();
			this._folderLabel = new System.Windows.Forms.Label();
			this._outputFolder = new System.Windows.Forms.TextBox();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this._logBox = new System.Windows.Forms.RichTextBox();
			this._system = new System.Windows.Forms.TabPage();
			this._vsIncludePath = new System.Windows.Forms.TextBox();
			this._browseButton = new System.Windows.Forms.Button();
			this._binBrowseBtn = new System.Windows.Forms.Button();
			this._vsBinPath = new System.Windows.Forms.TextBox();
			this.ConfigTabControl.SuspendLayout();
			this._flavorSetupPage.SuspendLayout();
			this._flavorConfigTab.SuspendLayout();
			this._webDownloadTab.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this._system.SuspendLayout();
			this.SuspendLayout();
			// 
			// ConfigTabControl
			// 
			this.ConfigTabControl.Controls.Add(this._system);
			this.ConfigTabControl.Controls.Add(this._flavorSetupPage);
			this.ConfigTabControl.Controls.Add(this._flavorConfigTab);
			this.ConfigTabControl.Controls.Add(this._webDownloadTab);
			this.ConfigTabControl.Controls.Add(this.tabPage1);
			this.ConfigTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ConfigTabControl.Location = new System.Drawing.Point(0, 0);
			this.ConfigTabControl.Multiline = true;
			this.ConfigTabControl.Name = "ConfigTabControl";
			this.ConfigTabControl.SelectedIndex = 0;
			this.ConfigTabControl.Size = new System.Drawing.Size(1086, 581);
			this.ConfigTabControl.TabIndex = 0;
			// 
			// _flavorSetupPage
			// 
			this._flavorSetupPage.Controls.Add(this._deleteFlavor);
			this._flavorSetupPage.Controls.Add(this._addFlavorBtn);
			this._flavorSetupPage.Controls.Add(this._flavorSetupLabel);
			this._flavorSetupPage.Controls.Add(this._flavorPanel);
			this._flavorSetupPage.Location = new System.Drawing.Point(4, 22);
			this._flavorSetupPage.Name = "_flavorSetupPage";
			this._flavorSetupPage.Padding = new System.Windows.Forms.Padding(3);
			this._flavorSetupPage.Size = new System.Drawing.Size(1078, 555);
			this._flavorSetupPage.TabIndex = 0;
			this._flavorSetupPage.Text = "Flavor Setup";
			this._flavorSetupPage.UseVisualStyleBackColor = true;
			// 
			// _deleteFlavor
			// 
			this._deleteFlavor.Location = new System.Drawing.Point(117, 438);
			this._deleteFlavor.Name = "_deleteFlavor";
			this._deleteFlavor.Size = new System.Drawing.Size(75, 23);
			this._deleteFlavor.TabIndex = 3;
			this._deleteFlavor.Text = "Delete Last Flavor";
			this._deleteFlavor.UseVisualStyleBackColor = true;
			this._deleteFlavor.Click += new System.EventHandler(this._deleteFlavor_Click);
			// 
			// _addFlavorBtn
			// 
			this._addFlavorBtn.Location = new System.Drawing.Point(27, 438);
			this._addFlavorBtn.Name = "_addFlavorBtn";
			this._addFlavorBtn.Size = new System.Drawing.Size(75, 23);
			this._addFlavorBtn.TabIndex = 2;
			this._addFlavorBtn.Text = "Add Flavor";
			this._addFlavorBtn.UseVisualStyleBackColor = true;
			this._addFlavorBtn.Click += new System.EventHandler(this._addFlavorBtn_Click);
			// 
			// _flavorSetupLabel
			// 
			this._flavorSetupLabel.AutoSize = true;
			this._flavorSetupLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this._flavorSetupLabel.Location = new System.Drawing.Point(23, 16);
			this._flavorSetupLabel.Name = "_flavorSetupLabel";
			this._flavorSetupLabel.Size = new System.Drawing.Size(309, 24);
			this._flavorSetupLabel.TabIndex = 1;
			this._flavorSetupLabel.Text = "Add, Modify, or Remove Flavors";
			// 
			// _flavorPanel
			// 
			this._flavorPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this._flavorPanel.ColumnCount = 2;
			this._flavorPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 200F));
			this._flavorPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this._flavorPanel.Location = new System.Drawing.Point(27, 53);
			this._flavorPanel.Name = "_flavorPanel";
			this._flavorPanel.RowCount = 1;
			this._flavorPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 482F));
			this._flavorPanel.Size = new System.Drawing.Size(662, 358);
			this._flavorPanel.TabIndex = 0;
			// 
			// _flavorConfigTab
			// 
			this._flavorConfigTab.Controls.Add(this._flavorConfigLabel);
			this._flavorConfigTab.Controls.Add(this._flavorOptionTable);
			this._flavorConfigTab.Location = new System.Drawing.Point(4, 22);
			this._flavorConfigTab.Name = "_flavorConfigTab";
			this._flavorConfigTab.Padding = new System.Windows.Forms.Padding(3);
			this._flavorConfigTab.Size = new System.Drawing.Size(1078, 555);
			this._flavorConfigTab.TabIndex = 1;
			this._flavorConfigTab.Text = "Flavor Configuration";
			this._flavorConfigTab.UseVisualStyleBackColor = true;
			// 
			// _flavorConfigLabel
			// 
			this._flavorConfigLabel.AutoSize = true;
			this._flavorConfigLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this._flavorConfigLabel.Location = new System.Drawing.Point(27, 17);
			this._flavorConfigLabel.Name = "_flavorConfigLabel";
			this._flavorConfigLabel.Size = new System.Drawing.Size(312, 24);
			this._flavorConfigLabel.TabIndex = 1;
			this._flavorConfigLabel.Text = "Select Products For Each Flavor";
			// 
			// _flavorOptionTable
			// 
			this._flavorOptionTable.ColumnCount = 5;
			this._flavorOptionTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 400F));
			this._flavorOptionTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 100F));
			this._flavorOptionTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 100F));
			this._flavorOptionTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 100F));
			this._flavorOptionTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 243F));
			this._flavorOptionTable.Location = new System.Drawing.Point(31, 64);
			this._flavorOptionTable.Name = "_flavorOptionTable";
			this._flavorOptionTable.RowCount = 1;
			this._flavorOptionTable.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this._flavorOptionTable.Size = new System.Drawing.Size(943, 405);
			this._flavorOptionTable.TabIndex = 0;
			// 
			// _webDownloadTab
			// 
			this._webDownloadTab.Controls.Add(this._goButton);
			this._webDownloadTab.Controls.Add(this.label1);
			this._webDownloadTab.Controls.Add(this._packageStyle);
			this._webDownloadTab.Controls.Add(this._certificatePassPhrase);
			this._webDownloadTab.Controls.Add(this.label2);
			this._webDownloadTab.Controls.Add(this._saveSettings);
			this._webDownloadTab.Controls.Add(this._buildDLPackages);
			this._webDownloadTab.Controls.Add(this._gatherFiles);
			this._webDownloadTab.Controls.Add(this._writeDownloadsXml);
			this._webDownloadTab.Controls.Add(this._writeInstallerXml);
			this._webDownloadTab.Controls.Add(this._compile);
			this._webDownloadTab.Controls.Add(this._folderLabel);
			this._webDownloadTab.Controls.Add(this._outputFolder);
			this._webDownloadTab.Location = new System.Drawing.Point(4, 22);
			this._webDownloadTab.Name = "_webDownloadTab";
			this._webDownloadTab.Padding = new System.Windows.Forms.Padding(3);
			this._webDownloadTab.Size = new System.Drawing.Size(1078, 555);
			this._webDownloadTab.TabIndex = 2;
			this._webDownloadTab.Text = "Build Web Downloads";
			this._webDownloadTab.UseVisualStyleBackColor = true;
			// 
			// _goButton
			// 
			this._goButton.Location = new System.Drawing.Point(32, 361);
			this._goButton.Name = "_goButton";
			this._goButton.Size = new System.Drawing.Size(75, 23);
			this._goButton.TabIndex = 17;
			this._goButton.Text = "Run";
			this._goButton.UseVisualStyleBackColor = true;
			this._goButton.Click += new System.EventHandler(this.goButton_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.Location = new System.Drawing.Point(25, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(196, 24);
			this.label1.TabIndex = 16;
			this.label1.Text = "Run Selected Tasks";
			// 
			// _packageStyle
			// 
			this._packageStyle.FormattingEnabled = true;
			this._packageStyle.Items.AddRange(new object[] {
            "FLEx",
            "Standard"});
			this._packageStyle.Location = new System.Drawing.Point(258, 272);
			this._packageStyle.Name = "_packageStyle";
			this._packageStyle.Size = new System.Drawing.Size(121, 21);
			this._packageStyle.TabIndex = 15;
			// 
			// _certificatePassPhrase
			// 
			this._certificatePassPhrase.Location = new System.Drawing.Point(160, 192);
			this._certificatePassPhrase.Name = "_certificatePassPhrase";
			this._certificatePassPhrase.PasswordChar = '*';
			this._certificatePassPhrase.Size = new System.Drawing.Size(100, 20);
			this._certificatePassPhrase.TabIndex = 14;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(29, 192);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(124, 13);
			this.label2.TabIndex = 13;
			this.label2.Text = "Password For Certificate:";
			// 
			// _saveSettings
			// 
			this._saveSettings.AutoSize = true;
			this._saveSettings.Location = new System.Drawing.Point(29, 301);
			this._saveSettings.Name = "_saveSettings";
			this._saveSettings.Size = new System.Drawing.Size(151, 17);
			this._saveSettings.TabIndex = 12;
			this._saveSettings.Text = "Remember These Settings";
			this._saveSettings.UseVisualStyleBackColor = true;
			// 
			// _buildDLPackages
			// 
			this._buildDLPackages.AutoSize = true;
			this._buildDLPackages.Location = new System.Drawing.Point(29, 272);
			this._buildDLPackages.Name = "_buildDLPackages";
			this._buildDLPackages.Size = new System.Drawing.Size(222, 17);
			this._buildDLPackages.TabIndex = 11;
			this._buildDLPackages.Text = "Build Self Extracting Download Packages";
			this._buildDLPackages.UseVisualStyleBackColor = true;
			// 
			// _gatherFiles
			// 
			this._gatherFiles.AutoSize = true;
			this._gatherFiles.Location = new System.Drawing.Point(29, 238);
			this._gatherFiles.Name = "_gatherFiles";
			this._gatherFiles.Size = new System.Drawing.Size(82, 17);
			this._gatherFiles.TabIndex = 9;
			this._gatherFiles.Text = "Gather Files";
			this._gatherFiles.UseVisualStyleBackColor = true;
			// 
			// _writeDownloadsXml
			// 
			this._writeDownloadsXml.AutoSize = true;
			this._writeDownloadsXml.Location = new System.Drawing.Point(29, 168);
			this._writeDownloadsXml.Name = "_writeDownloadsXml";
			this._writeDownloadsXml.Size = new System.Drawing.Size(132, 17);
			this._writeDownloadsXml.TabIndex = 7;
			this._writeDownloadsXml.Text = "Write Downloads XML";
			this._writeDownloadsXml.UseVisualStyleBackColor = true;
			// 
			// _writeInstallerXml
			// 
			this._writeInstallerXml.AutoSize = true;
			this._writeInstallerXml.Location = new System.Drawing.Point(29, 136);
			this._writeInstallerXml.Name = "_writeInstallerXml";
			this._writeInstallerXml.Size = new System.Drawing.Size(150, 17);
			this._writeInstallerXml.TabIndex = 5;
			this._writeInstallerXml.Text = "Write Master Installer XML";
			this._writeInstallerXml.UseVisualStyleBackColor = true;
			// 
			// _compile
			// 
			this._compile.AutoSize = true;
			this._compile.Location = new System.Drawing.Point(29, 104);
			this._compile.Name = "_compile";
			this._compile.Size = new System.Drawing.Size(137, 17);
			this._compile.TabIndex = 3;
			this._compile.Text = "Compile Master Installer";
			this._compile.UseVisualStyleBackColor = true;
			// 
			// _folderLabel
			// 
			this._folderLabel.AutoSize = true;
			this._folderLabel.Location = new System.Drawing.Point(26, 63);
			this._folderLabel.Name = "_folderLabel";
			this._folderLabel.Size = new System.Drawing.Size(74, 13);
			this._folderLabel.TabIndex = 1;
			this._folderLabel.Text = "Output Folder:";
			// 
			// _outputFolder
			// 
			this._outputFolder.Location = new System.Drawing.Point(118, 63);
			this._outputFolder.Name = "_outputFolder";
			this._outputFolder.Size = new System.Drawing.Size(375, 20);
			this._outputFolder.TabIndex = 0;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this._logBox);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(1078, 555);
			this.tabPage1.TabIndex = 3;
			this.tabPage1.Text = "Results";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// _logBox
			// 
			this._logBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this._logBox.Location = new System.Drawing.Point(3, 3);
			this._logBox.Name = "_logBox";
			this._logBox.Size = new System.Drawing.Size(1072, 549);
			this._logBox.TabIndex = 0;
			this._logBox.Text = "";
			// 
			// _system
			// 
			this._system.Controls.Add(this._binBrowseBtn);
			this._system.Controls.Add(this._vsBinPath);
			this._system.Controls.Add(this._browseButton);
			this._system.Controls.Add(this._vsIncludePath);
			this._system.Location = new System.Drawing.Point(4, 22);
			this._system.Name = "_system";
			this._system.Padding = new System.Windows.Forms.Padding(3);
			this._system.Size = new System.Drawing.Size(1078, 555);
			this._system.TabIndex = 4;
			this._system.Text = "System Settings";
			this._system.UseVisualStyleBackColor = true;
			// 
			// _vsIncludePath
			// 
			this._vsIncludePath.Location = new System.Drawing.Point(25, 38);
			this._vsIncludePath.Name = "_vsIncludePath";
			this._vsIncludePath.Size = new System.Drawing.Size(272, 20);
			this._vsIncludePath.TabIndex = 0;
			// 
			// _browseButton
			// 
			this._browseButton.Location = new System.Drawing.Point(320, 38);
			this._browseButton.Name = "_browseButton";
			this._browseButton.Size = new System.Drawing.Size(75, 23);
			this._browseButton.TabIndex = 1;
			this._browseButton.Text = "Browse";
			this._browseButton.UseVisualStyleBackColor = true;
			this._browseButton.Click += new System.EventHandler(this._browseButton_Click);
			// 
			// _binBrowseBtn
			// 
			this._binBrowseBtn.Location = new System.Drawing.Point(320, 91);
			this._binBrowseBtn.Name = "_binBrowseBtn";
			this._binBrowseBtn.Size = new System.Drawing.Size(75, 23);
			this._binBrowseBtn.TabIndex = 3;
			this._binBrowseBtn.Text = "Browse";
			this._binBrowseBtn.UseVisualStyleBackColor = true;
			this._binBrowseBtn.Click += new System.EventHandler(this._binBrowseBtn_Click);
			// 
			// _vsBinPath
			// 
			this._vsBinPath.Location = new System.Drawing.Point(25, 91);
			this._vsBinPath.Name = "_vsBinPath";
			this._vsBinPath.Size = new System.Drawing.Size(272, 20);
			this._vsBinPath.TabIndex = 2;
			// 
			// ConfigurationWizard
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1086, 581);
			this.Controls.Add(this.ConfigTabControl);
			this.Name = "ConfigurationWizard";
			this.ShowIcon = false;
			this.Text = "Master Installer Configurator";
			this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.ConfigurationWizard_FormClosed);
			this.ConfigTabControl.ResumeLayout(false);
			this._flavorSetupPage.ResumeLayout(false);
			this._flavorSetupPage.PerformLayout();
			this._flavorConfigTab.ResumeLayout(false);
			this._flavorConfigTab.PerformLayout();
			this._webDownloadTab.ResumeLayout(false);
			this._webDownloadTab.PerformLayout();
			this.tabPage1.ResumeLayout(false);
			this._system.ResumeLayout(false);
			this._system.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TabControl ConfigTabControl;
		private System.Windows.Forms.TabPage _flavorSetupPage;
		private System.Windows.Forms.TabPage _flavorConfigTab;
		private System.Windows.Forms.TabPage _webDownloadTab;
		private System.Windows.Forms.Label _flavorSetupLabel;
		private System.Windows.Forms.TableLayoutPanel _flavorPanel;
		private System.Windows.Forms.TableLayoutPanel _flavorOptionTable;
		private System.Windows.Forms.Label _flavorConfigLabel;
		private System.Windows.Forms.TextBox _certificatePassPhrase;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox _saveSettings;
		private System.Windows.Forms.CheckBox _buildDLPackages;
		private System.Windows.Forms.CheckBox _gatherFiles;
		private System.Windows.Forms.CheckBox _writeDownloadsXml;
		private System.Windows.Forms.CheckBox _writeInstallerXml;
		private System.Windows.Forms.CheckBox _compile;
		private System.Windows.Forms.Label _folderLabel;
		private System.Windows.Forms.TextBox _outputFolder;
		private System.Windows.Forms.ComboBox _packageStyle;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button _goButton;
		private System.Windows.Forms.Button _addFlavorBtn;
		private System.Windows.Forms.Button _deleteFlavor;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.RichTextBox _logBox;
		private System.Windows.Forms.TabPage _system;
		private System.Windows.Forms.Button _browseButton;
		private System.Windows.Forms.TextBox _vsIncludePath;
		private System.Windows.Forms.Button _binBrowseBtn;
		private System.Windows.Forms.TextBox _vsBinPath;
	}
}

