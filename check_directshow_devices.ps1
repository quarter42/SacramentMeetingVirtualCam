# PowerShell script to enumerate DirectShow video capture devices
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "DirectShow Video Capture Devices Enumeration" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

# Create DirectShow System Device Enumerator
$type = [Type]::GetTypeFromCLSID([Guid]"62BE5D10-60EB-11d0-BD3B-00A0C911CE86")
$devEnum = [Activator]::CreateInstance($type)

# Video Input Device Category
$videoCategory = [Guid]"860BB310-5D01-11d0-BD3B-00A0C911CE86"

Write-Host "Enumerating video capture devices..." -ForegroundColor Yellow
Write-Host ""

try {
    $enumMoniker = $devEnum.CreateClassEnumerator($videoCategory, 0)

    if ($enumMoniker -eq $null) {
        Write-Host "No video capture devices found!" -ForegroundColor Red
        exit
    }

    $count = 0
    $found = $false

    while ($true) {
        $moniker = $null
        $fetched = 0
        $enumMoniker.Next(1, [ref]$moniker, [ref]$fetched)

        if ($fetched -eq 0) { break }

        $count++

        # Get property bag
        $propBag = $null
        $guid = [Guid]"55272A00-42CB-11CE-8135-00AA004BB851"
        $moniker.BindToStorage($null, $null, $guid, [ref]$propBag)

        if ($propBag -ne $null) {
            $name = $null
            $propBag.Read("FriendlyName", [ref]$name, $null)

            if ($name -match "Sacrament") {
                Write-Host "[$count] $name" -ForegroundColor Green
                Write-Host "      ^-- FOUND! Sacrament camera is registered!" -ForegroundColor Green
                $found = $true
            } else {
                Write-Host "[$count] $name" -ForegroundColor White
            }
        }
    }

    Write-Host ""
    Write-Host "Total devices found: $count" -ForegroundColor Cyan
    Write-Host ""

    if (-not $found) {
        Write-Host "Sacrament Virtual Camera NOT found in DirectShow enumeration!" -ForegroundColor Red
        Write-Host ""
        Write-Host "This means the category registration failed." -ForegroundColor Yellow
        Write-Host "Check registry at:" -ForegroundColor Yellow
        Write-Host "  HKCR\CLSID\{860BB310-5D01-11d0-BD3B-00A0C911CE86}\Instance\" -ForegroundColor Yellow
    }

} catch {
    Write-Host "Error enumerating devices: $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
