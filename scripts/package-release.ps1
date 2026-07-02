param(
    [string]$Config = "Release",
    [string]$BuildDir = "build",
    [string]$OutputDir = "dist\LabelPrinterApp"
)

$ErrorActionPreference = "Stop"

function Get-CMakeCacheValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $cachePath = Join-Path $BuildDir "CMakeCache.txt"
    if (!(Test-Path $cachePath)) {
        return $null
    }

    $pattern = "^$([regex]::Escape($Name)):[^=]*=(.*)$"
    $match = Select-String -LiteralPath $cachePath -Pattern $pattern | Select-Object -First 1
    if (!$match) {
        return $null
    }

    return $match.Matches[0].Groups[1].Value.Trim()
}

function Add-DirectoryCandidate {
    param(
        [System.Collections.Generic.List[string]]$Candidates,
        [string]$Path
    )

    if (!$Path) {
        return
    }

    $resolved = Resolve-Path $Path -ErrorAction SilentlyContinue
    if (!$resolved) {
        return
    }

    $fullPath = $resolved.Path
    if (!$Candidates.Contains($fullPath)) {
        $Candidates.Add($fullPath)
    }
}

function Add-OpenSslRootCandidate {
    param(
        [System.Collections.Generic.List[string]]$Candidates,
        [string]$Root
    )

    if (!$Root) {
        return
    }

    Add-DirectoryCandidate -Candidates $Candidates -Path (Join-Path $Root "bin")
    Add-DirectoryCandidate -Candidates $Candidates -Path $Root
}

function Add-OpenSslLibCandidate {
    param(
        [System.Collections.Generic.List[string]]$Candidates,
        [string]$LibPath
    )

    if (!$LibPath) {
        return
    }

    $libDirectory = Split-Path -Parent $LibPath
    if (!$libDirectory) {
        return
    }

    foreach ($relativePath in @("..\..\..\bin", "..\..\..", "..\..\..\..", ".", "..")) {
        Add-DirectoryCandidate -Candidates $Candidates -Path (Join-Path $libDirectory $relativePath)
    }
}

function Copy-OpenSslRuntimeDlls {
    $candidateDirectories = [System.Collections.Generic.List[string]]::new()

    Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root (Get-CMakeCacheValue -Name "OPENSSL_ROOT_DIR")
    Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root $env:OPENSSL_ROOT_DIR
    Add-OpenSslLibCandidate -Candidates $candidateDirectories -LibPath (Get-CMakeCacheValue -Name "SSL_EAY_RELEASE")
    Add-OpenSslLibCandidate -Candidates $candidateDirectories -LibPath (Get-CMakeCacheValue -Name "LIB_EAY_RELEASE")

    foreach ($commonRoot in @(
        "C:\Program Files\OpenSSL-Win64",
        "C:\Program Files\OpenSSL",
        "C:\OpenSSL-Win64",
        "C:\OpenSSL"
    )) {
        Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root $commonRoot
    }

    foreach ($directory in $candidateDirectories) {
        $sslDlls = Get-ChildItem -LiteralPath $directory -Filter "libssl*.dll" -ErrorAction SilentlyContinue
        $cryptoDlls = Get-ChildItem -LiteralPath $directory -Filter "libcrypto*.dll" -ErrorAction SilentlyContinue
        if (!$sslDlls -or !$cryptoDlls) {
            continue
        }

        foreach ($dll in @($sslDlls) + @($cryptoDlls)) {
            Copy-Item -LiteralPath $dll.FullName -Destination (Join-Path $OutputDir $dll.Name) -Force
        }
        Write-Host "Copied OpenSSL runtime DLLs from $directory"
        return
    }

    Write-Warning "OpenSSL runtime DLLs were not found. The update checker requires libssl/libcrypto DLLs beside LabelPrinterApp.exe."
}

& "$PSScriptRoot\build-and-test.ps1" -Config $Config -BuildDir $BuildDir

$exePath = Join-Path $BuildDir "$Config\LabelPrinterApp.exe"
if (!(Test-Path $exePath)) {
    throw "Expected executable was not found: $exePath"
}

$launcherExePath = Join-Path $BuildDir "$Config\LabelPrinterAppLauncher.exe"
if (!(Test-Path $launcherExePath)) {
    throw "Expected executable was not found: $launcherExePath"
}

if (Test-Path $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}

New-Item -ItemType Directory -Path $OutputDir | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "templates") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "examples") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $OutputDir "docs") | Out-Null

