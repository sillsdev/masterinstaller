using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Windows.Forms;

/*
 * This utility is essentially a wrapper for the Microsoft SignTool utility.
 * It is geared toward signing of Installers, Master Installers and Download Packages.
 * It accepts several of the command-line arguments that SignTool accepts (when
 * SignTool is used with the "sign" qualifier), and if enough arguments are given
 * to do the signing, the signing is done with no user interface. If there are
 * insufficient arguments to do any signing, a dialog box is brought up allowing
 * the user to supply the missing information.
 * A distinct advantage of this SignMaster is the ability to specify mulitple files
 * to be signed. All the files will have the same certificate, description etc.
 */

namespace SignMaster
{
	class SignMaster
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			// Set up nice appearance:
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			// Just create an instance of our main class and run it:
			var signMaster = new SignMaster();
			signMaster.Run(args);
		}

		private SigningParameters m_parameters; // paramters used for signing.

		/// <summary>
		/// Constructor
		/// </summary>
		public SignMaster()
		{
			m_parameters = new SigningParameters();
		}

		/// <summary>
		/// Main control method
		/// </summary>
		/// <param name="cmdArgs">Command line arguments</param>
		private void Run(string[] cmdArgs)
		{
			try
			{
				Init(cmdArgs);
				if (GetAnyNeededUserInput()) // returns false only if user canceled.
					SignFiles();
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, "Error encountered in SignMaster",
					MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		/// <summary>
		/// Initialize the SignMaster object
		/// </summary>
		/// <param name="cmdArgs"></param>
		private void Init(string[] cmdArgs)
		{
			// All we have to do is set up the m_parameters according to 
			// the command line arguments:
			m_parameters.ParseCommandLineArgs(cmdArgs);
		}

		/// <summary>
		/// Test if we have enough data to do a signing. If not, display a
		/// dialog box asking for the remainder.
		/// </summary>
		/// <returns>false if user canceled, otherwise true.</returns>
		private bool GetAnyNeededUserInput()
		{
			// Test if we have enough data:
			if (m_parameters.IsUserInputNeeded())
			{
				// Show dialog box asking for more details:
				var dlg = new SignMasterForm(ref m_parameters);
				if (dlg.ShowDialog() == DialogResult.Cancel)
					return false;
			}
			// Make sure the input data is sensible:
			m_parameters.ValidateData();
			return true;
		}

		/// <summary>
		/// Displays a progress dialog in our main thread and initiates the signing
		/// of files in another thread.
		/// </summary>
		private void SignFiles()
		{
			// Initiate the progress dialog:
			var progressForm = new ProgressForm();
			progressForm.Init(m_parameters.PathsOfFilesToSign.Count);

			// Launch the signing task:
			System.Threading.ThreadPool.QueueUserWorkItem(ReallySignFiles, progressForm);

			// Display progress dialog:
			progressForm.ShowDialog();
		}

		/// <summary>
		/// This is a callback method run in a secondary thread from the main UI.
		/// It launches SignTool to do the actual signing, and monitors for problems.
		/// </summary>
		/// <param name="status">Interface to the progress dialog</param>
		private void ReallySignFiles(object status)
		{
			// Get a proper pointer to the progress dialog's interface:
			var progressForm = status as IProgressCallback;
			// Tally of failures:
			var numProblems = 0;

			// We loop over each file to be signed:
			foreach (var filePath in m_parameters.PathsOfFilesToSign)
			{
				retry: // Label so we can repeatedly try to sign current file.

				// Check if user canceled on progress dialog and if not, record progress:
				if (progressForm != null)
				{
					if (progressForm.IsCanceled())
						break;
					progressForm.BeginStep(filePath);
				}

				// On 4/20/11 a problem was identified (while using this application's
				// predecessor SignFile.js) in which files on a networked drive mapped
				// to G: could not be signed. Consequently, this fix has been implemented:
				// If file is on a networked drive, move it to the %TEMP% folder:
				string adjustedFilePath = null;
				try
				{
					var driveName = Path.GetPathRoot(filePath);
					var driveInfo = new DriveInfo(driveName);
					if (driveInfo.DriveType == DriveType.Network)
					{
						adjustedFilePath = Path.Combine(Environment.GetEnvironmentVariable("TEMP"),
							Path.GetFileName(filePath));
						File.Move(filePath, adjustedFilePath);
					}
				}
				catch
				{
					// Ignore any errors - we will just proceed without having moved the file:
					adjustedFilePath = null;
				}

				try
				{
					// Set up SignTool process with all the paramters needed:
					var signProc = new Process();
					signProc.StartInfo.FileName = "SignTool";
					signProc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
					signProc.StartInfo.Arguments = "sign";
					signProc.StartInfo.Arguments += " /f \"" + m_parameters.CertificatePath + "\"";
					signProc.StartInfo.Arguments += " /p \"" + m_parameters.Password + "\"";
					signProc.StartInfo.Arguments += " /d \"" + m_parameters.Description + "\"";
					signProc.StartInfo.Arguments += " /t \"" + m_parameters.TimestampUrl + "\"";
					if (m_parameters.FurtherInfoUrl != null)
						signProc.StartInfo.Arguments += " /du \"" + m_parameters.FurtherInfoUrl + "\"";
					signProc.StartInfo.Arguments += " \"" + (adjustedFilePath ?? filePath) + "\"";
					// We don't want to see SignTool's DOS window:
					signProc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
					signProc.StartInfo.CreateNoWindow = true;

					// Capture any failure information from SignTool:
					signProc.StartInfo.UseShellExecute = false;
					signProc.StartInfo.RedirectStandardError = true;
					signProc.StartInfo.RedirectStandardOutput = true;

					// Run SignTool:
					signProc.Start();

					// Store any failure information from SignTool:
					var output = signProc.StandardOutput.ReadToEnd();
					var error = signProc.StandardError.ReadToEnd();

					signProc.WaitForExit();

					if (adjustedFilePath != null)
						File.Move(adjustedFilePath, filePath);

					// Check for problems during signing:);
					if (signProc.ExitCode == 0)
					{
						// Update progress dialog:
						if (progressForm != null)
							progressForm.EndStep("Done.");
					}
					else
					{
						// Create string describing problem:
						var problemType = "Unhandled exception.";
						if (signProc.ExitCode == 1)
							problemType = "Failed execution.";
						else if (signProc.ExitCode == 2)
							problemType = "Completed with warnings.";

						var message = problemType + Environment.NewLine;
						if (output.Length > 0)
						{
							message += "Output stream:" + Environment.NewLine;
							message += output;
						}
						if (error.Length > 0)
						{
							message += "Error stream:" + Environment.NewLine;
							message += error;
						}
						// Update progress dialog:
						if (progressForm != null)
							progressForm.EndStep(message);

						// Inform user of the problem and give them the AbortRetryIgnore option:
						switch (MessageBox.Show(message, "SignMaster found problem executing SignTool",
							MessageBoxButtons.AbortRetryIgnore, MessageBoxIcon.Error))
						{
							case DialogResult.Retry:
								goto retry;
							case DialogResult.Abort:
								numProblems++;
								if (progressForm != null)
									progressForm.Finished(false);
								return;
						}
						numProblems++;
					} // End else exit code non-zero
				}
				catch
				{
					if (progressForm != null)
					{
						progressForm.EndStep("Crash while attempting to run SignTool utility.");
						numProblems++;
						break;
					}
				}
			}
			// Notify progress dialog of completion of work:
			if (progressForm != null)
				progressForm.Finished(!progressForm.IsCanceled() && numProblems == 0);
		}
	}
}
