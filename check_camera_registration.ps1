# Check Sacrament Virtual Camera Registration Status

Write-Host "=== Sacrament Virtual Camera Registration Check ===" -ForegroundColor Cyan
Write-Host ""

$clsid = "{4F8B3A50-1E5D-4E3A-8F2B-1234567890AB}"

# Check CLSID registration
Write-Host "1. Checking CLSID Registration..." -ForegroundColor Yellow
$clsidPath = "HKLM:\SOFTWARE\Classes\CLSID\$clsid"
if (Test-Path $clsidPath) {
    Write-Host "   [OK] CLSID registered at: $clsidPath" -ForegroundColor Green

    # Check InprocServer32
    $inprocPath = "$clsidPath\InprocServer32"
    if (Test-Path $inprocPath) {
        $dllPath = (Get-ItemProperty $inprocPath).'(default)'
        Write-Host "   [OK] DLL path: $dllPath" -ForegroundColor Green

        # Check if DLL exists
        if (Test-Path $dllPath) {
            Write-Host "   [OK] DLL file exists" -ForegroundColor Green
        } else {
            Write-Host "   [ERROR] DLL file not found!" -ForegroundColor Red
        }
    } else {
        Write-Host "   [ERROR] InprocServer32 not found!" -ForegroundColor Red
    }

    # Check FrameServerMode
    $frameServerMode = Get-ItemProperty -Path $clsidPath -Name "EnableFrameServerMode" -ErrorAction SilentlyContinue
    if ($frameServerMode) {
        Write-Host "   [OK] EnableFrameServerMode = $($frameServerMode.EnableFrameServerMode)" -ForegroundColor Green
    } else {
        Write-Host "   [WARN] EnableFrameServerMode not set" -ForegroundColor Yellow
    }
} else {
    Write-Host "   [ERROR] CLSID not registered!" -ForegroundColor Red
}

Write-Host ""

# Check DirectShow category registration
Write-Host "2. Checking DirectShow Video Capture Category..." -ForegroundColor Yellow
$categoryPath = "HKLM:\SOFTWARE\Classes\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\$clsid"
if (Test-Path $categoryPath) {
    Write-Host "   [OK] Camera registered in Video Capture category" -ForegroundColor Green
    $friendlyName = (Get-ItemProperty $categoryPath).FriendlyName
    Write-Host "   [OK] Friendly Name: $friendlyName" -ForegroundColor Green
} else {
    Write-Host "   [ERROR] Not found in Video Capture category!" -ForegroundColor Red
}

Write-Host ""

# Check if Windows Camera Service can see it
Write-Host "3. Checking Windows Camera Frame Server..." -ForegroundColor Yellow
try {
    $cameras = Get-PnpDevice | Where-Object { $_.Class -eq "Camera" }
    $found = $false
    foreach ($cam in $cameras) {
        if ($cam.FriendlyName -like "*Sacrament*") {
            Write-Host "   [OK] Found in PnP devices: $($cam.FriendlyName)" -ForegroundColor Green
            $found = $true
        }
    }
    if (-not $found) {
        Write-Host "   [WARN] Not found as PnP device (might be OK for virtual cameras)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "   [WARN] Could not check PnP devices" -ForegroundColor Yellow
}

Write-Host ""

# Try to enumerate with DirectShow
Write-Host "4. Attempting DirectShow Enumeration..." -ForegroundColor Yellow
Write-Host "   (This requires running from elevated prompt)" -ForegroundColor Gray

$code = @"
using System;
using System.Runtime.InteropServices;

public class DSEnumerator {
    [DllImport("ole32.dll")]
    public static extern int CoInitialize(IntPtr pvReserved);

    [DllImport("ole32.dll")]
    public static extern void CoUninitialize();
}
"@

try {
    Add-Type -TypeDefinition $code -ErrorAction SilentlyContinue
    [DSEnumerator]::CoInitialize([IntPtr]::Zero)
    Write-Host "   [OK] COM initialized" -ForegroundColor Green
    [DSEnumerator]::CoUninitialize()
} catch {
    Write-Host "   [WARN] Could not test DirectShow enumeration" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Check Complete ===" -ForegroundColor Cyan
