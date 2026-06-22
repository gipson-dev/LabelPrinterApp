param(
    [string]$Config = "Release",
    [string]$BuildDir = "build",
    [string]$OutputDir = "dist\LabelPrinterApp"
)

$ErrorActionPreference = "Stop"

& "$PSScriptRoot\build-and-test.ps1" -Config $Config -BuildDir $BuildDir

$exePath = Join-Path $BuildDir "$Config\LabelPrinterApp.exe"
if (!(Test-Path $exePath)) {
    throw "Expected executable was not found: $exePath"
}

if (Test-Path $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}

New-Item -ItemType Directory -Path $OutputDir | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "templates") | Out-Null

Copy-Item -LiteralPath $exePath -Destination (Join-Path $OutputDir "LabelPrinterApp.exe")
Copy-Item -LiteralPath "templates\default_label.json" -Destination (Join-Path $OutputDir "templates\default_label.json")
Copy-Item -LiteralPath "README.md" -Destination (Join-Path $OutputDir "README.md")
Copy-Item -LiteralPath "LICENSE" -Destination (Join-Path $OutputDir "LICENSE")

Write-Host "Packaged release to $OutputDir"
