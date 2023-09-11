using Nativity;
using System;
using System.Runtime.InteropServices;
using Windows.Win32;
using Windows.Win32.Foundation;
using Windows.Win32.System.Com;
using static Windows.Win32.Foundation.HRESULT;
using static Windows.Win32.PInvoke;

namespace NativityClient;

internal static class Program
{
	[STAThread]
	unsafe static void Main(string[] args)
	{
		CreateBindCtx(0, out var bind).ThrowOnFailure();
		CreateClassMoniker(typeof(INativity).GUID, out var moniker).ThrowOnFailure();
		bind.GetRunningObjectTable(out var rot);

		rot.EnumRunning(out var enumRunning);
		IMoniker[] monikers = [null];
		ref var otherMoniker = ref monikers[0];
		while (enumRunning.Next(monikers, null) == S_OK)
		{
            if (moniker.IsEqual(otherMoniker) != S_OK)
			{
				continue;
			}

			rot.GetObject(otherMoniker, out var ppunkObject);
            Console.WriteLine("Found object in ROT");
            if (ppunkObject is INativity nativity)
			{
                Console.WriteLine("Overlays?");
                nativity.OverlaysEnabled();
			}
		}
	}
}
