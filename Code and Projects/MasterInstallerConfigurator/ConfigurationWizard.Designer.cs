﻿namespace MasterInstallerConfigurator
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
			this._flavorConfigTab = new System.Windows.Forms.TabPage();
			this._webDownloadTab = new System.Windows.Forms.TabPage();
			this.ConfigTabControl.SuspendLayout();
			this.SuspendLayout();
			// 
			// ConfigTabControl
			// 
			this.ConfigTabControl.Controls.Add(this._flavorSetupPage);
			this.ConfigTabControl.Controls.Add(this._flavorConfigTab);
			this.ConfigTabControl.Controls.Add(this._webDownloadTab);
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
			this._flavorSetupPage.Location = new System.Drawing.Point(4, 22);
			this._flavorSetupPage.Name = "_flavorSetupPage";
			this._flavorSetupPage.Padding = new System.Windows.Forms.Padding(3);
			this._flavorSetupPage.Size = new System.Drawing.Size(1078, 555);
			this._flavorSetupPage.TabIndex = 0;
			this._flavorSetupPage.Text = "Flavor Setup";
			this._flavorSetupPage.UseVisualStyleBackColor = true;
			// 
			// _flavorConfigTab
			// 
			this._flavorConfigTab.Location = new System.Drawing.Point(4, 22);
			this._flavorConfigTab.Name = "_flavorConfigTab";
			this._flavorConfigTab.Padding = new System.Windows.Forms.Padding(3);
			this._flavorConfigTab.Size = new System.Drawing.Size(1078, 555);
			this._flavorConfigTab.TabIndex = 1;
			this._flavorConfigTab.Text = "Flavor Configuration";
			this._flavorConfigTab.UseVisualStyleBackColor = true;
			// 
			// _webDownloadTab
			// 
			this._webDownloadTab.Location = new System.Drawing.Point(4, 22);
			this._webDownloadTab.Name = "_webDownloadTab";
			this._webDownloadTab.Padding = new System.Windows.Forms.Padding(3);
			this._webDownloadTab.Size = new System.Drawing.Size(1078, 555);
			this._webDownloadTab.TabIndex = 2;
			this._webDownloadTab.Text = "Build Web Downloads";
			this._webDownloadTab.UseVisualStyleBackColor = true;
			// 
			// ConfigurationWizard
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1086, 581);
			this.Controls.Add(this.ConfigTabControl);
			this.Name = "ConfigurationWizard";
			this.Text = "Form1";
			this.ConfigTabControl.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.TabControl ConfigTabControl;
		private System.Windows.Forms.TabPage _flavorSetupPage;
		private System.Windows.Forms.TabPage _flavorConfigTab;
		private System.Windows.Forms.TabPage _webDownloadTab;
	}
}

