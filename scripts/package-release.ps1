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

function Add-PathDirectoryCandidates {
    param(
        [System.Collections.Generic.List[string]]$Candidates
    )

    foreach ($pathEntry in ($env:PATH -split [IO.Path]::PathSeparator)) {
        Add-DirectoryCandidate -Candidates $Candidates -Path $pathEntry
    }
}

function Get-VisualStudioTool {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($root in @(
        "C:\Program Files\Microsoft Visual Studio",
        "C:\Program Files (x86)\Microsoft Visual Studio",
        "C:\Program Files (x86)\Windows Kits",
        "C:\Program Files\Microsoft SDKs"
    )) {
        if (!(Test-Path $root)) {
            continue
        }

        $tool = Get-ChildItem -LiteralPath $root -Recurse -Filter $Name -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\" -or $_.FullName -match "\\Hostx64\\x64\\" } |
            Select-Object -First 1
        if ($tool) {
            return $tool.FullName
        }
    }

    return $null
}

function Get-ImportedDllNames {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ExePath
    )

    $dumpbin = Get-VisualStudioTool -Name "dumpbin.exe"
    if (!$dumpbin) {
        Write-Warning "dumpbin.exe was not found. OpenSSL runtime copying will fall back to candidate directories."
        return @()
    }

    $output = & $dumpbin /DEPENDENTS $ExePath
    if ($LASTEXITCODE -ne 0) {
        throw "dumpbin.exe failed while inspecting $ExePath."
    }

    return $output |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_ -match "^[A-Za-z0-9_.+-]+\.dll$" } |
        Select-Object -Unique
}

function Get-OpenSslRuntimeDllNames {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ExePath
    )

    $imports = Get-ImportedDllNames -ExePath $ExePath
    $opensslImports = @($imports | Where-Object { $_ -match "^lib(ssl|crypto).+\.dll$" })
    if ($opensslImports.Count -gt 0) {
        return $opensslImports
    }

    return @("libssl*.dll", "libcrypto*.dll")
}

function Copy-OpenSslRuntimeDlls {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ExePath
    )

    $candidateDirectories = [System.Collections.Generic.List[string]]::new()

    Add-DirectoryCandidate -Candidates $candidateDirectories -Path (Split-Path -Parent $ExePath)
    Add-DirectoryCandidate -Candidates $candidateDirectories -Path $OutputDir
    Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root (Get-CMakeCacheValue -Name "OPENSSL_ROOT_DIR")
    Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root $env:OPENSSL_ROOT_DIR
    Add-OpenSslLibCandidate -Candidates $candidateDirectories -LibPath (Get-CMakeCacheValue -Name "SSL_EAY_RELEASE")
    Add-OpenSslLibCandidate -Candidates $candidateDirectories -LibPath (Get-CMakeCacheValue -Name "LIB_EAY_RELEASE")

    foreach ($commonRoot in @(
        "C:\Program Files\OpenSSL-Win64",
        "C:\Program Files\OpenSSL",
        "C:\OpenSSL-Win64",
        "C:\OpenSSL",
        "C:\Program Files\Git\mingw64",
        "C:\Program Files\Git\mingw64\bin",
        "C:\Program Files\Git\mingw64\libexec\git-core"
    )) {
        Add-OpenSslRootCandidate -Candidates $candidateDirectories -Root $commonRoot
    }
    Add-PathDirectoryCandidates -Candidates $candidateDirectories

    $requiredDllNames = @(Get-OpenSslRuntimeDllNames -ExePath $ExePath)
    $copied = [System.Collections.Generic.List[string]]::new()
    foreach ($requiredDllName in $requiredDllNames) {
        $match = $null
        foreach ($directory in $candidateDirectories) {
            $match = Get-ChildItem -LiteralPath $directory -Filter $requiredDllName -ErrorAction SilentlyContinue |
                Select-Object -First 1
            if ($match) {
                break
            }
        }

        if (!$match) {
            if ($requiredDllName.Contains("*")) {
                continue
            }
            throw "Required OpenSSL runtime DLL was not found: $requiredDllName. Install the matching OpenSSL runtime or rebuild after updating OPENSSL_ROOT_DIR."
        }

        Copy-Item -LiteralPath $match.FullName -Destination (Join-Path $OutputDir $match.Name) -Force
        $copied.Add($match.Name)
    }

    if ($copied.Count -gt 0) {
        Write-Host "Copied OpenSSL runtime DLLs: $($copied -join ', ')"
    } else {
        Write-Warning "OpenSSL runtime DLLs were not found. The update checker requires libssl/libcrypto DLLs beside LabelPrinterApp.exe."
    }

    Copy-OpenSslCompatibilityRuntimeDlls -CandidateDirectories $candidateDirectories
}

