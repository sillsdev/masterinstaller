using System;
using System.Windows.Forms;

namespace SignMaster
{
	/// <summary>
	/// Interface allowing one thread to control the progress dialog in another thread.
	/// </summary>
	public interface IProgressCallback
	{
		void Init(int numSteps);
		bool IsCanceled();
		void BeginStep(string item);
		void EndStep(string report);
		void Finished(bool canClose);
	}

	/// <summary>
	/// Progress dialog class
	/// </summary>
	public partial class ProgressForm : Form, IProgressCallback
	{
		private bool m_canceled; // True if user pressed Cancel button.

		/// <summary>
		/// Constructor
		/// </summary>
		public ProgressForm()
		{
			m_canceled = false;
			InitializeComponent();
		}

		/// <summary>
		/// Response to user pressing Cancel button
		/// </summary>
		/// <param name="sender">The "Cancel" button</param>
		/// <param name="e"></param>
		private void BtnProgressCancelClick(object sender, EventArgs e)
		{
			// If work is already finished, but with errors, this is the main
			// way the user terminates the process - the Cancel button will
			// have been renamed "Close":
			if (btnProgressCancel.Text == "Close")
				Close();
			else
				m_canceled = true;
		}

		/// <summary>
		/// Initializes the progress dialog.
		/// </summary>
		/// <param name="numSteps">Number of steps of work to be done</param>
		public void Init(int numSteps)
		{
			m_canceled = false;

			// Set up progress bar:
			progressBar.Minimum = 0;
			progressBar.Maximum = numSteps;
			progressBar.Step = 1;
			progressBar.Value = 0;
		}

		public bool IsCanceled()
		{
			return m_canceled;
		}

		// Several delegate definitions are needed, so that methods which
		// affect dialog controls can be safely called from a different thread:
		// A delegate for methods that take no parameters:
		public delegate void VoidInvoker();
		// A delegate for methods that take a string:
		public delegate void TextInvoker(string text);
		// A delegate for methods that take a boolean:
		public delegate void BoolInvoker(bool flag);

		/// <summary>
		/// Called when a new progress item of work is started.
		/// </summary>
		/// <param name="item">Description of work, typically the name of the file about to be signed.</param>
		public void BeginStep(string item)
		{
			// Check whether we can do this directly or we need to invoke a delegate:
			if (progressText.InvokeRequired)
				Invoke(new TextInvoker(DoBeginStep), new object[] { item });
			else
				DoBeginStep(item);
		}

		/// <summary>
		/// Delegated version of BeginStep().
		/// </summary>
		/// <param name="item">Description of work, typically the name of the file about to be signed.</param>
		private void DoBeginStep(string item)
		{
			progressText.Text += "Signing: " + item + "... ";
			ScrollProgressTextToBottom();
		}

		/// <summary>
		/// Called when a progress item of work is completed.
		/// </summary>
		/// <param name="report">Desciption of any problems encounterd while doing the work step.</param>
		public void EndStep(string report)
		{
			// Check whether we can do this directly or we need to invoke a delegate:
			if (progressBar.InvokeRequired)
				Invoke(new TextInvoker(DoEndStep), new object[] { report });
			else
				DoEndStep(report);
		}

		/// <summary>
		/// Delegated version of EndStep().
		/// </summary>
		/// <param name="report">Desciption of any problems encounterd while doing the work step.</param>
		private void DoEndStep(string report)
		{
			progressBar.PerformStep();
			progressText.Text += report + Environment.NewLine;
			ScrollProgressTextToBottom();
		}

		/// <summary>
		/// Called when all work covered by progress dialog is complete.
		/// </summary>
		/// <param name="canClose">True if work completed successfully.</param>
		public void Finished(bool canClose)
		{
			// Check whether we can do this directly or we need to invoke a delegate:
			if (InvokeRequired)
				Invoke(new BoolInvoker(DoFinished), new object[] { canClose });
			else
				DoFinished(canClose);
		}

		/// <summary>
		/// Delegated version of Finished().
		/// </summary>
		/// <param name="canClose">True if work completed successfully.</param>
		private void DoFinished(bool canClose)
		{
			if (canClose)
				Close();
			else
			{
				progressText.Text += "Operation terminated because of problems." + Environment.NewLine;
				btnProgressCancel.Text = "Close";
			}
		}

		/// <summary>
		/// Trick to make sure multi-line text box shows latest text added.
		/// </summary>
		private void ScrollProgressTextToBottom()
		{
			progressText.SelectionStart = progressText.Text.Length;
			progressText.ScrollToCaret();
		}
	}
}
