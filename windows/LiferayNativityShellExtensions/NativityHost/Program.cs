using Windows.Win32.System.Com;
using Nativity;
using System.Runtime.InteropServices;
using static Windows.Win32.PInvoke;
using System;
using System.Threading;

namespace NativityHost;

internal class Program
{
	[STAThread]
	static void Main(string[] args)
	{
		Handler handler = new();
		//CreateBindCtx(0, out var bindCtx).ThrowOnFailure();
		//bindCtx.GetRunningObjectTable(out var rot);
		GetRunningObjectTable(0, out var rot);
		CreateClassMoniker(typeof(INativity).GUID, out var moniker).ThrowOnFailure();
		rot.Register(ROT_FLAGS.ROTFLAGS_REGISTRATIONKEEPSALIVE, handler, moniker, out var register);

		ManualResetEventSlim handle = new(false);
		Console.CancelKeyPress += (s, e) => handle.Set();
		handle.Wait();
		rot.Revoke(register);
	}

	[ComDefaultInterface(typeof(INativity))]
	public class Handler : INativity
	{
		public bool IsFiltered(string Message)
		{
			Console.WriteLine($"IsFiltered: {Message}");
			return true;
		}

		public bool OverlaysEnabled()
		{
			Console.WriteLine($"OverlaysEnabled");
			return false;
		}

		public string ReceiveMessage(string Message)
		{
			Console.WriteLine($"ReceiveMessage: {Message}");
			return "[]";
		}
	}
}