function Copy-OpenSslCompatibilityRuntimeDlls {
    param(
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$CandidateDirectories
    )

    foreach ($pair in @(
        @("libssl-3-x64.dll", "libcrypto-3-x64.dll"),
        @("libssl-4-x64.dll", "libcrypto-4-x64.dll")
    )) {
        Copy-OptionalOpenSslRuntimePair `
            -CandidateDirectories $CandidateDirectories `
            -SslDllName $pair[0] `
            -CryptoDllName $pair[1]
    }
}

function Copy-OptionalOpenSslRuntimePair {
    param(
        [Parameter(Mandatory = $true)]
        [System.Collections.Generic.List[string]]$CandidateDirectories,
        [Parameter(Mandatory = $true)]
        [string]$SslDllName,
        [Parameter(Mandatory = $true)]
        [string]$CryptoDllName
    )

    if ((Test-Path (Join-Path $OutputDir $SslDllName)) -and
        (Test-Path (Join-Path $OutputDir $CryptoDllName))) {
        return
    }

    foreach ($directory in $CandidateDirectories) {
        $ssl = Get-ChildItem -LiteralPath $directory -Filter $SslDllName -ErrorAction SilentlyContinue |
            Select-Object -First 1
        $crypto = Get-ChildItem -LiteralPath $directory -Filter $CryptoDllName -ErrorAction SilentlyContinue |
            Select-Object -First 1
        if (!$ssl -or !$crypto) {
            continue
        }

        Copy-Item -LiteralPath $ssl.FullName -Destination (Join-Path $OutputDir $ssl.Name) -Force
        Copy-Item -LiteralPath $crypto.FullName -Destination (Join-Path $OutputDir $crypto.Name) -Force
        Write-Host "Copied optional OpenSSL compatibility DLLs: $SslDllName, $CryptoDllName"
        return
    }

    Write-Warning "Optional OpenSSL compatibility DLLs were not found: $SslDllName, $CryptoDllName"
}

function Add-RequireAdministratorManifest {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ExePath,
        [Parameter(Mandatory = $true)]
        [string]$WorkDir
    )

    $mt = Get-VisualStudioTool -Name "mt.exe"
    if (!$mt) {
        Write-Warning "mt.exe was not found. The setup EXE was created, but no UAC manifest was embedded."
        return
    }

    $manifestPath = Join-Path $WorkDir "LabelPrinterApp_Setup.manifest"
    $manifest = @'
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="requireAdministrator" uiAccess="false" />
      </requestedPrivileges>
    </security>
  </trustInfo>
</assembly>
'@
    Set-Content -LiteralPath $manifestPath -Value $manifest -Encoding UTF8
    & $mt -manifest $manifestPath "-outputresource:$ExePath;#1" | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "mt.exe failed while embedding the UAC manifest in $ExePath."
    }
    Write-Host "Embedded requireAdministrator manifest in $ExePath"
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

Copy-OpenSslRuntimeDlls -ExePath (Join-Path $OutputDir "LabelPrinterApp.exe")

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
    Add-RequireAdministratorManifest -ExePath $setupExe -WorkDir $installerWork
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
