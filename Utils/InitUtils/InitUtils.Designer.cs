namespace InitUtils
{
	partial class InitUtils
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(InitUtils));
			this.MasterInstallerFolderBox = new System.Windows.Forms.TextBox();
			this.CancelButton = new System.Windows.Forms.Button();
			this.OkButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.ProductsFolderBox = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.CdImagesFolderBox = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.CdDrivesComboBox = new System.Windows.Forms.ComboBox();
			this.MasterInstallerBrowseButton = new System.Windows.Forms.Button();
			this.ProductsBrowseButton = new System.Windows.Forms.Button();
			this.CdsBrowseButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// MasterInstallerFolderBox
			// 
			this.MasterInstallerFolderBox.Location = new System.Drawing.Point(10, 25);
			this.MasterInstallerFolderBox.Name = "MasterInstallerFolderBox";
			this.MasterInstallerFolderBox.Size = new System.Drawing.Size(485, 20);
			this.MasterInstallerFolderBox.TabIndex = 1;
			// 
			// CancelButton
			// 
			this.CancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.CancelButton.Location = new System.Drawing.Point(500, 165);
			this.CancelButton.Name = "CancelButton";
			this.CancelButton.Size = new System.Drawing.Size(75, 23);
			this.CancelButton.TabIndex = 2;
			this.CancelButton.Text = "Cancel";
			this.CancelButton.UseVisualStyleBackColor = true;
			this.CancelButton.Click += new System.EventHandler(this.OnClickCancel);
			// 
			// OkButton
			// 
			this.OkButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.OkButton.Location = new System.Drawing.Point(406, 164);
			this.OkButton.Name = "OkButton";
			this.OkButton.Size = new System.Drawing.Size(75, 23);
			this.OkButton.TabIndex = 3;
			this.OkButton.Text = "OK";
			this.OkButton.UseVisualStyleBackColor = true;
			this.OkButton.Click += new System.EventHandler(this.OnClickOk);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(10, 10);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(191, 13);
			this.label1.TabIndex = 4;
			this.label1.Text = "Location of main Master Installer folder:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(10, 55);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(162, 13);
			this.label2.TabIndex = 5;
			this.label2.Text = "Location of main Products folder:";
			// 
			// ProductsFolderBox
			// 
			this.ProductsFolderBox.Location = new System.Drawing.Point(10, 70);
			this.ProductsFolderBox.Name = "ProductsFolderBox";
			this.ProductsFolderBox.Size = new System.Drawing.Size(485, 20);
			this.ProductsFolderBox.TabIndex = 6;
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(10, 100);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(180, 13);
			this.label3.TabIndex = 7;
			this.label3.Text = "Default location of CD Images folder:";
			// 
			// CdImagesFolderBox
			// 
			this.CdImagesFolderBox.Location = new System.Drawing.Point(10, 115);
			this.CdImagesFolderBox.Name = "CdImagesFolderBox";
			this.CdImagesFolderBox.Size = new System.Drawing.Size(485, 20);
			this.CdImagesFolderBox.TabIndex = 8;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(10, 145);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(173, 13);
			this.label4.TabIndex = 9;
			this.label4.Text = "Drive letter for digital certificate CD:";
			// 
			// CdDrivesComboBox
			// 
			this.CdDrivesComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.CdDrivesComboBox.FormattingEnabled = true;
			this.CdDrivesComboBox.Location = new System.Drawing.Point(184, 141);
			this.CdDrivesComboBox.Name = "CdDrivesComboBox";
			this.CdDrivesComboBox.Size = new System.Drawing.Size(51, 21);
			this.CdDrivesComboBox.TabIndex = 10;
			// 
			// MasterInstallerBrowseButton
			// 
			this.MasterInstallerBrowseButton.Location = new System.Drawing.Point(500, 23);
			this.MasterInstallerBrowseButton.Name = "MasterInstallerBrowseButton";
			this.MasterInstallerBrowseButton.Size = new System.Drawing.Size(75, 23);
			this.MasterInstallerBrowseButton.TabIndex = 11;
			this.MasterInstallerBrowseButton.Text = "Browse...";
			this.MasterInstallerBrowseButton.UseVisualStyleBackColor = true;
			this.MasterInstallerBrowseButton.Click += new System.EventHandler(this.OnClickMasterInstallerBrowse);
			// 
			// ProductsBrowseButton
			// 
			this.ProductsBrowseButton.Location = new System.Drawing.Point(500, 68);
			this.ProductsBrowseButton.Name = "ProductsBrowseButton";
			this.ProductsBrowseButton.Size = new System.Drawing.Size(75, 23);
			this.ProductsBrowseButton.TabIndex = 12;
			this.ProductsBrowseButton.Text = "Browse...";
			this.ProductsBrowseButton.UseVisualStyleBackColor = true;
			this.ProductsBrowseButton.Click += new System.EventHandler(this.OnClickProductsBrowse);
			// 
			// CdsBrowseButton
			// 
			this.CdsBrowseButton.Location = new System.Drawing.Point(500, 113);
			this.CdsBrowseButton.Name = "CdsBrowseButton";
			this.CdsBrowseButton.Size = new System.Drawing.Size(75, 23);
			this.CdsBrowseButton.TabIndex = 13;
			this.CdsBrowseButton.Text = "Browse...";
			this.CdsBrowseButton.UseVisualStyleBackColor = true;
			this.CdsBrowseButton.Click += new System.EventHandler(this.OnClickCDsBrowse);
			// 
			// InitUtils
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(583, 200);
			this.Controls.Add(this.CdsBrowseButton);
			this.Controls.Add(this.ProductsBrowseButton);
			this.Controls.Add(this.MasterInstallerBrowseButton);
			this.Controls.Add(this.CdDrivesComboBox);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.CdImagesFolderBox);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.ProductsFolderBox);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.OkButton);
			this.Controls.Add(this.CancelButton);
			this.Controls.Add(this.MasterInstallerFolderBox);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "InitUtils";
			this.Text = "Initialize Master Installer Utilities";
			this.Load += new System.EventHandler(this.InitUtils_Load);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox MasterInstallerFolderBox;
		private System.Windows.Forms.Button CancelButton;
		private System.Windows.Forms.Button OkButton;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox ProductsFolderBox;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox CdImagesFolderBox;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox CdDrivesComboBox;
		private System.Windows.Forms.Button MasterInstallerBrowseButton;
		private System.Windows.Forms.Button ProductsBrowseButton;
		private System.Windows.Forms.Button CdsBrowseButton;
	}
}

