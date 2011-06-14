namespace SignMaster
{
	partial class SignMasterForm
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
			this.label1 = new System.Windows.Forms.Label();
			this.eboxFilePaths = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.eboxCertificatePath = new System.Windows.Forms.TextBox();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.eboxTimeStampUrl = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.eboxFurtherInfoUrl = new System.Windows.Forms.TextBox();
			this.btnSign = new System.Windows.Forms.Button();
			this.btnCancel = new System.Windows.Forms.Button();
			this.eboxPassword = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.btnBrowseFiles = new System.Windows.Forms.Button();
			this.btnBrowseCertificate = new System.Windows.Forms.Button();
			this.cboxDescription = new System.Windows.Forms.ComboBox();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 33);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(98, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "File(s) to be signed:";
			this.label1.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// eboxFilePaths
			// 
			this.eboxFilePaths.AllowDrop = true;
			this.eboxFilePaths.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.eboxFilePaths.Location = new System.Drawing.Point(116, 30);
			this.eboxFilePaths.Name = "eboxFilePaths";
			this.eboxFilePaths.Size = new System.Drawing.Size(330, 20);
			this.eboxFilePaths.TabIndex = 1;
			this.eboxFilePaths.DragDrop += new System.Windows.Forms.DragEventHandler(this.OnFilesToBeSignedPathDrop);
			this.eboxFilePaths.DragEnter += new System.Windows.Forms.DragEventHandler(this.OnFilesToBeSignedDragEnter);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(16, 57);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(94, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Path of Certificate:";
			this.label2.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// eboxCertificatePath
			// 
			this.eboxCertificatePath.AllowDrop = true;
			this.eboxCertificatePath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.eboxCertificatePath.Location = new System.Drawing.Point(116, 54);
			this.eboxCertificatePath.Name = "eboxCertificatePath";
			this.eboxCertificatePath.Size = new System.Drawing.Size(330, 20);
			this.eboxCertificatePath.TabIndex = 2;
			this.eboxCertificatePath.DragDrop += new System.Windows.Forms.DragEventHandler(this.OnPathOfCertificatePathDrop);
			this.eboxCertificatePath.DragEnter += new System.Windows.Forms.DragEventHandler(this.OnPathOfCertificateDragEnter);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(47, 81);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(63, 13);
			this.label3.TabIndex = 4;
			this.label3.Text = "Description:";
			this.label3.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(24, 105);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(86, 13);
			this.label4.TabIndex = 6;
			this.label4.Text = "Timestamp URL:";
			this.label4.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// eboxTimeStampUrl
			// 
			this.eboxTimeStampUrl.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.eboxTimeStampUrl.Location = new System.Drawing.Point(116, 102);
			this.eboxTimeStampUrl.Name = "eboxTimeStampUrl";
			this.eboxTimeStampUrl.Size = new System.Drawing.Size(330, 20);
			this.eboxTimeStampUrl.TabIndex = 4;
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(22, 129);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(88, 13);
			this.label5.TabIndex = 8;
			this.label5.Text = "Further info URL:";
			this.label5.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// eboxFurtherInfoUrl
			// 
			this.eboxFurtherInfoUrl.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.eboxFurtherInfoUrl.Location = new System.Drawing.Point(116, 126);
			this.eboxFurtherInfoUrl.Name = "eboxFurtherInfoUrl";
			this.eboxFurtherInfoUrl.Size = new System.Drawing.Size(330, 20);
			this.eboxFurtherInfoUrl.TabIndex = 5;
			// 
			// btnSign
			// 
			this.btnSign.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.btnSign.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.btnSign.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.btnSign.Location = new System.Drawing.Point(225, 157);
			this.btnSign.Name = "btnSign";
			this.btnSign.Size = new System.Drawing.Size(85, 31);
			this.btnSign.TabIndex = 6;
			this.btnSign.Text = "Sign!";
			this.btnSign.UseVisualStyleBackColor = true;
			this.btnSign.Click += new System.EventHandler(this.OnBtnSignClicked);
			// 
			// btnCancel
			// 
			this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnCancel.Location = new System.Drawing.Point(447, 165);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.Size = new System.Drawing.Size(75, 23);
			this.btnCancel.TabIndex = 7;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.UseVisualStyleBackColor = true;
			this.btnCancel.Click += new System.EventHandler(this.OnBtnCancelClicked);
			// 
			// eboxPassword
			// 
			this.eboxPassword.AcceptsTab = true;
			this.eboxPassword.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.eboxPassword.Location = new System.Drawing.Point(116, 6);
			this.eboxPassword.Name = "eboxPassword";
			this.eboxPassword.PasswordChar = '*';
			this.eboxPassword.Size = new System.Drawing.Size(330, 20);
			this.eboxPassword.TabIndex = 0;
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(54, 9);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(56, 13);
			this.label6.TabIndex = 13;
			this.label6.Text = "Password:";
			this.label6.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// btnBrowseFiles
			// 
			this.btnBrowseFiles.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.btnBrowseFiles.Location = new System.Drawing.Point(452, 28);
			this.btnBrowseFiles.Name = "btnBrowseFiles";
			this.btnBrowseFiles.Size = new System.Drawing.Size(70, 23);
			this.btnBrowseFiles.TabIndex = 14;
			this.btnBrowseFiles.Text = "Browse...";
			this.btnBrowseFiles.UseVisualStyleBackColor = true;
			this.btnBrowseFiles.Click += new System.EventHandler(this.BtnBrowseFilesClick);
			// 
			// btnBrowseCertificate
			// 
			this.btnBrowseCertificate.Location = new System.Drawing.Point(452, 57);
			this.btnBrowseCertificate.Name = "btnBrowseCertificate";
			this.btnBrowseCertificate.Size = new System.Drawing.Size(70, 23);
			this.btnBrowseCertificate.TabIndex = 15;
			this.btnBrowseCertificate.Text = "Browse...";
			this.btnBrowseCertificate.UseVisualStyleBackColor = true;
			this.btnBrowseCertificate.Click += new System.EventHandler(this.BtnBrowseCertificateClick);
			// 
			// cboxDescription
			// 
			this.cboxDescription.FormattingEnabled = true;
			this.cboxDescription.Items.AddRange(new object[] {
            "SIL Software Installer",
            "SIL Software Package"});
			this.cboxDescription.Location = new System.Drawing.Point(116, 78);
			this.cboxDescription.Name = "cboxDescription";
			this.cboxDescription.Size = new System.Drawing.Size(330, 21);
			this.cboxDescription.TabIndex = 16;
			// 
			// SignMasterForm
			// 
			this.AcceptButton = this.btnSign;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.btnCancel;
			this.ClientSize = new System.Drawing.Size(534, 200);
			this.Controls.Add(this.cboxDescription);
			this.Controls.Add(this.btnBrowseCertificate);
			this.Controls.Add(this.btnBrowseFiles);
			this.Controls.Add(this.label6);
			this.Controls.Add(this.eboxPassword);
			this.Controls.Add(this.btnCancel);
			this.Controls.Add(this.btnSign);
			this.Controls.Add(this.eboxFurtherInfoUrl);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.eboxTimeStampUrl);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.eboxCertificatePath);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.eboxFilePaths);
			this.Controls.Add(this.label1);
			this.Name = "SignMasterForm";
			this.Text = "Sign Master";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox eboxPassword;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox eboxFilePaths;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox eboxCertificatePath;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox eboxTimeStampUrl;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox eboxFurtherInfoUrl;
		private System.Windows.Forms.Button btnSign;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button btnBrowseFiles;
		private System.Windows.Forms.Button btnBrowseCertificate;
		private System.Windows.Forms.ComboBox cboxDescription;
	}
}

