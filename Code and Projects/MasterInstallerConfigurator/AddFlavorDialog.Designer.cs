namespace MasterInstallerConfigurator
{
	partial class AddFlavorDialog
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
			this._flavorName = new System.Windows.Forms.TextBox();
			this._flavorUrl = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this._okBtn = new System.Windows.Forms.Button();
			this._cancelBtn = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// _flavorName
			// 
			this._flavorName.Location = new System.Drawing.Point(35, 85);
			this._flavorName.Name = "_flavorName";
			this._flavorName.Size = new System.Drawing.Size(131, 20);
			this._flavorName.TabIndex = 0;
			// 
			// _flavorUrl
			// 
			this._flavorUrl.Location = new System.Drawing.Point(234, 85);
			this._flavorUrl.Name = "_flavorUrl";
			this._flavorUrl.Size = new System.Drawing.Size(320, 20);
			this._flavorUrl.TabIndex = 1;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(35, 49);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(67, 13);
			this.label1.TabIndex = 2;
			this.label1.Text = "Flavor Name";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(231, 49);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(52, 13);
			this.label2.TabIndex = 3;
			this.label2.Text = "Flavor Url";
			// 
			// _okBtn
			// 
			this._okBtn.Location = new System.Drawing.Point(38, 132);
			this._okBtn.Name = "_okBtn";
			this._okBtn.Size = new System.Drawing.Size(75, 23);
			this._okBtn.TabIndex = 4;
			this._okBtn.Text = "Ok";
			this._okBtn.UseVisualStyleBackColor = true;
			this._okBtn.Click += new System.EventHandler(this._okBtn_Click);
			// 
			// _cancelBtn
			// 
			this._cancelBtn.Location = new System.Drawing.Point(135, 131);
			this._cancelBtn.Name = "_cancelBtn";
			this._cancelBtn.Size = new System.Drawing.Size(75, 23);
			this._cancelBtn.TabIndex = 5;
			this._cancelBtn.Text = "Cancel";
			this._cancelBtn.UseVisualStyleBackColor = true;
			this._cancelBtn.Click += new System.EventHandler(this._cancelBtn_Click);
			// 
			// AddFlavorDialog
			// 
			this.AcceptButton = this._okBtn;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(678, 174);
			this.Controls.Add(this._cancelBtn);
			this.Controls.Add(this._okBtn);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this._flavorUrl);
			this.Controls.Add(this._flavorName);
			this.Name = "AddFlavorDialog";
			this.Text = "AddFlavorDialog";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TextBox _flavorName;
		private System.Windows.Forms.TextBox _flavorUrl;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button _okBtn;
		private System.Windows.Forms.Button _cancelBtn;
	}
}