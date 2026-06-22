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
New-Item -ItemType Directory -Path (Join-Path $OutputDir "examples") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "docs") | Out-Null

Copy-Item -LiteralPath $exePath -Destination (Join-Path $OutputDir "LabelPrinterApp.exe")
Copy-Item -Path "templates\*.json" -Destination (Join-Path $OutputDir "templates")
Copy-Item -Path "examples\*.csv" -Destination (Join-Path $OutputDir "examples")
Copy-Item -LiteralPath "docs\ARCHITECTURE.md" -Destination (Join-Path $OutputDir "docs\ARCHITECTURE.md")
Copy-Item -LiteralPath "docs\example_generated.zpl" -Destination (Join-Path $OutputDir "docs\example_generated.zpl")
Copy-Item -LiteralPath "README.md" -Destination (Join-Path $OutputDir "README.md")
Copy-Item -LiteralPath "LICENSE" -Destination (Join-Path $OutputDir "LICENSE")

$windeployqt = $null
$windeployqtCommand = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
if ($windeployqtCommand) {
    $windeployqt = $windeployqtCommand.Source
}
if (!$windeployqt -and $env:CMAKE_PREFIX_PATH) {
    $candidate = Join-Path $env:CMAKE_PREFIX_PATH "bin\windeployqt.exe"
    if (Test-Path $candidate) {
        $windeployqt = $candidate
    }
}
if (!$windeployqt) {
    $qtRoot = "C:\Qt"
    if (Test-Path $qtRoot) {
        $qtPrefix = Get-ChildItem -Path $qtRoot -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object {
                $prefix = Join-Path $_.FullName "msvc2022_64"
                $candidate = Join-Path $prefix "bin\windeployqt.exe"
                if (Test-Path $candidate) {
                    $candidate
                }
            } |
            Select-Object -First 1
        if ($qtPrefix) {
            $windeployqt = $qtPrefix
        }
    }
}

if ($windeployqt) {
    & $windeployqt (Join-Path $OutputDir "LabelPrinterApp.exe")
} else {
    Write-Warning "windeployqt.exe was not found. Copy Qt runtime DLLs manually or add Qt bin to PATH."
}

Write-Host "Packaged release to $OutputDir"
