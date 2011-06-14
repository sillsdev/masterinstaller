namespace SignMaster
{
	partial class ProgressForm
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
			this.progressBar = new System.Windows.Forms.ProgressBar();
			this.btnProgressCancel = new System.Windows.Forms.Button();
			this.progressText = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// progressBar
			// 
			this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.progressBar.Location = new System.Drawing.Point(12, 12);
			this.progressBar.Name = "progressBar";
			this.progressBar.Size = new System.Drawing.Size(324, 23);
			this.progressBar.TabIndex = 0;
			// 
			// btnProgressCancel
			// 
			this.btnProgressCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnProgressCancel.Location = new System.Drawing.Point(261, 176);
			this.btnProgressCancel.Name = "btnProgressCancel";
			this.btnProgressCancel.Size = new System.Drawing.Size(75, 23);
			this.btnProgressCancel.TabIndex = 2;
			this.btnProgressCancel.Text = "Cancel";
			this.btnProgressCancel.UseVisualStyleBackColor = true;
			this.btnProgressCancel.Click += new System.EventHandler(this.BtnProgressCancelClick);
			// 
			// progressText
			// 
			this.progressText.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
						| System.Windows.Forms.AnchorStyles.Left)
						| System.Windows.Forms.AnchorStyles.Right)));
			this.progressText.Cursor = System.Windows.Forms.Cursors.Default;
			this.progressText.Location = new System.Drawing.Point(12, 41);
			this.progressText.Multiline = true;
			this.progressText.Name = "progressText";
			this.progressText.ReadOnly = true;
			this.progressText.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.progressText.Size = new System.Drawing.Size(324, 129);
			this.progressText.TabIndex = 3;
			this.progressText.WordWrap = false;
			// 
			// ProgressForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(348, 211);
			this.Controls.Add(this.progressText);
			this.Controls.Add(this.btnProgressCancel);
			this.Controls.Add(this.progressBar);
			this.Name = "ProgressForm";
			this.Text = "ProgressForm";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ProgressBar progressBar;
		private System.Windows.Forms.Button btnProgressCancel;
		private System.Windows.Forms.TextBox progressText;
	}
}