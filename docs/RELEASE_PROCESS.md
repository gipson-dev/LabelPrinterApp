# Release Process

LabelPrinterApp uses GitHub Actions to build and publish release artifacts.

## Release Outputs

The packaged release contains:

- `LabelPrinterApp_Portable.zip`
- `LabelPrinterApp_Setup.exe`

The portable ZIP contains `LabelPrinterApp.exe`, `LabelPrinterAppLauncher.exe`, Qt runtime files, OpenSSL runtime DLLs for the update checker, blank templates, examples, docs, and an initial print-history file.

These are also the releases that LabelPrinterApp's in-app updater (`Help > Check for Updates` and the silent startup check) looks for at `gipson-dev/LabelPrinterApp`. The updater reads the version from the `v*` tag and only recognizes an asset named exactly `LabelPrinterApp_Portable.zip`, so keep both conventions unchanged.

## Local Preflight

Before tagging a release:

1. Close any running copy of `LabelPrinterApp.exe`.
2. Run the build and tests:

```powershell
.\scripts\build-and-test.ps1 -Config Release
```

3. Build the package:

```powershell
.\scripts\package-release.ps1 -Config Release
```

4. Confirm `dist\LabelPrinterApp` and `dist\LabelPrinterApp_Portable.zip` include matching `libssl*.dll` and `libcrypto*.dll` files beside `LabelPrinterApp.exe`.
5. Open `dist\LabelPrinterApp\LabelPrinterApp.exe`.
6. Run the manual checks in `docs\MANUAL_QA_CHECKLIST.md`, including the `Check for Updates` flow against the currently-latest published GitHub release.
7. Review `docs\KNOWN_ISSUES.md` and update it with any new release-specific limitations.
8. In VS Code, open `core\TemplateStorage.cpp` and confirm C/C++ IntelliSense resolves `nlohmann/json.hpp` after configure/build.
9. Bump the project version in the root `CMakeLists.txt` (`project(LabelPrinterApp VERSION ...)`) to match the tag you are about to push, so the update checker can compare versions correctly.

## Publish A Beta Release

Use a beta tag name such as:

```text
v0.1.0-beta.2
```

Push `main`, then push the tag:

```powershell
git push origin main
git tag v0.1.0-beta.2
git push origin v0.1.0-beta.2
```

The GitHub Actions release workflow runs on `v*` tags. Tags containing `beta` are published as prereleases.

## Verify GitHub Release

After the workflow completes:

1. Open the GitHub Releases page.
2. Confirm the new beta release exists.
3. Confirm both artifacts are attached:
   - `LabelPrinterApp_Portable.zip`
   - `LabelPrinterApp_Setup.exe`
4. Download and run the portable ZIP on a clean Windows folder.
5. Run the setup EXE on a test machine or VM.
6. From an older installed build, run `Help > Check for Updates` and confirm it finds this release, downloads and verifies it, and offers to restart; confirm the restart applies the update through `LabelPrinterAppLauncher.exe` and keeps `templates\` and `logs\` intact.

## Current Beta Scope

The current beta includes:

- Blank 2.25 x 0.75 and 4 x 2 canvas templates.
- Number and Description field workflow.
- CSV and Excel import from the Data tab.
- Selected/all imported record printing.
- Canvas drag-to-move and anchor resizing.
- Vertically centered text preview with corrected Zebra text-origin output.
- Line and Box design elements with preview, resizing, template storage, and ZPL output.
- Cut, Copy, Paste, Undo, Redo, Zoom In/Out/Fit, and Help actions in the classic toolbar/menu.
- Font sizes above 72 dots.
- Zebra RAW ZPL printing through Windows printers.
- Persistent app settings and print history.
- VS Code C/C++ include paths for fetched `nlohmann/json.hpp` so local developer squiggles match the CMake build.
- Self-updating releases: a silent startup check and `Help > Check for Updates` download and verify newer GitHub releases and apply them through `LabelPrinterAppLauncher.exe` on restart.
