# Release Process

LabelPrinterApp uses GitHub Actions to build and publish release artifacts.

## Release Outputs

The packaged release contains:

- `LabelPrinterApp_Portable.zip`
- `LabelPrinterApp_Setup.exe`

The portable ZIP contains the app, Qt runtime files, blank templates, examples, docs, and an initial print-history file.

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

4. Open `dist\LabelPrinterApp\LabelPrinterApp.exe`.
5. Run the manual checks in `docs\MANUAL_QA_CHECKLIST.md`.

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
