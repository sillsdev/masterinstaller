using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace SignMaster
{
	/// <summary>
	/// The class of the dialog requesting full details for the signing task from the user.
	/// </summary>
	public partial class SignMasterForm : Form
	{
		private readonly SigningParameters m_signingParamters; // Storage for signing details
		private string[] m_conflictingFilePaths; // Temporary store for drag'n'drop files.

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="signParams">Reference to main structure containing signing details.</param>
		public SignMasterForm(ref SigningParameters signParams)
		{
			// Keep local reference to main signing details structure:
			m_signingParamters = signParams;
			InitializeComponent();

			// Fill out controls according to signing details known so far:
			if (m_signingParamters.Password != null)
				eboxPassword.Text = m_signingParamters.Password;

			if (m_signingParamters.PathsOfFilesToSign != null)
				eboxFilePaths.Text = string.Join("; ", m_signingParamters.PathsOfFilesToSign.ToArray());

			if (m_signingParamters.CertificatePath != null)
				eboxCertificatePath.Text = m_signingParamters.CertificatePath;

			if (m_signingParamters.Description != null)
				cboxDescription.Text = m_signingParamters.Description;

			if (m_signingParamters.TimestampUrl != null)
				eboxTimeStampUrl.Text = m_signingParamters.TimestampUrl;

			if (m_signingParamters.FurtherInfoUrl != null)
				eboxFurtherInfoUrl.Text = m_signingParamters.FurtherInfoUrl;
		}

		/// <summary>
		/// Response to user clicking button to commence signing process.
		/// </summary>
		/// <param name="sender">the "Sign!" button</param>
		/// <param name="e"></param>
		private void OnBtnSignClicked(object sender, EventArgs e)
		{
			// Fill in signing details structure with data from dialog controls:
			m_signingParamters.Password = eboxPassword.Text;

			// Get file paths from semi-colon (or comma) separated text into string array:
			var filePaths = new List<string>();
			filePaths.AddRange(eboxFilePaths.Text.Split(new[] { ";", "," }, StringSplitOptions.RemoveEmptyEntries));
			// Remove leading and trailing whitespace:
			for (var i = 0; i < filePaths.Count; i++)
				filePaths[i] = filePaths[i].Trim();
			m_signingParamters.PathsOfFilesToSign = filePaths;

			m_signingParamters.CertificatePath = eboxCertificatePath.Text;
			m_signingParamters.Description = cboxDescription.Text;
			m_signingParamters.TimestampUrl = eboxTimeStampUrl.Text;
			m_signingParamters.FurtherInfoUrl = eboxFurtherInfoUrl.Text;

			DialogResult = DialogResult.OK;
			Close();
		}

		/// <summary>
		/// Response to user canceling.
		/// </summary>
		/// <param name="sender">The "Cancel" button</param>
		/// <param name="e"></param>
		private void OnBtnCancelClicked(object sender, EventArgs e)
		{
			DialogResult = DialogResult.Cancel;
			Close();
		}

		/// <summary>
		/// Response to user dragging files over the files-to-be-signed edit box.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnFilesToBeSignedDragEnter(object sender, DragEventArgs e)
		{
			// Make sure user is dropping files (not text or anything else):
			if (e.Data.GetDataPresent(DataFormats.FileDrop, false))
				e.Effect = DragDropEffects.All; // Allow user to continue
		}

		/// <summary>
		/// Response to user dropping files into the files-to-be-signed edit box.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnFilesToBeSignedPathDrop(object sender, DragEventArgs e)
		{
			// Get array of file paths:
			var files = (string[])e.Data.GetData(DataFormats.FileDrop);

			// If there are already files listed in the text box, give the user the option
			// to append or replace with the dropped files via a popup menu:
			if (eboxFilePaths.Text.Length > 0)
			{
				// Store list of dropped files to be picked up later:
				m_conflictingFilePaths = files;

				// Set up popup menu:
				var menu = new ContextMenu();
				menu.MenuItems.Add("Append", OnFileSetAppend);
				menu.MenuItems.Add("Replace", OnFileSetReplace);
				menu.Show((Control)sender, new Point(eboxFilePaths.Width, 0));
				// The user's selection will come later via OnFileSetAppend or OnFileSetReplace.
			}
			else
				eboxFilePaths.Text = string.Join("; ", files);
		}

		/// <summary>
		/// Called by popup menu when user dropped files into pre-existing list and then opted for append.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnFileSetAppend(object sender, EventArgs e)
		{
			// Append file array to existing list:
			eboxFilePaths.Text += "; " + string.Join("; ", m_conflictingFilePaths);
		}

		/// <summary>
		/// Called by popup menu when user dropped files into pre-existing list and then opted for replace.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnFileSetReplace(object sender, EventArgs e)
		{
			// Replace existing list with file array:
			eboxFilePaths.Text = string.Join("; ", m_conflictingFilePaths);
		}

		/// <summary>
		/// Response to user dragging a file over the certificate path edit box.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnPathOfCertificateDragEnter(object sender, DragEventArgs e)
		{
			// Make sure user is dropping files (not text or anything else):
			if (e.Data.GetDataPresent(DataFormats.FileDrop, false))
				e.Effect = DragDropEffects.All; // Allow user to continue
		}

		/// <summary>
		/// Response to user dropping a file into the certificate path edit box.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void OnPathOfCertificatePathDrop(object sender, DragEventArgs e)
		{
			var files = (string[])e.Data.GetData(DataFormats.FileDrop);
			// Only use first file (in case user dropped more than one):
			eboxCertificatePath.Text = files[0];
		}

		/// <summary>
		/// Response to user pressing Browse button beside the files-to-be-signed edit box.
		/// </summary>
		/// <param name="sender">The  "Browse..." button</param>
		/// <param name="e"></param>
		private void BtnBrowseFilesClick(object sender, EventArgs e)
		{
			// Give the user a File-Open dialog box that allows multiple selections:
			var dlg = new OpenFileDialog();
			dlg.CheckFileExists = true;
			dlg.Multiselect = true;
			if (dlg.ShowDialog() == DialogResult.OK)
			{
				if (eboxFilePaths.Text.Length > 0)
					eboxFilePaths.Text += "; " + string.Join("; ", dlg.FileNames);
				else
					eboxFilePaths.Text = string.Join("; ", dlg.FileNames);
			}
		}

		/// <summary>
		/// Response to user pressing Browse button beside the certificate path edit box.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void BtnBrowseCertificateClick(object sender, EventArgs e)
		{
			// Give the user a File-Open dialog box that allows a single selection:
			var dlg = new OpenFileDialog();
			dlg.CheckFileExists = true;
			dlg.Multiselect = false;
			if (dlg.ShowDialog() == DialogResult.OK)
				eboxCertificatePath.Text = dlg.FileName;
		}
	}
}
