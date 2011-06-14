using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace SignMaster
{
	/// <summary>
	/// Data structure holding all the information needed to sign a file (or files).
	/// </summary>
	public class SigningParameters
	{
		public string Password { get; set; }
		public List<string> PathsOfFilesToSign { get; set; }
		public string CertificatePath { get; set; }
		public string Description { get; set; }
		public string TimestampUrl { get; set; }
		public string FurtherInfoUrl { get; set; }

		/// <summary>
		/// Constructor.
		/// </summary>
		public SigningParameters()
		{
			Reset();
		}

		/// <summary>
		/// Used to delete all current data.
		/// </summary>
		private void Reset()
		{
 			Password = null;
			PathsOfFilesToSign = null;
			CertificatePath = null;
			Description = null;
			TimestampUrl = null;
			FurtherInfoUrl = null;
		}

		/// <summary>
		/// Interprets the process's command line arguments and fills in the data
		/// structure accordingly. The command line need not contain a comprehensive
		/// set of values.
		/// </summary>
		/// <param name="args">Command line arguments to our process</param>
		public void ParseCommandLineArgs(string[] args)
		{
			// Forget everything we knew before:
			Reset();

			// Set up an array of flags so we can know which arguments we've dealt with:
			var usedArgs = new bool[args.Length];

			for (var i = 0; i < args.Length; i++)
			{
				switch (args[i].ToLowerInvariant())
				{
					case "-p": // Certifiacte password
					case "/p":
						if (i + 1 >= args.Length || usedArgs[i + 1])
							throw new Exception("Option /p used with no password following it.");
						Password = args[i + 1];
						usedArgs[i] = usedArgs[i + 1] = true;
						break;
					case "-f": // Certificate file path
					case "/f":
						if (i + 1 >= args.Length || usedArgs[i + 1])
							throw new Exception("Option /f used with no certificate path following it.");
						CertificatePath = args[i + 1];
						usedArgs[i] = usedArgs[i + 1] = true;
						break;
					case "-d":
					case "/d":
						if (i + 1 >= args.Length || usedArgs[i + 1])
							throw new Exception("Option /d used with no description following it.");
						Description = args[i + 1];
						usedArgs[i] = usedArgs[i + 1] = true;
						break;
					case "-t": // URL of timestamp
					case "/t":
						if (i + 1 >= args.Length || usedArgs[i + 1])
							throw new Exception("Option /t used with no timestamp URL following it.");
						TimestampUrl = args[i + 1];
						usedArgs[i] = usedArgs[i + 1] = true;
						break;
					case "-du": // (rare) URL for product further information
					case "/du":
						if (i + 1 >= args.Length || usedArgs[i + 1])
							throw new Exception("Option /du used with no further information URL following it.");
						FurtherInfoUrl = args[i + 1];
						usedArgs[i] = usedArgs[i + 1] = true;
						break;
					default: // Path of a file to be signed
						if (usedArgs[i])
							break;
						if (PathsOfFilesToSign == null)
							PathsOfFilesToSign = new List<string>();
						PathsOfFilesToSign.Add(args[i]);
						usedArgs[i] = true;
						break;
				}
			}

			// Fill in known defaults for missing parameters:
			if (CertificatePath == null)
			{
				// If the Master Installer utilties have been initialize properly, there
				// will be an environment variable DIGITAL_CERT_DRIVE set to the drive of
				// the certificate CD:
				var digitalCertificateDrive = Environment.GetEnvironmentVariable("DIGITAL_CERT_DRIVE");
				if (digitalCertificateDrive != null)
					CertificatePath = Path.Combine(digitalCertificateDrive, "Comodo.pfx");
			}

			if (TimestampUrl == null)
				TimestampUrl = "http://timestamp.comodoca.com/authenticode";
		}

		/// <summary>
		/// Tests that the signing details do not contain easily-detected errors.
		/// </summary>
		public void ValidateData()
		{
			if (PathsOfFilesToSign != null)
			{
				if (PathsOfFilesToSign.Count == 0)
					throw new Exception("No files specified - nothing to sign!");

				foreach (var path in PathsOfFilesToSign.Where(path => !File.Exists(path)))
					throw new Exception("File to be signed does not exist: \"" + path + "\"");
			}
			if (CertificatePath != null)
			{
				var driveName = Path.GetPathRoot(CertificatePath);
				var driveInfo = new DriveInfo(driveName);
				while (!driveInfo.IsReady || !File.Exists(CertificatePath))
				{
					var cause = driveInfo.IsReady ? "The certificate file [" + CertificatePath + "] does not exist."
						: "The digital certificate CD appears not to be in the drive [" + driveName + "].";
 
					if (MessageBox.Show(cause + Environment.NewLine +
									"If you wish the file(s) to be signed, insert the CD with access to \"" +
									CertificatePath + "\" and press OK."
									+ Environment.NewLine + Environment.NewLine +
									"To continue without signing file(s), press Cancel.",
									"SignMaster can't find digital certificate",
									MessageBoxButtons.OKCancel) == DialogResult.Cancel)
					{
						throw new Exception("Certificate not found - file(s) not signed.");
					}
				}
			}
		}

		/// <summary>
		/// Tests if we need the dialog box to fill in missing details of files
		/// to be signed
		/// </summary>
		/// <returns>True if dialog box is needed. Otherwise false</returns>
		public bool IsUserInputNeeded()
		{
			if (Password == null)
				return true;

			if (PathsOfFilesToSign == null)
				return true;

			if (CertificatePath == null)
				return true;

			if (Description == null)
				return true;

			if (TimestampUrl == null)
				return true;

			return false;
		}
	}
}
