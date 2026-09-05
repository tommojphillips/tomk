# run-qemu.ps1

param (
    [int]$mon_index = 0,
    [int]$new_w = 1800,
    [int]$new_h = 1080
)

Add-Type -AssemblyName System.Windows.Forms

Add-Type @"
using System;
using System.Runtime.InteropServices;

public class Win32 {
    [DllImport("user32.dll")]
    public static extern bool MoveWindow(
        IntPtr hWnd,
        int X,
        int Y,
        int nWidth,
        int nHeight,
        bool bRepaint
    );
}
"@

Add-Type @"
using System;
using System.Runtime.InteropServices;

public class DPI {
    [DllImport("user32.dll")]
    public static extern bool SetProcessDPIAware();
}
"@

Add-Type @"
using System;
using System.Runtime.InteropServices;

public class ConsoleCtrl {
    [DllImport("Kernel32")]
    public static extern bool SetConsoleCtrlHandler(
        HandlerRoutine HandlerRoutine,
        bool Add
    );

    public delegate bool HandlerRoutine(uint CtrlType);
}
"@

# Find QEMU started by the current CMD
$p = $null

for ($i = 0; $i -lt 100; $i++) {

    Start-Sleep -Milliseconds 10

    $p = Get-Process -Name "qemu-system-i386" -ErrorAction SilentlyContinue |
         Sort-Object StartTime -Descending |
         Select-Object -First 1

    if ($null -ne $p) {
        break
    }
}

if ($null -eq $p) {
    Write-Host "Failed to find QEMU"
    exit 1
}

# ------------------------------------------------------------
# Wait for QEMU window
# ------------------------------------------------------------

$handle = [IntPtr]::Zero

for ($i = 0; $i -lt 100; $i++) {

    Start-Sleep -Milliseconds 10

    $p.Refresh()

    if ($p.HasExited) {
        Write-Host "QEMU exited"
        exit 1
    }

    if ($p.MainWindowHandle -ne [IntPtr]::Zero) {
        $handle = $p.MainWindowHandle
        break
    }
}

if ($handle -eq [IntPtr]::Zero) {
    Write-Host "Could not find the QEMU window"
    exit 1
}

# ------------------------------------------------------------
# Find monitor 2
# ------------------------------------------------------------

[DPI]::SetProcessDPIAware() | Out-Null

$monitors = [System.Windows.Forms.Screen]::AllScreens
if ($monitors.Count -lt $mon_index + 1) {
    Write-Host "Error: Only $($monitors.Count) monitor(s) detected"
    exit 1
}

$x = $monitors[$mon_index].WorkingArea.X
$y = $monitors[$mon_index].WorkingArea.Y
$w = $monitors[$mon_index].WorkingArea.Width
$h = $monitors[$mon_index].WorkingArea.Height
$new_x = $x + (($w - $new_w) / 2)
$new_y = $y + (($h - $new_h) / 2)

# ------------------------------------------------------------
# Move QEMU
# ------------------------------------------------------------

[Win32]::MoveWindow($handle, $new_x, $new_y, $new_w, $new_h, $true) | Out-Null
