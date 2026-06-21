param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build"
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

$cmake = Find-CMake
$ctest = Join-Path (Split-Path $cmake -Parent) "ctest.exe"
if (!(Test-Path $ctest)) {
    $ctestCommand = Get-Command ctest.exe -ErrorAction SilentlyContinue
    if (!$ctestCommand) {
        throw "Unable to find ctest.exe next to CMake or on PATH."
    }
    $ctest = $ctestCommand.Source
}

Write-Host "Using CMake: $cmake"
& $cmake -S . -B $BuildDir
& $cmake --build $BuildDir --config $Config
& $ctest --test-dir $BuildDir -C $Config --output-on-failure