Copy-Item -LiteralPath $exePath -Destination (Join-Path $OutputDir "LabelPrinterApp.exe")
Copy-Item -LiteralPath $launcherExePath -Destination (Join-Path $OutputDir "LabelPrinterAppLauncher.exe")
Copy-Item -Path "templates\*.json" -Destination (Join-Path $OutputDir "templates")
Copy-Item -Path "examples\*.csv" -Destination (Join-Path $OutputDir "examples")
Copy-Item -Path "docs\*" -Destination (Join-Path $OutputDir "docs")
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

Copy-OpenSslRuntimeDlls

$distRoot = Split-Path -Parent $OutputDir
$portableZip = Join-Path $distRoot "LabelPrinterApp_Portable.zip"
$setupExe = Join-Path $distRoot "LabelPrinterApp_Setup.exe"

if (Test-Path $portableZip) {
    Remove-Item -LiteralPath $portableZip -Force
}
Compress-Archive -Path $OutputDir -DestinationPath $portableZip -Force

$installerWork = Join-Path $distRoot "installer_tmp"
if (Test-Path $installerWork) {
    Remove-Item -LiteralPath $installerWork -Recurse -Force
}
New-Item -ItemType Directory -Path $installerWork | Out-Null
Copy-Item -LiteralPath $portableZip -Destination (Join-Path $installerWork "LabelPrinterApp_Portable.zip")

$installerScript = @'
$ErrorActionPreference = "Stop"
$installRoot = Join-Path $env:LOCALAPPDATA "LabelPrinterAppBeta"
New-Item -ItemType Directory -Path $installRoot -Force | Out-Null
Expand-Archive -Path (Join-Path $PSScriptRoot "LabelPrinterApp_Portable.zip") -DestinationPath $installRoot -Force
$appPath = Join-Path $installRoot "LabelPrinterApp\LabelPrinterApp.exe"
$desktop = [Environment]::GetFolderPath("Desktop")
$shortcutPath = Join-Path $desktop "LabelPrinterApp Beta.lnk"
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $appPath
$shortcut.WorkingDirectory = Split-Path -Parent $appPath
$shortcut.Save()
Start-Process -FilePath $appPath
'@
Set-Content -Path (Join-Path $installerWork "install-labelprinterapp.ps1") -Value $installerScript

$iexpress = Get-Command iexpress.exe -ErrorAction SilentlyContinue
if ($iexpress) {
    if (Test-Path $setupExe) {
        Remove-Item -LiteralPath $setupExe -Force
    }

    $sedPath = Join-Path $installerWork "LabelPrinterApp_Setup.sed"
    $installerWorkSed = $installerWork.TrimEnd('\') + '\'
    $setupExeSed = (Resolve-Path $distRoot).Path + "\LabelPrinterApp_Setup.exe"
    $sed = @"
[Version]
Class=IEXPRESS
SEDVersion=3
[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=0
HideExtractAnimation=1
UseLongFileName=1
InsideCompressed=0
CAB_FixedSize=0
CAB_ResvCodeSigning=0
RebootMode=N
InstallPrompt=
DisplayLicense=
FinishMessage=LabelPrinterApp beta setup complete.
TargetName=$setupExeSed
FriendlyName=LabelPrinterApp Beta Setup
AppLaunched=powershell.exe -ExecutionPolicy Bypass -File install-labelprinterapp.ps1
PostInstallCmd=<None>
AdminQuietInstCmd=
UserQuietInstCmd=
SourceFiles=SourceFiles
[Strings]
FILE0="LabelPrinterApp_Portable.zip"
FILE1="install-labelprinterapp.ps1"
[SourceFiles]
SourceFiles0=$installerWorkSed
[SourceFiles0]
%FILE0%=
%FILE1%=
"@
    Set-Content -Path $sedPath -Value $sed
    $process = Start-Process -FilePath $iexpress.Source -ArgumentList @("/N", "/Q", $sedPath) -PassThru
    if (!$process.WaitForExit(120000)) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw "iexpress.exe timed out while creating LabelPrinterApp_Setup.exe."
    }
    if ($process.ExitCode -ne 0) {
        throw "iexpress.exe failed with exit code $($process.ExitCode)."
    }
} else {
    Write-Warning "iexpress.exe was not found. Skipping LabelPrinterApp_Setup.exe."
}

if (Test-Path $installerWork) {
    Remove-Item -LiteralPath $installerWork -Recurse -Force
}

Write-Host "Packaged release to $OutputDir"
Write-Host "Portable ZIP: $portableZip"
if (Test-Path $setupExe) {
    Write-Host "Setup EXE: $setupExe"
}
