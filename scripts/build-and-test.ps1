param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build",
    [string]$CMakePrefixPath = $env:CMAKE_PREFIX_PATH
)

$ErrorActionPreference = "Stop"

function Find-CMake {
    $pathCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($pathCommand) {
        return $pathCommand.Source
    }

    $candidates = @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $vswhereCandidates = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe",
        "C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
    )

    foreach ($vswhere in $vswhereCandidates) {
        if (!(Test-Path $vswhere)) {
            continue
        }

        $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if (!$installPath) {
            continue
        }

        $candidate = Join-Path $installPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "Unable to find cmake.exe. Install CMake or Visual Studio with Desktop development with C++."
}

function Find-QtPrefix {
    if ($env:CMAKE_PREFIX_PATH) {
        return $env:CMAKE_PREFIX_PATH
    }

    if ($env:Qt6_DIR) {
        $qt6ConfigDir = Resolve-Path $env:Qt6_DIR -ErrorAction SilentlyContinue
        if ($qt6ConfigDir) {
            $qtPrefix = Resolve-Path (Join-Path $qt6ConfigDir.Path "..\..\..") -ErrorAction SilentlyContinue
            if ($qtPrefix) {
                return $qtPrefix.Path
            }
        }
    }

    $qtRoot = "C:\Qt"
    if (Test-Path $qtRoot) {
        $candidate = Get-ChildItem -Path $qtRoot -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object {
                $prefix = Join-Path $_.FullName "msvc2022_64"
                if (Test-Path (Join-Path $prefix "lib\cmake\Qt6\Qt6Config.cmake")) {
                    $prefix
                }
            } |
            Select-Object -First 1

        if ($candidate) {
            return $candidate
        }
    }

    return $null
}

function Invoke-Native {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $FilePath $($Arguments -join ' ')"
    }
}

function Clear-MsBuildTrackingLogs {
    param(
        [string]$BuildDir,
        [string]$Config
    )

    if (!(Test-Path $BuildDir)) {
        return
    }

    $configDirs = Get-ChildItem -Path $BuildDir -Directory -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -eq $Config -and $_.FullName -like "*.dir\$Config" }

    foreach ($configDir in $configDirs) {
        Get-ChildItem -Path $configDir.FullName -Directory -Filter "*.tlog" -ErrorAction SilentlyContinue |
            ForEach-Object {
                Remove-Item -LiteralPath $_.FullName -Recurse -Force
            }
    }
}

$cmake = Find-CMake
$ctest = Join-Path (Split-Path $cmake -Parent) "ctest.exe"
if (!(Test-Path $ctest)) {
    $ctestCommand = Get-Command ctest.exe -ErrorAction SilentlyContinue
    if (!$ctestCommand) {
        throw "Unable to find ctest.exe next to CMake or on PATH."
    }
    $ctest = $ctestCommand.Source
}

if (!$CMakePrefixPath) {
    $CMakePrefixPath = Find-QtPrefix
}

Write-Host "Using CMake: $cmake"
if ($CMakePrefixPath) {
    Write-Host "Using CMAKE_PREFIX_PATH: $CMakePrefixPath"
}
if ($CMakePrefixPath) {
    Invoke-Native $cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH="$CMakePrefixPath"
} else {
    Invoke-Native $cmake -S . -B $BuildDir
}
Clear-MsBuildTrackingLogs -BuildDir $BuildDir -Config $Config
Invoke-Native $cmake --build $BuildDir --config $Config
Invoke-Native $ctest --test-dir $BuildDir -C $Config --output-on-failure
